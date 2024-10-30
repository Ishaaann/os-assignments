# Code Explanation: Multi-Process Scheduling System

This code implements a basic multi-process scheduling system with priority-based scheduling. It uses `fork` to create child processes for task execution and a heap (priority queue) to manage these tasks based on priority. Below is a detailed explanation of its implementation and flow.

## 1. Initialization and Argument Parsing
- **Arguments**: The program starts by taking two command-line arguments:
   - `NCPU`: Number of processes (CPUs) available.
   - `TSLICE`: Time slice (quantum) determining how long each process will run before context switching.
- **Pipe Setup**: A pipe is created for communication between the parent and child processes, enabling commands to be sent from the parent (user input) to the child (scheduler).

## 2. Process Creation with `fork`
- **Forking the Scheduler Process**:
   - The program forks itself to create a child process acting as the **scheduler**, responsible for managing and scheduling other processes.
   - The `pid` variable holds the result of `fork`. If `pid` is 0, the code is in the child process (scheduler). If non-zero, it's in the parent process (input shell).

## 3. Scheduler Process (Child)
- **Signal Handling**:
   - The scheduler (child process) sets up signal handlers using `sigaction` to handle signals like `SIGALRM` (for timing and scheduling) and `SIGINT` (for interrupts).
   - A custom handler function `sig_alarm_handler` is defined to control how the scheduler responds to these signals.
- **Pipe Communication**:
   - The scheduler closes the write end of the pipe since it only reads from the parent process.
- **Priority Queue Initialization**:
   - The scheduler initializes a heap (priority queue) to store processes with their respective priorities. The queue (`q1`) is dynamically allocated with a capacity of 100, which is the maximum number of processes it can hold.
- **Scheduling Loop**:
   - The scheduler enters an infinite loop, constantly checking for new processes submitted by the parent.
   - It reads input from the pipe, expecting a `"submit"` command format (e.g., `submit command_name priority`).
   - After parsing, it creates a new process (`struct proc`) and forks a new process specifically for executing this command.
  
   - **Process Execution**:
      - The newly forked child process calls `raise(SIGSTOP)` to stop itself immediately after creation, remaining paused until the scheduler is ready to run it.
      - Once resumed, the child process executes the command using `system(p.cmd)`.
      - After execution, it sends a signal (`SIGCHLD`) to the scheduler to notify completion.
      - The process then exits.

   - **Inserting Process into the Queue**:
      - The newly created process is added to the priority queue based on its priority, allowing the scheduler to later pick processes with higher priority for execution.

## 4. Input Shell Process (Parent)
- **Pipe Communication**:
   - The parent process closes the read end of the pipe, as it only writes commands to the child process (scheduler).
   - `pid_scheduler` stores the scheduler's PID for managing signals.
- **Shell Loop**:
   - The parent enters a loop that serves as a simple shell for receiving commands from the user.
   - It displays a prompt (`SimpleShell $`) and reads user input.
   - Only commands beginning with `"submit"` are valid, indicating a new task for scheduling. Invalid commands display an error message.
   - Valid commands are written to the pipe for the scheduler to read.
  
- **Waiting for Scheduler Termination**:
   - When `shell_running` is set to false, the parent exits the shell loop and waits for the scheduler to complete by calling `waitpid(pid_scheduler)`, ensuring clean program termination.

## 5. Key Functionality Overview
- **Command Submission**:
   - The parent process receives commands from the user, parses them, and sends them to the scheduler through a pipe.
- **Process Creation and Management**:
   - The scheduler, upon receiving a command, creates a process with the specified command and priority.
- **Priority-Based Scheduling**:
   - The scheduler uses a heap to manage processes by priority, allowing higher-priority processes to be scheduled first.
   - Processes are inserted into the priority queue, and the scheduler uses the time slice (`TSLICE`) for managing process execution time.
- **Signal Handling and Inter-Process Communication**:
   - The scheduler uses signals to manage timing (`SIGALRM`) and handle interrupts (`SIGINT`). Each process signals the scheduler upon completion.

## 6. Execution Flow Summary
1. **Program Start**: The main function begins with parsing arguments and setting up the pipe.
2. **Fork Scheduler**: The program forks into two processes — the parent (input shell) and the child (scheduler).
3. **Scheduler (Child Process)**:
   - Sets up signal handlers.
   - Reads commands from the pipe.
   - Creates and schedules processes based on priority.
4. **Input Shell (Parent Process)**:
   - Reads commands from the user and writes valid ones to the pipe for the scheduler.
   - Waits for the scheduler to finish execution before exiting.

This code structure facilitates a priority-based scheduling system, where processes are queued and executed based on their priority. The use of signals allows for flexible control over timing and process lifecycle management.
