#include "exec.h"


void execute_command(char* in, char* out, int bg, char*** seq){
    //For now not using in and out
    for(int i =0; seq!=0, seq[i]!=0, seq[i][0]!=0;i++){
        //meaning there is a command in the sequence, at least
        pid_t pid = fork();
        if(pid<0){
            printf("Fork failed.");
            exit(1);
        }else if (pid==0){
            //we are in the child
            if(execvp(seq[i][0], seq)==-1){
                print("Error executing command %s.\n", seq[i][0]);
                exit(1);
            }
        }else{
            //we are in the parent so we must wait for the child to finish
            wait(NULL);
        }

    }
    return 0; 
}