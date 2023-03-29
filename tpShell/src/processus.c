import "processus.h"


void ajout_processus(pid_t pid, char* name, struct Processus ** liste) {
	/*ajout d'un processus à la liste des processus lancés en tâche de fond*/
	struct Processus * p = (struct Processus *) malloc(sizeof(struct Processus));
	p->pid = pid;
	p->name = strdup(name);
	p->next = *liste;

	*liste = p;
}


void grand_menage_processus(struct Processus ** liste) {
	/*supprime tous les processus terminés de la liste*/
	struct Processus * prec = NULL;
	struct Processus * courant = * liste;

	while(courant != NULL) {
		int32_t status;
		pid_t pid = waitpid(courant->pid, &status, WNOHANG);

		if(pid != 0) {
			/*a terminé : on supprime*/
			prec->next = courant->next;
			free(courant->name);
			free(courant);
			/*on continue le parcours*/
			courant = prec->next;
		}
		else {
			/*on continue le parcours*/
			prec = courant;
			courant = courant->next;
		}
	}
}

void jobs(struct Processus ** liste_processus) {
	/*afficher la liste des processus lancés en tâche de fond de votre shell
	avec leur pid et la commande lancée*/
	grand_menage_processus(liste_processus);

	struct Processus *p = *liste_processus;
    while(p != NULL) {
        printf("[%d] running %s\n", p->pid, p->name);
        p = p->next;
    }
}

