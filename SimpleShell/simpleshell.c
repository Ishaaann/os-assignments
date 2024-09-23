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
#define MAX_INPUT_SIZE 1024

int checkHistory(char* cmd){
    if(strcmp(cmd, "history") == 0){
        return 1;
    }
    return 0;
}

int checkCD(char* cmd){
    if(strncmp(cmd,"cd ",3) == 0){
        return 1;
    }
    return 0;
}

int checkPipe(char* cmd){
    if(strchr(cmd, '|') == NULL){
        return 1;
    }
    return 0;
}

int launch(char* cmd){
    int status = create_process_and_run(cmd);
    return status;
}

void my_handler(int signum){
    if(signum==SIGINT){
        SIGINT_history();
    }
}

//defined an array for storigng the history of commands
struct history history_entries[100];
int counter = 0;//counter for iterating the history array - history_entries

void print_history(){
    //print the history by looping through the entry
    for(int i = 0; i<counter; i++){
        printf("%s\n",history_entries[i].command);
    }
}

void SIGINT_history(){
    //print all the commands in the history when CTRL+C is pressed
    for(int i = 0; i<counter; i++){
        struct history entry = history_entries[i];

        printf("Command: %s\n", entry.command);
        printf("PID: %d\n", entry.entries[0]);
        printf("Starting Time: %s", ctime((const time_t *)&entry.entries[1])); // Convert time to a readable format
        if (entry.entries[2] != -1) {
            printf("Running Time: %d seconds\n", entry.entries[2]);
        } else {
            printf("Running Time: nil\n");
        }
        printf("\n");
        
    }
}

void create_process_and_run(char* cmd){
    if(checkHistory){
        print_history(cmd);
        return 0;
    }
    if(checkCD(cmd)){
        char* path = cmd+3;
        if(chdir(path) == -1){
            perror("chdir");
            return 1;
        }
    }
    //Record cd command in history(array)
    history_entries[counter].command = strdup(cmd);
        history_entries[counter].entries[0] = getpid(); // Current process ID
        history_entries[counter].entries[1] = time(NULL); // Current time
        history_entries[counter].entries[2] = -1; // Execution time not available
        counter++;

    if(checkPipe(cmd)){
        return pipe_commands(cmd);
    }

    pid_t child_pid;
    int status;

    //Recording the starting time
    struct timeval startTime;
    mingw_gettimeofday(&startTime, NULL);

    child_pid = fork();
    if(child_pid == -1){
        perror("Fork");
        return 1;
    }
    else if(child_pid == 0){
        //child process starts running 
        //this code is executed by the child process
        //using exec commands
        if(execl("/bin/sh","sh","-c", cmd, NULL) == -1){
            perror("Execl");
            exit(EXIT_FAILURE);
        }
        exit(0);
    }
    else(
        //this code is executed by the parent process
        //parent waits for the child process to complete 
        waitpid(child_pid,&status,0);
        if(WIFEXITED(status)){
            //Recording the end time
            struct timeval endTime;
            mingw_gettimeofday(&endTime, NULL);
            long long interval = (endTime.tv_sec - startTime.tv_sec) * 1000LL +
                                 (endTime.tv_usec - startTime.tv_usec) / 1000LL;
        

        //update entry
        if (counter < 100) {
                history_entries[counter].command = strdup(cmd);
                history_entries[counter].entries[0] = child_pid;
                history_entries[counter].entries[1] = startTime.tv_sec;
                history_entries[counter].entries[2] = (int)interval;
                counter++;
            }
            return WIFEXITED(status);
        }
        else{
            perror("waitpid");
            return 1;
        }
    )
}


int main(){
    struct sigaction sig;
    memset(&sig,0,sizeof(sig));
    sig.sa_handler = my_handler;
    sigaction(SIGINT, &sig, NULL);

    do{
        printf("ishaan&&divyansh@SimpleShell:$ ");
        char* cmd = read_command();

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