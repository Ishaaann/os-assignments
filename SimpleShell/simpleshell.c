#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// System calls for process creation, waiting, and types.
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h> // Signal handling
#include <time.h> // Time-related functions
#include <ctype.h> 

int launch(char* cmd){
    int status = create_process_and_run(cmd);
    return status;
}

void my_handler(int signum){
    if(signum==SIGINT){
        SIGINT_history();
    }
}


int main(){
    struct sigaction sig;
    memset(&sig,0,sizeof(sig));
    sig.sa_handler = my_handler;
    sigaction(SIGINT, &sig, NULL);

    do{
        printf("ishaan&&divyansh@SimpleShell:$ ");
        char* cmd = readUserInput();

        if(cmd!=NULL){
            int status = launch(cmd);
            free(cmd);

            if(cmd!=0){
                printf("Command exited with status %d\n", status);  
            }
        }
    }
    while(true); //infinte loop until loop is broken with CTRL+C

    return 0;
}