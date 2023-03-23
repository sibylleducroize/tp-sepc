/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/
#include <stdint.h>
#include <assert.h>
#include <stdint.h>
#include "mem.h"
#include "mem_internals.h"



/*
 * enlève le premier élément de la liste chainée pointée par arena.chunkpool
 * appelle la fonction de marquage en lui donnant l’adresse de l’élément, sa taille (CHUNKSIZE) et le type (SMALL_KIND)
 * renvoie le résultat de la fonction de marquage
 *
 * Si arena.chunkpool est vide (NULL), appeler unsigned long mem_realloc_small() qui maj arena.chunkpool
 * et renvoie la taille du bloc en octets qui est dans arena.chunkpool
 * construire une liste chainée de chunks en utilisant ce bloc.
 * Il suffit de chainer vos pointeurs vers le chunk suivant tous les 96 octets (CHUNKSIZE).
 */
void * emalloc_small(unsigned long size) {
    if (arena.chunkpool == NULL) {
        //mise à jour de arena.chunkpool
        unsigned long taille_bloc = mem_realloc_small();
        //découpage du bloc en plusieurs chunks de CHUNKSIZE octets et fabrication liste chaînée
        uint8_t * ptr_chunk = (uint8_t *) arena.chunkpool;
        uint64_t nb_chunks = taille_bloc/CHUNKSIZE;
        for (uint64_t i = 0; i < nb_chunks - 1; i++) {
            //on lie le chunk courant au suivant
            * (unsigned long *) ptr_chunk = *(unsigned long *) (ptr_chunk + CHUNKSIZE);
            ptr_chunk += CHUNKSIZE;
        }
        //pour le dernier élément, on le fait pointer vers rien (null)
        ptr_chunk = NULL;
    }
    void ** elem_tete = (void **) arena.chunkpool;
    void * elem_suiv = * elem_tete;
    arena.chunkpool = elem_suiv;
    void * ptr_mark = mark_memarea_and_get_user_ptr(elem_tete, CHUNKSIZE, SMALL_KIND);
    return ptr_mark;
}

void efree_small(Alloc a) {
    //séparation des cas si arena.chunkpool est vide ou pas
    if (arena.chunkpool == NULL) {
        //ajout de a
        arena.chunkpool = a.ptr;
        //maj du suivant de a
        * (void **) a.ptr = (void *) NULL;
    }
    else {
        //ajout de a en tête
        void * elem_tete = arena.chunkpool;
        arena.chunkpool = a.ptr;
        * (void **) a.ptr = elem_tete;   
    }
}
