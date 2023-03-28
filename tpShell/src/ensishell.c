/*****************************************************
 * Copyright Grégory Mounié 2008-2015                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "variante.h"
#include "readcmd.h"

#ifndef VARIANTE
#error "Variante non défini !!"
#endif

/* Guile (1.8 and 2.0) is auto-detected by cmake */
/* To disable Scheme interpreter (Guile support), comment the
 * following lines.  You may also have to comment related pkg-config
 * lines in CMakeLists.txt.
 */

#if USE_GUILE == 1
#include <libguile.h>

int question6_executer(char *line)
{
	/* Question 6: Insert your code to execute the command line
	 * identically to the standard execution scheme:
	 * parsecmd, then fork+execvp, for a single command.
	 * pipe and i/o redirection are not required.
	 */
	printf("Not implemented yet: can not execute %s\n", line);

	/* Remove this line when using parsecmd as it will free it */
	free(line);
	
	return 0;
}

SCM executer_wrapper(SCM x)
{
        return scm_from_int(question6_executer(scm_to_locale_stringn(x, 0)));
}
#endif


void terminate(char *line) {
#if USE_GNU_READLINE == 1
	/* rl_clear_history() does not exist yet in centOS 6 */
	clear_history();
#endif
	if (line)
	  free(line);
	printf("exit\n");
	exit(0);
}

struct Processus {
	pid_t pid;
	char * name;
	struct Processus * next;
};

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


int main() {
	
	struct Processus * liste_processus = NULL;

    printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);

#if USE_GUILE == 1
    scm_init_guile();
    /* register "executer" function in scheme */
    scm_c_define_gsubr("executer", 1, 0, 0, executer_wrapper);
#endif

	while (1) {
		struct cmdline *l;
		char *line=0;
		char *prompt = "ensishell>";
		int status;

		/* Readline use some internal memory structure that
		   can not be cleaned at the end of the program. Thus
		   one memory leak per command seems unavoidable yet */
		line = readline(prompt);
		if (line == 0 || ! strncmp(line,"exit", 4)) {
			terminate(line);
		}

#if USE_GNU_READLINE == 1
		add_history(line);
#endif


#if USE_GUILE == 1
		/* The line is a scheme command */
		if (line[0] == '(') {
			char catchligne[strlen(line) + 256];
			sprintf(catchligne, "(catch #t (lambda () %s) (lambda (key . parameters) (display \"mauvaise expression/bug en scheme\n\")))", line);
			scm_eval_string(scm_from_locale_string(catchligne));
			free(line);
            continue;
        }
#endif
		/* parsecmd free line and set it up to 0 */
		l = parsecmd(& line);

		/* If input stream closed, normal termination */
		if (!l) {
			terminate(0);
		}

		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		/*exécution des commandes*/
		for (uint32_t i=0; l->seq[i]!=0; i++) {

			if (!strncmp(l->seq[0][0], "jobs", 4)) {
				/*commande interne jobs pour afficher les processus en arrière plan*/
				jobs(&liste_processus);
				continue;
			}

			/*fork pour dupliquer le processus*/
			pid_t pid = fork();

			if (pid == -1) {
				printf("erreur de fork");
			}

			/*processus enfant*/
			else if (pid == 0) {
				/*execvp pour avoir le bon processus*/
				execvp(l->seq[i][0], l->seq[i]);
				exit(0);
			}

			/*processus parent*/
			else {
				if(!l->bg) {
					/*processus pas en tâche de fond => le père attend la fin du fils*/
					pid_t wpid = waitpid(pid, &status, 0);
					if(wpid == -1) {
						printf("erreur dans le wait");
						exit(1);
					}
				}
				else {
					/*processus fils en tâche de fond : onl'ajoute à la liste*/
					ajout_processus(pid, l->seq[i][0], &liste_processus);
				}
			}
		}
	}

}
