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
    int l = LEFT(i);//storing indices
    int r = RIGHT(i);//storing indices
    int largest = i;//storing indices

    //checks if the left child is less than the size of the heap
    //and compares the entries for priority
    if (l <= heap->size && cmp_entries(heap->arr[l], heap->arr[i])) {
        largest = l; //if yes, then largest is set to left
    }
    //checks if the right child is less than the size of the heap
    //and compares the entries for priority
    if (r <= heap->size && cmp_entries(heap->arr[r], heap->arr[largest])) {
        largest = r; //if yes, then largest is set to right
    }

    //if the largest is not equal to i
    if (largest != i) {
        //exchange the entries
        exchange(heap, i, largest);
        //recursively call the function
        //maintianing the heap property
        Heapify(heap, largest);
    }
}

// Extract and return the process with the highest priority
struct proc extract_max(struct Heap* heap) {
    struct proc max = heap->arr[1].p.process;
    heap->arr[1] = heap->arr[heap->size--];
    //maintain the heap property
    Heapify(heap, 1);
    return max; //return the process with the highest priority
}

// Extract and return the process with the given PID
struct proc extract_by_pid(struct Heap* heap, pid_t pid) {
    int i;
    //search for the process with the given PID
    for (i = 1; i <= heap->size; i++) {
        if (heap->arr[i].p.process.pid == pid) {
            break;
        }
    }
    //if the process is not found
    if (i > heap->size) {
        //throw an error
        perror("Process not found");
    }

    //return the process
    struct proc p = heap->arr[i].p.process;
    heap->arr[i] = heap->arr[heap->size--];
    //maintain the heap property
    Heapify(heap, i);
    return p; //return the process
}

 // declarations
struct Heap* q1 = NULL; //initialize the heap, q1 is the ready queue, queue for processes that are ready to run
struct proc terminated_arr[100];//array to store the terminated processes
int num_terminated = 0;//number of terminated processes
int NCPU; //number of cores
int TSLICE; //Time slice
pid_t pid_scheduler;//scheduler process ID

// Function to print the terminated processes
void print_terminated_arr() {
    //iterate through the terminated processes
    for (int i = 0; i < num_terminated; i++) {
        //print the process information
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

// Timer structure
struct itimerval timer;

// Start the timer
void start_timer() {
    timer.it_value.tv_sec = TSLICE;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = TSLICE;
    timer.it_interval.tv_usec = 0;

    // Set the timer
    // error checking for the same
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        //if error, print the error
        perror("Error in setting timer");
    }
}

// Stop the timer
void stop_timer() {
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    // Set the timer
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        //if error, print the error
        perror("Error in setting timer");
    }
} 

//custom signal handler for SIGINT, when CTRL+C is pressed
void signal_handler(int signum){
    //if the signal is SIGINT
    if(signum == SIGINT){
        //kill the scheduler process
        //terminate the process
        kill(pid_scheduler,SIGINT);
    }
}

// Function to check if a string is a substring of another string
//used for checking the input command for submit command
int isSubstring(const char *string, const char *substring){
    int string_length = strlen(string); //length of the string
    int substring_length = strlen(substring); //length of the substring

    //iterate through the string
    for(int i = 0; i<=string_length; i++){
        //iterate through the substring
        //j is the index of the substring
        int j;
        for( j = 0; j<substring_length; j++){
            //if the substring is not equal to the string
            if(string[i+j] != substring[j]){
                //break the loop
                break;
            }
        }
        //if j reaches the substring length
        if(j == substring_length){
            return 1; //return 1
        }
    }
    return 0; //return 0
}

