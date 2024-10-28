#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/time.h>

int PARENT(int i){
    return i/2;
}
int LEFT(int i){
    return 2*i;
}
int RIGHT(int i){
    return 2*i+1;
}
struct proc {
    char cmd[100];
    pid_t pid;
    int priority;
    int execution_time;
    int wait_time;
    char state[10]; // READY or RUNNING
    // process gets dumped in the terminated_arr after the process gets terminated
};

struct pair {
    struct proc process;
    int priority;
};

struct entry {
    struct pair p;
    int arrival_time;
};

struct Heap {
    struct entry* arr;
    int size;
    int capacity;
};

struct proc make_process(char* cmd, int priority) {
    struct proc p;

    if (strcpy(p.cmd, cmd) == NULL) {
        perror("Error in copying command");
    }

    p.priority = priority;
    p.execution_time = 0;
    p.wait_time = 0;


    if (strcpy(p.state, "READY") == NULL) {
        perror("Error in copying state");
    }


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
    if (heap->size == heap->capacity) {
        perror("Heap overflow");
        return;
    }

    heap->size++;
    struct pair p;
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

 // declarations
struct Heap* q1 = NULL;
struct proc terminated_arr[100];
int num_terminated = 0;
int NCPU; //number of cores
int TSLICE; //Time slice
pid_t scheduler_pid;

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

// ------------------ Priority Queue APIs ------------------ //

// ------------------ Timer APIs ------------------ //

struct itimerval timer;

void start_timer() {
    timer.it_value.tv_sec = TSLICE;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = TSLICE;
    timer.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("Error in setting timer");
    }
}

void stop_timer() {
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("Error in setting timer");
    }
} 


void signal_handler(int signum){
    if(signum = SIGINT){
        kill(pid_scheduler,SIGINT);
    }
}

int isSubstring(const char *string, const char *substring){
    int string_length = strlen(string);
    int substring_length = strlen(substring)

    for(int i = 0; i<=string_length; i++){
        int j;
        for( j = 0; j<substring_length; i++){
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

void sig_alarm_handler(int signum){
    if(signum == SIGALRM){
        stop_timer();

        int n;
        int i;

        for(i=1; i<q1->size; i++){
            struct proc process = q1->arr[i].p.process;

            while(wait_pid(process.pid, NULL, WNOHANG) == process.pid){
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
                extract_by_pid(q1,process,pid);
                insert(q1,process);
            }
        }
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
