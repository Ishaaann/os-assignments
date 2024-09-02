# os-assignments
# Team Members : 
- Ishaan Raj (2023248)
- Divyansh Kumar Gautam(2023208)

# Implementation:
This C code implements a simple ELF (Executable and Linkable Format) loader that loads and executes a 32-bit ELF executable file. 
Global variables Ehdr and phdr are declared which point to ELF Header and ELF program header. Fd represents the file descriptor.
Loader_cleanup function frees the space occupied by Ehdr and phdr in case of any errors and then closes the fd.
Memory_mapping function uses the mmap function and maps the file into the memory, if mapping fails then it throws and error and exits the process else returns the pointer to the address.
Readfile reads the bytes of the file passed as a parameter and further calls loader_cleanup in case of any error and exits the process.
The function then reads the program header table after obtaining the header information. All of the program's segments that require loading are listed in this table. After that, the function loops over these segments, locating the ones that have the ptype as PT_LOAD. The code provides the proper permissions, including making it executable if needed, for each of these segments after allocating memory and reading the segment from the file into that memory.

The program's entry point, or the memory address where execution should start, is finally found by the code. To begin the program, typecasts the address to that of function pointer matching "_start" method in fib.c
The code prints the result that the program returned when it has finished running. Hence completing the job of a loader.

# Contribution:
Both members contributed to the assignment equally and with passion. We both worked on coding the assignment together at all times including the brainstorming, writing the code and debugging it.
