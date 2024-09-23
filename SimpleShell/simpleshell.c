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

int main(){

    do{
        printf("ishaan&&divyansh@simpleShell:~$ ");
        if(fgets(input,sizeof(input),stdin)==NULL){
            perror("")
        }
    }

    return 0;
}