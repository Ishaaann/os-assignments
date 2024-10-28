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

        q1 = (struct Heap*) malloc(sizeof(struct Heap));
        if(q1 = NULL){
            perror("Error allocating memory.");
        }

        q1->arr = (struct->entry) malloc(100*sizeof(struct entry));
        if(q1->arr == NULL){
            perror("Error allocating memory")
        }

        q1->size = 0;
        q1->capacity = 100;

        start_timer();

        while(1){
            read(pipefd[0],input,100);
            sscanf(input,"submit %s %d", exe, &priority);
            struct proc p = make_process(exe,priority);

            pid_t pid_process = fork();

            if(pid_process == 0){
                raise(SIGSTOP);
            }

            if(system(p.cmd) == -1){
                perror("Error executing command");
            }

            kill(pid_scheduler, SIGCHLD);
            exit(0);
        }

        else{
            p.pid = pid_process;
            p.waitTime = TSLICE;
        }
        insert(q1,p);
    }
    else{
        if(close(pipefd[0]) == -1){
            perror("Error closing pipe");
        }
        pid_scheduler = pid;

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
    }
}