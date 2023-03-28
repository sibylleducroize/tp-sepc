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





int piping(char* in, char* out, int bg, char*** seq){
    int pipefd[2];
    pid_t pid_1, pid_2;
    
    // create pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // fork and execute left hand of pipe
    pid_1 = fork();
    if (pid_1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid_1 == 0) {
        // child process - set stdout to write end of pipe
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        // execute left hand command
        if (execvp(seq[0][0], seq[0]) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else {
        // parent process - fork and execute right hand command
        pid_2 = fork();
        if (pid_2 == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid_2 == 0) {
            // child process - set stdin to read end of pipe
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            // execute wc
            if (execvp(seq[1][0], seq[1]) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else {
            // parent process - close pipe and wait for children
            close(pipefd[0]);
            close(pipefd[1]);
            waitpid(pid_1, NULL, 0);
            waitpid(pid_2, NULL, 0);
        }
    }

    return 0;
}

int execute_command(char* in, char* out, int bg, char*** seq){
    //check for piping
    if(seq[1]!=0){
        return piping(in, out, bg, seq);
    }

    pid_t pid = fork();

    if(pid<0){
        perror("Fork failed.");
        exit(EXIT_FAILURE);
    }else if(pid==0){
        //we are in the child processe, where the command should be executed
        if(execvp(seq[0][0], seq[0])==-1){
            perror("Execution error");
            exit(EXIT_FAILURE);
        }
    }else{
        //we are in the parent 
        wait(NULL);
    }
    return 0;
}


int main() {
        printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);

#if USE_GUILE == 1
        scm_init_guile();
        /* register "executer" function in scheme */
        scm_c_define_gsubr("executer", 1, 0, 0, executer_wrapper);
#endif

	while (1) {
		struct cmdline *l;
		char *line=0;
		int i, j;
		char *prompt = "ensishell>";

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
		l = parsecmd( & line);

		/* If input stream closed, normal termination */
		if (!l) {
		  
			terminate(0);
		}
		

		
		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		execute_command(l->in, l->out, l->bg, l->seq);

		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
			printf("seq[%d]: ", i);
                        for (j=0; cmd[j]!=0; j++) {
                                printf("'%s' ", cmd[j]);
                        }
			printf("\n");
		}
	}

}
