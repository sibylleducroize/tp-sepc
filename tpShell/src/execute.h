#ifndef __EXECUTE_H
#define __EXECUTE_H

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

int execute_command(char* in, char* out, int bg, char*** seq);

#endif