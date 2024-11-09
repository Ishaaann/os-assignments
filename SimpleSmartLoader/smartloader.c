#include "loader.h"
#include <setjmp.h>
#include <signal.h>

#define error 1

// Global pointers for ELF and Program Header
Elf32_Ehdr *ehdr; // Pointer to ELF header
Elf32_Phdr *phdr; // Pointer to Program Header Table
int fd;           // File descriptor for the ELF file

// Variables to track memory management statistics
int pf = 0;  // Number of page faults
int pa = 0;  // Number of page allocations
int inf = 0; // Total internal fragmentation (in bytes)

// Jump buffer for handling page faults
sigjmp_buf page_fault_jump;


//Cleans up allocated resources like file descriptors, ELF headers, and program headers.
 void loader_cleanup() {
    if (phdr) {
        free(phdr);
        phdr = NULL;
    }
    if (ehdr) {
        free(ehdr);
        ehdr = NULL;
    }
    if (fd != -1)
        close(fd);
}
 
void* memory_mapping(void* address, size_t size, int prot, int flags) {
    void* memory = mmap(address, size, prot, flags, -1, 0);
    if (memory == MAP_FAILED) {
        perror("Error thrown in mapping memory");
        exit(1);
    }
    return memory;
}

//Signal handler for page faults (SIGSEGV).
void handle_page_fault(int signo, siginfo_t *si, void *context) {
    if (signo == SIGSEGV) {
        pf++;  // Increment page fault counter
        pa++;  // Increment page allocation counter

        // Get the faulting address
        void *addr = si->si_addr;

        // Check if the fault address falls within any segment
        for (int i = 0; i < ehdr->e_phnum; i++) {
            void* start = (void*)(phdr[i].p_vaddr);
            void* end = start + phdr[i].p_memsz;

            // Check if the fault address is within the segment range
            if (addr < end && addr >= start) {
                int num_pages = (phdr[i].p_memsz + 4095) / 4096; // Total pages needed
                int index = ((uintptr_t)addr - (uintptr_t)start) / 4096; // Page index
                void* page = mmap(start + index * 4096, 4096, 
                                  PROT_READ | PROT_WRITE | PROT_EXEC, 
                                  MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, fd, 
                                  phdr[i].p_offset + index * 4096); // Map the faulting page

                // Check if page mapping failed
                if (page == MAP_FAILED) {
                    perror("Error mapping page");
                    exit(1);
                }

                // Seek to the appropriate file offset and read the segment data into memory
                seek(phdr[i].p_offset + index * 4096);
                int remaining = phdr[i].p_filesz - index * 4096;  
                int bytes = remaining > 4096 ? 4096 : remaining; // Bytes to read

                // Read data from file to the allocated page
                readfile(page, bytes);

                // Calculate internal fragmentation if this is the last page of the segment
                if (addr + 4096 > end) {
                    inf += 4096 - (end - addr);
                }
                return;
            }
        }
    }
}

//Helper function to move the file pointer within the ELF file.
void seek(int offset) {
    if (lseek(fd, offset, SEEK_SET) == -1) {
        perror("lseek failed");
        loader_cleanup();
        exit(1);
    }
}

// Helper function to read data from the ELF file into a buffer.
void readfile(void *dest, size_t size) {
    ssize_t bytes_read = read(fd, dest, size);
    if (bytes_read != size) {
        perror("read error");
        loader_cleanup();
        exit(1);
    }
}

// Loads an ELF executable into memory, sets up signal handlers for page faults, and executes the entry point of the ELF file.
void load_and_run_elf(char **exe) {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_page_fault;

    // Register the SIGSEGV handler
    if (sigaction(SIGSEGV, &sa, NULL) == -1) { 
        perror("sigaction failed");
        exit(1);
    }

    // Open the ELF file
    fd = open(exe[1], O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }

    // Read the ELF header
    ehdr = malloc(sizeof(Elf32_Ehdr));
    if (ehdr == NULL) {
        perror("Corrupt ehdr");
        exit(1);
    }
    readfile(ehdr, sizeof(Elf32_Ehdr));

    // Read the Program Header Table
    phdr = malloc(sizeof(Elf32_Phdr) * ehdr->e_phnum);
    if (phdr == NULL) {
        perror("Corrupt phdr");
        exit(1);
    }
    seek(ehdr->e_phoff);
    readfile(phdr, sizeof(Elf32_Phdr) * ehdr->e_phnum);

    // Execute the entry point of the ELF file
    int (*_start)(void) = (int (*)(void))(ehdr->e_entry);
    int result;

    // Start the loaded program
    result = _start();
    printf("_start return value = %d\n", result);

    // Reporting statistics
    printf("Page faults = %d\n", pf);
    printf("Page allocations = %d\n", pa);
    printf("Internal fragmentation = %f KB\n", (float)inf / 1024.0);

    // Cleanup resources
    loader_cleanup();
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <ELF executable>\n", argv[0]);
        exit(1);
    }

    // Load and run the specified ELF executable
    load_and_run_elf(argv);

    return 0;
}
