#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/time.h>


#define PARENT(i) (i/2)
#define LEFT(i) (2*i)
#define RIGHT(i) (2*i + 1)
/*int PARENT(int i){
    return i/2;
}
int LEFT(int i){
    return 2*i;
}
int RIGHT(int i){
    return 2*i+1;
}*/

//process structure which contains all necessary and relevant properties
struct proc {
    char cmd[100];//command
    pid_t pid;//process ID
    int priority;//priority
    int execution_time;//time taken for execution
    int wait_time;//time waited brfore execution
    char state[10]; // READY or RUNNING
    // process gets dumped in the terminated_arr after the process gets terminated
};

//acts as a hashmap(sort of) between a process and its priority
//and links them together
struct pair {
    struct proc process; //process
    int priority; //priority
};

//links pair with its arrival time
struct entry {
    struct pair p;//pair(process and its priority)
    int arrival_time;//time at which function arrives
};

//maintains a dynamic array for processes
//processes are stored inside this array
struct Heap {
    struct entry* arr;//array to store processes
    int size;//current size of array
    int capacity;//capacity or maximum size of array
};

//takes a program and makes it a process by running it according 
//to its priority
struct proc make_process(char* cmd, int priority) {
    struct proc p;

    //strcpy copies the cmd to p.cmd
    //if it fails and it equal to null
    if (strcpy(p.cmd, cmd) == NULL) {
        perror("Error in copying command");//then an error is printed
    }

    //initializes the attributes of process according to the input
    p.priority = priority;//sets priority
    p.execution_time = 0;//initially the execution time is zero
    p.wait_time = 0;

    //process is initialized and ready to execute
    //error checking for the same
    if (strcpy(p.state, "READY") == NULL) {
        perror("Error in copying state");
    }

    //returns the process
    return p;
}

// function to print the contents of the priority queue
void print_heap(struct Heap* heap) { 
    printf("\n");

    for (int i = 1; i <= heap->size; i++) {
        printf("%s %d %d\n", heap->arr[i].p.process.cmd, heap->arr[i].p.priority, heap->arr[i].p.process.pid);
    }
    printf("\n");
}

// Comparison function to compare two entries in the heap
int cmp_entries(struct entry e1, struct entry e2) {
    //compares the priority of the both processes
    if (e1.p.priority == e2.p.priority) {
        return e1.arrival_time < e2.arrival_time;
    }

    return e1.p.priority > e2.p.priority;
}

// Swap two entries in the heap
void exchange(struct Heap *heap, int i, int j) {
    struct entry temp = heap->arr[i];
    heap->arr[i] = heap->arr[j];
    heap->arr[j] = temp;
}

// Insert a new process into the priority queue
void insert(struct Heap *heap, struct proc x) {
    //checks if size is less than or equal to capacity
    if (heap->size == heap->capacity) {
        //if yes, throws heap overflow
        perror("Heap overflow");
        return;
    }

    heap->size++;//increase current size of heap
    struct pair p;//initialize the process
    p.process = x;
    p.priority = x.priority;
    heap->arr[heap->size].p = p;
    heap->arr[heap->size].arrival_time = heap->size; // Use a different arrival time mechanism

    int i = heap->size;

    while (i > 1 && cmp_entries(heap->arr[i], heap->arr[PARENT(i)])) {
        exchange(heap, i, PARENT(i));
        i = PARENT(i);
    }
}

// Find the process with the highest priority in the queue
struct proc find_max(struct Heap* heap) {
    if (heap->size == 0) {
        perror("Heap underflow");
    }

    return heap->arr[1].p.process;
}

// Reorganize the heap after extracting the maximum element
void Heapify(struct Heap* heap, int i) {
    int l = LEFT(i);
    int r = RIGHT(i);
    int largest = i;

    if (l <= heap->size && cmp_entries(heap->arr[l], heap->arr[i])) {
        largest = l;
    }
    if (r <= heap->size && cmp_entries(heap->arr[r], heap->arr[largest])) {
        largest = r;
    }

    if (largest != i) {
        exchange(heap, i, largest);
        Heapify(heap, largest);
    }
}

// Extract and return the process with the highest priority
struct proc extract_max(struct Heap* heap) {
    struct proc max = heap->arr[1].p.process;
    heap->arr[1] = heap->arr[heap->size--];
    Heapify(heap, 1);
    return max;
}

// Extract and return a process by its PID, reorganizing the heap afterward
struct proc extract_by_pid(struct Heap* heap, pid_t pid) {
    int i;

    for (i = 1; i <= heap->size; i++) {
        if (heap->arr[i].p.process.pid == pid) {
            break;
        }
    }

    if (i > heap->size) {
        perror("Process not found");
    }

    struct proc p = heap->arr[i].p.process;
    heap->arr[i] = heap->arr[heap->size--];
    Heapify(heap, i);
    return p;
}

 // Global declarations
struct Heap* q1 = NULL;
struct proc terminated_arr[100];
int num_terminated = 0;
int NCPU; //number of cores
int TSLICE; //Time slice duration for scheduling
pid_t pid_scheduler;

//for printint the array of terminated processes
void print_terminated_arr() {
    for (int i = 0; i < num_terminated; i++) {
        struct proc p = terminated_arr[i];
        
        printf("\n");
        printf("+--------------------+\n");
        printf("|Process Information:\n");
        printf("|  Command:       %s\n", p.cmd);
        printf("|  PID:           %d\n", p.pid);
        printf("|  Priority:      %d\n", p.priority);
        printf("|  Execution Time: %d\n", p.execution_time);
        printf("|  Wait Time:     %d\n", p.wait_time);
        printf("|  State:         %s\n", p.state);
        printf("+--------------------+\n");
        printf("\n");
    }
}


