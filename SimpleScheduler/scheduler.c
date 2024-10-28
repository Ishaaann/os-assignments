#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/time.h>

int NCPU; //no. of cores
int TSLICE;// time slice

int shell_running = 1; //flag to check if the shell is running 

//main function
int main(int argc, char** argv){
    NCPU = atoi(argv[1]); //number of processes
    TSLICE = atoi(argv[2]); //Time slice quantum
    char input[100]; //input array
    int pipefd[2]; //pipe array
    pipe(pipefd);

    //forking a child process
    pid_t pid = fork();


    //runs if the process is a child process
    if(pid == 0){
        struct sigaction sig; //setting up our own signal handler to handle custom signals or the way we want them to execute
        memset(&sig, 0, sizeof(sig));
        sig.sa_handler = &sig_alarm_handler;
        sigaction(SIGALRM, &sig, NULL); //SIGALRM command handling
        sigaction(SIGINT, &sig, NULL); //SIGINT command handling

        //error in handling pipes
        if(close(pipefd[1]) == -1){
            perror("Error in closing pipe")
        }
        char exe[20];
        int priority;
    }

    while(shell_running){
        //custom prompt 
        printf("SimpleShell $ ");
        if(fgets(input, 100, stdin) == NULL){
            perror("Error while reading input");
        }

        if(!subString(input,"submit")){
            printf("Invalid command\n");
            continue;
        }

        write(pipefd[1], input, strlen(input)+1);
    
    }


    return 0;
}