#include "execute.h"

int execute_command(char* in, char* out, int bg, char*** seq, struct Processus ** liste_processus){
    
    if (!strncmp(seq[0][0], "jobs", 4)) {
		/*commande interne jobs pour afficher les processus en arrière plan*/
		jobs(liste_processus);
		return 0;
	}
    
    char piping = 0;
    int pipefd[2] = {0,0};
    /*Initializing parameters of execution*/
    if(seq[1]!=0){
        piping = 1;
        pipe(pipefd);
    }

    pid_t pid_1 = fork();
    if(pid_1<0){
        perror("Fork failed.");
        exit(EXIT_FAILURE);
    }
    else if(pid_1==0){
        //we are in the child process, 
        //where the left hand command should be executed
        if(piping){
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);
        }
        if(in!=NULL){
            if(freopen(in, "r", stdin) == NULL){
                perror("Failed to open input file");
                exit(EXIT_FAILURE);
            }
        }

        if(out!=NULL && !piping){ // If output redirection, redirect stdout to file
                if(freopen(out, "w", stdout) == NULL){
                    perror("Failed to open output file");
                    exit(EXIT_FAILURE);
                }
        }
        
        if(execvp(seq[0][0], seq[0])==-1){
            perror("Execution error");
            exit(EXIT_FAILURE);
        }
    }
    else{
        //we are in the parent
        if(piping){
            pid_t pid_2 = fork();
            if (pid_2 == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (pid_2 == 0) {
                // child process - set stdin to read end of pipe
                close(pipefd[1]);
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);
                // execute right hand command
                if(out!=NULL){
                    // If output redirection, redirect stdout to file
                    if(freopen(out, "w", stdout) == NULL){
                        perror("Failed to open output file");
                        exit(EXIT_FAILURE);
                    }
                }
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
        }else{
            if(!bg) {
				/*processus pas en tâche de fond => le père attend la fin du fils*/
				int status;
                pid_t wpid = waitpid(pid_1, &status, 0);
				if(wpid == -1) {
					printf("erreur dans le wait");
					exit(1);
                }
            }
			else {
				/*processus fils en tâche de fond : onl'ajoute à la liste*/
				ajout_processus(pid_1, seq[0][0], liste_processus);
			}
        }
    }
    return 0;
}