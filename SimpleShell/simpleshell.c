#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <ctype.h> 
#define MAX_INPUT_LENGTH 1024

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

//defined an array for storing the history of commands
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


char* read_command() {
    char command[MAX_INPUT_LENGTH];

    //read user input
    if (fgets(command, sizeof(command), stdin) == NULL){
        perror("fgets error");
        exit(EXIT_FAILURE);
    }
    size_t length = strlen(command);
    if (length > 0 && command[length - 1] == '\n') {
        command[length - 1] = '\0';
    }
    // Check for backslashes or quotes in the input
    if (strchr(command, '\\') != NULL || strchr(command, '"') != NULL || strchr(command, '\'') != NULL) {
        printf("Error: Backslashes or quotes are not allowed in the command.\n");
        return NULL;
    }
    return strdup(command);
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

void pipe(char *command) {
    char *commands[MAX_INPUT_LENGTH];
    int number_of_commands = 0;

    char *token = strtok(command, "|"); //will implement do while loop here later
    while (token != NULL && number_of_commands < MAX_INPUT_LENGTH) {
        commands[number_of_commands] = token;
        number_of_commands++;
        token = strtok(NULL, "|");
    }
    commands[number_of_commands] = NULL;

    int pipefd[2];
    pid_t cpid;
    int status;
    int init_pipe_read = STDIN_FILENO;

    char piped_command[MAX_INPUT_LENGTH] = "";
    //concatenate the commands with '|' in between them
    for (int i = 0; i < number_of_commands; i++) {
        strcat(piped_command, commands[i]);
        if (i<number_of_commands-1){
            strcat(piped_command, " | ");
        }
    }
    for (int i = 0; i < number_of_commands; i++) {
        //create pipe for the commands except the ending one
        if (i < number_of_commands - 1) {
            if (pipe(pipefd) == -1){
                perror("pipe error");
                exit(EXIT_FAILURE);
            }
        }
        cpid = fork();
        if (cpid == -1){
            perror("fork error");
            exit(EXIT_FAILURE);
        } else if (cpid == 0) {
            if (i!=0){
                if (dup2(init_pipe_read, init_pipe_read) == -1) {
                    perror("dup2 error");
                    exit(EXIT_FAILURE);
                }
                close(init_pipe_read);
            }

            if (i < number_of_commands - 1) {
                if (dup2(pipefd[1], init_pipe_read) == -1) {
                    perror("dup2 error");
                    exit(EXIT_FAILURE);
                }
                close(pipefd[1]);
            }

            //close all pipes
            for (int j = 0; j < number_of_commands; j++) {
                close(pipefd[j]);
            }

            //split the current command
            char *args[MAX_INPUT_LENGTH];
            int argc = 0;

            char *arg_token = strtok(commands[i], "|");
            while (arg_token != NULL && argc < MAX_INPUT_LENGTH) {
                args[argc] = arg_token;
                argc++;
                arg_token = strtok(NULL, " ");
            }
            args[argc] = NULL;

            //Executing the command using execvp
            if (execvp(args[0], args) == -1) {
                perror("execvp error");
                exit(EXIT_FAILURE);
            }
        } else {
            //parent process
            if (i < number_of_commands - 1) {
                close(pipefd[1]);
            }
            if (i!=0){
                close(init_pipe_read);
            }

            waitpid(cpid, &status, 0);
            if(WIFEXITED(status)) {
                if(i==0) {
                    history_entries[counter].command = strdup(piped_command);
                    history_entries[counter].entries[0] = cpid;
                    history_entries[counter].entries[1] = time(NULL); // Current time
                    history_entries[counter].entries[2] = -1; // Execution time not available
                    counter++;
                }
            } else {
                perror("waitpid error");
                exit(EXIT_FAILURE);
            }
            init_pipe_read = pipefd[1];
        }
    }
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
