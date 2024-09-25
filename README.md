# Simple Shell in C
## a UNIX shell in C from scratch
### how to run
1) make sure you are in the `SimpleShell` directory
2)  compile the shell by running the makefile
```bash
make
```
3) then simply run the shell by executing
```bash
./shell
```
4) the prompt will be displayed
```bash
ishaan&&divyansh@SimpleShell:~$
```
5) execute any commands!
6) to exit, type `exit` or press `[CTRL-C]`
###### sample files fib, helloworld and a commands.sh containing a list of commands are given for testing purposes.
### Implementation
The entire code is written in the C Language, in the file `simpleshell.c`.
1. Shell Interface
The shell continuously prompts the user with a custom prompt (`ishaan&&divyansh@SimpleShell:~$`) and reads input using the `read_command()` function.
Commands are executed by either running them directly, handling cd for changing directories, supporting pipes (`|`), or accessing the command history with history.
Input commands are stored in the history_entries array, which tracks the command, process ID (PID), start time, and duration of execution.
2. History Management
The shell stores up to 100 command history entries. The history includes the command, PID, start time, and execution duration.
The history command prints all previously executed commands.
On pressing `[CTRL+C]`, the shell prints all commands in the history, along with their PIDs and runtime information.
3. Piping Implementation
The shell supports piping (`|`), allowing multiple commands to be executed where the output of one command is passed as input to the next. The function `execute_pipe()` breaks the command by pipes and runs them sequentially using child processes.
4. Command Execution (includes BONUS `&` implementation)
Commands are executed by forking a child process and using `execl()` for execution. Background processes are handled by appending `&` to commands, which runs them without blocking the shell.
The execution status, PID, and runtime are recorded in the command history.
5. Signal Handling
The shell includes signal handling for `SIGINT` (`CTRL+C`). When this signal is triggered, it interrupts the shell and prints the history of commands before exiting.

### Limitations
1. we have explicitly handled  `&` and pipe commands with `execvp`. attempting to run built-in shell commands in background or in pipes would lead to something like an execvp error and might print unexpected output. e.g. `history | wc` will not work
## [Link to repo](https://github.com/Ishaaann/os-assignments)