// Function to handle the SIGALRM signal
void sig_alarm_handler(int signum){
    //if the signal is SIGALRM
    if(signum == SIGALRM){
        stop_timer();//stop the timer

        //declare variables
        int n;
        int i;

        //iterate through the processes in the ready queue
        for(i=1; i<q1->size; i++){
            struct proc process = q1->arr[i].p.process;
            //check if the process is terminated
            //WNOHANG returns immediately if no child has exited
            while(waitpid(process.pid, NULL, WNOHANG) == process.pid){
                //if yes, copy the state as terminated
                if(strcpy(process.state, "TERMINATED") == NULL){
                    //if error, print the error
                    perror("Error copying state");
                }

                //increment the execution time by the time slice, cuz the process will consume the whole time slice
                process.execution_time+=TSLICE;
                //add the process to the terminated array
                terminated_arr[num_terminated++] = process;
                extract_by_pid(q1,process.pid); //extract the process from the ready queue
                i--; //decrement the index
                break; //break the loop after done
            }

            //if the process is running
            if(strcmp(q1->arr[i].p.process.state, "RUNNING") == 0){
                //copy the state as ready
                if(strcpy(process.state,"READY") == NULL){
                    //if error, print the error
                    perror("Error copying state");
                }
                
                //increment the wait time by the time slice as the process is waiting and will consume the whole time slice
                process.wait_time += TSLICE;
                kill(process.pid, SIGSTOP); //stop the process
                extract_by_pid(q1,process.pid);//extract the process from the heap
                insert(q1,process);//insert the process back to the heap
            }

            else{
                process.wait_time += TSLICE;//increment the wait time by the time slice
            }
        }
        n=1;
        i=1;

        //iterate through the processes in the ready queue
        while(n<=NCPU && i<=q1->size){
            //get the process
            struct proc process = q1->arr[i].p.process;

            //if the process is ready
            if(strcmp(process.state,"READY") == 0){
                //copy the state as running
                if(strcpy(process.state,"RUNNING") == NULL){
                    //if error, print the error
                    perror("Error copying state");
                }

                process.wait_time+=TSLICE;//increment the wait time by the time slice
                kill(process.pid, SIGCONT);//continue the process
                n++;//increment the counter
            }
            i++;//increment the index
        }
        start_timer();//start the timer again
    }
    //if the signal is SIGINT
    else if(signum== SIGINT){        
        stop_timer();//stop the timer
        print_terminated_arr();//print the terminated processes at the time of termination
        raise(SIGKILL);//raise the kill signal
        free(q1->arr);//free the memory 
        free(q1);//free the memory 
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

        //initialize the heap
        q1 = (struct Heap*) malloc(sizeof(struct Heap));
        //error checking for the same
        if(q1 == NULL){
            perror("Error allocating memory.");
        }

        //initialize the array
        q1->arr = (struct entry*) malloc(100*sizeof(struct entry));
        //error checking for the same
        if(q1->arr == NULL){
            //if error, print the error
            perror("Error allocating memory");
        }

        //initialize the size and capacity of the heap
        q1->size = 0;//set the size to 0 initially
        q1->capacity = 100;//set the capacity to 100 initially

        start_timer();//start the timer

        //creating pipes for the processes
        while(1){
            read(pipefd[0],input,100);//read the input from the pipe
            sscanf(input,"submit %s %d", exe, &priority);//scan the input
            struct proc p = make_process(exe,priority);//make the process according to the input

            pid_t pid_process = fork();//fork a child process

            //if the process is a child process
            if(pid_process == 0){
                raise(SIGSTOP); //suspend the child process until it is scheduled to run
            
                //execute the command
                if(system(p.cmd) == -1){
                    perror("Error executing command");
                }
            //kill the scheduler process
            kill(pid_scheduler, SIGCHLD);
            exit(0);//exit the process
            }
            //if the process is a parent process
            else{
                p.pid = pid_process;//set the process ID
                p.wait_time = TSLICE;//set the wait time to the time slice
            }
            insert(q1,p);//insert the process into the ready queue
        }
    }
    //if the process is a parent process
    else{
        //error in handling pipes
        if(close(pipefd[0]) == -1){
            perror("Error closing pipe");
        }
        pid_scheduler = pid;//set the scheduler process ID
        
        //while the shell is running
        while(shell_running){
            printf("SimpleShell $: ");//custom prompt
            //read the input
            if(fgets(input, 100, stdin) == NULL){
                perror("Error while reading input");
            }
            //if the input is not submit command
            if(!isSubstring(input,"submit")){
                //print invalid command
                printf("Invalid command\n");
                continue;
            }
            //write the input to the pipe
            write(pipefd[1], input, strlen(input)+1);
    
        }
    }
}