// ------------------ Timer APIs ------------------ //
// a structure for setting up interval timers
struct itimerval timer;

//start the timer with predefined Time-slice
void start_timer() {
    timer.it_value.tv_sec = TSLICE;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = TSLICE;
    timer.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("Error in setting timer");
    }
}
//stop the timer
void stop_timer() {
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("Error in setting timer");
    }
} 

// Signal handler for SIGINT. stops the scheduler when signal is received
void signal_handler(int signum){
    if(signum == SIGINT){
        kill(pid_scheduler,SIGINT);
    }
}

int isSubstring(const char *string, const char *substring){
    int string_length = strlen(string);
    int substring_length = strlen(substring);

    for(int i = 0; i<=string_length; i++){
        int j;
        for( j = 0; j<substring_length; j++){
            if(string[i+j] != substring[j]){
                break;
            }
        }
        if(j == substring_length){
            return 1;
        }
    }
    return 0;
}
// Signal handler for SIGALRM
void sig_alarm_handler(int signum){
    // if (signum == SIGCHLD) {
    //     pid_t pid;
    //     while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
    //         for (int i = 1; i <= q1->size; i++) {
    //             if (q1->arr[i].p.process.pid == pid) {
    //                 strcpy(q1->arr[i].p.process.state, "TERMINATED");
    //                 terminated_arr[num_terminated++] = q1->arr[i].p.process;
    //                 extract_by_pid(q1, pid);
    //                 break;
    //             }
    //         }
    //     }
    // }
    if(signum == SIGALRM){
        stop_timer();

        int n;
        int i;

        for(i=1; i<q1->size; i++){
            struct proc process = q1->arr[i].p.process;

            while(waitpid(process.pid, NULL, WNOHANG) == process.pid){
                if(strcpy(process.state, "TERMINATED") == NULL){
                    perror("Error copying state");
                }

                process.execution_time+=TSLICE;
                terminated_arr[num_terminated++] = process;
                extract_by_pid(q1,process.pid);
                i--;
                break;
            }

            if(strcmp(q1->arr[i].p.process.state, "RUNNING") == 0){
                if(strcpy(process.state,"READY") == NULL){
                    perror("Error copying state");
                }

                process.wait_time += TSLICE;
                kill(process.pid, SIGSTOP);
                extract_by_pid(q1,process.pid);
                insert(q1,process);
            }

            else{
                process.wait_time += TSLICE;
            }
        }
        n=1;
        i=1;

        while(n<=NCPU && i<=q1->size){
            struct proc process = q1->arr[i].p.process;

            if(strcmp(process.state,"READY") == 0){
                if(strcpy(process.state,"RUNNING") == NULL){
                    perror("Error copying state");
                }

                process.wait_time+=TSLICE;
                kill(process.pid, SIGCONT);
                n++;
            }
            i++;
        }
        start_timer();
    }
    else if(signum== SIGINT){
        stop_timer();
        print_terminated_arr();
        raise(SIGKILL);
        free(q1->arr);
        free(q1);
        // exit(0);
    }
}


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


    //runs if the process is a child process ; acts as the scheduler
    if(pid == 0){
        struct sigaction sig; //setting up our own signal handler to handle custom signals or the way we want them to execute
        memset(&sig, 0, sizeof(sig));
        sig.sa_handler = &sig_alarm_handler;
        sigaction(SIGALRM, &sig, NULL); //SIGALRM command handling
        sigaction(SIGINT, &sig, NULL); //SIGINT command handling

        //error in handling pipes
        if(close(pipefd[1]) == -1){
            perror("Error in closing pipe");
        }
        char exe[20];
        int priority;

        // Initialize priority queue (heap) with a fixed capacity
        q1 = (struct Heap*) malloc(sizeof(struct Heap));
        if(q1 == NULL){
            perror("Error allocating memory.");
        }

        q1->arr = (struct entry*) malloc(100*sizeof(struct entry));
        if(q1->arr == NULL){
            perror("Error allocating memory");
        }

        q1->size = 0;
        q1->capacity = 100; // maximum number of processes allowed in the heap

        start_timer();
        // scheduler's main loop for process management
        while(1){
            read(pipefd[0],input,100);
            sscanf(input,"submit %s %d", exe, &priority);
            struct proc p = make_process(exe,priority);

            pid_t pid_process = fork();

            if(pid_process == 0){
                raise(SIGSTOP); //suspend the child process until it is scheduled to run
            
                //execute the command using system
                if(system(p.cmd) == -1){
                    perror("Error executing command");
                }
            //notify the scheduler of the process completion
            kill(pid_scheduler, SIGCHLD);
            exit(0);
        }

        else{
            //set process ID and initial wait time for scheduling
            p.pid = pid_process;
            p.wait_time = TSLICE;
        }
        insert(q1,p); //Insert the new process into our priority queue
        }
    }
    else{
        //close unused read end of the pipe in the parent process
        if(close(pipefd[0]) == -1){
            perror("Error closing pipe");
        }
        pid_scheduler = pid; //storing scheduler PID

        while(shell_running){
            //custom prompt 
            printf("SimpleShell $ ");
            //reading user input
            if(fgets(input, 100, stdin) == NULL){
                perror("Error while reading input");
            }
            //checking if command starts with "submit", will not accept it otherwise
            if(!isSubstring(input,"submit")){
                printf("Invalid command\n");
                continue;
            }

            write(pipefd[1], input, strlen(input)+1);
    
        }
        //wait for the child (scheduler) process to complete execution
        if (waitpid(pid_scheduler, NULL, 0) == -1) {
            perror("Error waiting for child process");
    }
    }
}
