#ifndef __PROCESSUS_H
#define __PROCESSUS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct Processus {
	pid_t pid;
	char * name;
	struct Processus * next;
};


/*Ajoute un processus dans la liste des processus s'exécutant en tâche de fond.*/
void ajout_processus(pid_t pid, char* name, struct Processus ** liste);


/*Retire les processus terminé de la liste des processus s'exécutant en tâche de fond.*/
void grand_menage_processus(struct Processus ** liste);


/*Affiche la liste des processus s'exécutant en tâche de fond au moment de l'appel de la commande interne jobs.*/
void jobs(struct Processus ** liste_processus);

#endif