#ifndef __EXECUTE_H
#define __EXECUTE_H

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

#include "processus.h"

int execute_command(char* in, char* out, int bg, char*** seq, struct Processus * liste_processus);

#endif
