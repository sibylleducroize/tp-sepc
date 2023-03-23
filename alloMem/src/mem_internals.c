/******************************************************
 * Copyright Grégory Mounié 2018-2022                 *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>
#include "mem.h"
#include "mem_internals.h"

unsigned long knuth_mmix_one_round(unsigned long in)
{
    return in * 6364136223846793005UL % 1442695040888963407UL;
}

void write_marks();
/*S: 0, M:1, L:2*/
void *mark_memarea_and_get_user_ptr(void *ptr, unsigned long size, MemKind k)
{   
    unsigned long magic = knuth_mmix_one_round((unsigned long) ptr);
    //MemKind is an enumerate
    magic = (magic & ~(0b11UL)) | (unsigned long) k;

    //create a pointer to 64bit space, pointing to beginning of allocated memory
    uint64_t *small_pointer = (uint64_t*) ptr;
    //now small pointer points to the first 4bits of ptr
    
    //fill first size
    *small_pointer = size;
    //fill first magic
    small_pointer++;
    *small_pointer = magic;

    void *returned_pointer = (void*)(small_pointer+1);

    char* pointer_to_end = (char*) ptr;
    pointer_to_end += size - 2*8;
    small_pointer = (uint64_t* ) pointer_to_end;

    //fill end magic
    *small_pointer = magic;
    //fill end size
    small_pointer++;
    *small_pointer = size;
    return returned_pointer;
}

Alloc
mark_check_and_get_alloc(void *ptr)
{
    Alloc *memory_space = malloc(sizeof(Alloc));
    memory_space->ptr = (void*) ((uint64_t*) ptr -2); // retuened pointer is the one before markings
    memory_space->size = *((uint64_t*)ptr -2);
    uint64_t magic = *((uint64_t*)ptr -1);
    memory_space->kind = magic & 0b11UL;
    assert(magic == ((knuth_mmix_one_round((unsigned long)memory_space->ptr) & ~(0b11UL)) | (unsigned long) memory_space->kind));

    //check end markings
    uint64_t* pointer_to_end = (uint64_t*) ((char*)memory_space->ptr +memory_space->size -2*8);
    //checking end magic
    assert(*pointer_to_end == magic);
    pointer_to_end++;
    assert(*pointer_to_end== memory_space->size);

    return *memory_space;
}


unsigned long
mem_realloc_small() {
    assert(arena.chunkpool == 0);
    unsigned long size = (FIRST_ALLOC_SMALL << arena.small_next_exponant);
    arena.chunkpool = mmap(0,
			   size,
			   PROT_READ | PROT_WRITE | PROT_EXEC,
			   MAP_PRIVATE | MAP_ANONYMOUS,
			   -1,
			   0);
    if (arena.chunkpool == MAP_FAILED)
	handle_fatalError("small realloc");
    arena.small_next_exponant++;
    return size;
}

unsigned long
mem_realloc_medium() {
    uint32_t indice = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;
    assert(arena.TZL[indice] == 0);
    unsigned long size = (FIRST_ALLOC_MEDIUM << arena.medium_next_exponant);
    assert( size == (1UL << indice));
    arena.TZL[indice] = mmap(0,
			     size*2, // twice the size to allign
			     PROT_READ | PROT_WRITE | PROT_EXEC,
			     MAP_PRIVATE | MAP_ANONYMOUS,
			     -1,
			     0);
    if (arena.TZL[indice] == MAP_FAILED)
	handle_fatalError("medium realloc");
    // align allocation to a multiple of the size
    // for buddy algo
    arena.TZL[indice] += (size - (((intptr_t)arena.TZL[indice]) % size));
    arena.medium_next_exponant++;
    return size; // lie on allocation size, but never free
}


// used for test in buddy algo
unsigned int
nb_TZL_entries() {
    int nb = 0;
    
    for(int i=0; i < TZL_SIZE; i++)
	if ( arena.TZL[i] )
	    nb ++;

    return nb;
}
