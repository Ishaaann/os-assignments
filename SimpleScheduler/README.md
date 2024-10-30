# Simple Scheduler
# Divyansh Kumar Gautam (2023208) | Ishaan Raj (2023248)

This code implements a simple multi-process scheduling system using priority-based scheduling and signal handling. Below is a brief overview of its implementation:

## 1. Initialization and Argument Parsing
- **Arguments**: The program takes two command-line arguments:
   - `NCPU`: Number of CPUs (processes).
   - `TSLICE`: Time slice quantum for each process.
- **Pipe Setup**: A pipe is created for communication between the parent (shell) and child (scheduler) processes.

## 2. Process Creation with `fork`
- **Fork Scheduler Process**:
   - `fork` is used to create a child process as the **scheduler**, which manages task scheduling and execution.
   - The `pid` variable determines whether the process is a parent (shell) or child (scheduler).

## 3. Scheduler Process (Child)
- **Signal Handling**:
   - The scheduler sets up custom signal handlers for `SIGALRM` and `SIGINT`.
- **Priority Queue**:
   - Initializes a priority queue (heap) for managing processes by priority.
- **Scheduling Loop**:
   - The scheduler reads input from the pipe, parses commands, and forks a new child process for each task.
   - Each task is added to the priority queue, where it waits based on priority until scheduled.

## 4. Input Shell Process (Parent)
- **Command Input**:
   - The parent (shell) prompts the user for commands, accepts `submit command priority` format, and writes them to the pipe.
   - Invalid commands display an error.
- **Waiting for Scheduler**:
   - After exiting the shell loop, the parent waits for the scheduler to terminate cleanly.

## 5. Flow Summary
1. **Main Start**: Parses arguments and sets up the pipe.
2. **Scheduler (Child)**: Reads commands from the pipe, creates and queues tasks by priority.
3. **Shell (Parent)**: Receives user commands, sends valid ones to the scheduler, and waits for scheduler termination.

# Contribution
Both of us mutually contributed equally . We worked together over discord and discussed and coded everything together.
[**Github repo link**](https://github.com/Ishaaann/os-assignments)
