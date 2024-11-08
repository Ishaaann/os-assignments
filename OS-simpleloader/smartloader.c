#include "loader.h"
#include <setjmp.h>
#include <signal.h>

#define error 1

Elf32_Ehdr *ehdr;//pointer to ELF header
Elf32_Phdr *phdr;//pointer to Program header table
int fd;          //file descriptor

int pf = 0;//number of page faults
int pa = 0;//number of page allocations
int inf = 0;//number of internal fragmentation

sigjmp_buf page_fault_jump;

void loader_cleanup() {
  if(phdr){
    free(phdr);
    phdr = NULL;
  }
  if(ehdr){
    free(ehdr);
    ehdr = NULL;
  }
  if(fd!=-1)
    close(fd);
}

void* memory_mapping(void* address, size_t size, int prot, int flags){
    void* memory = mmap(address, size, prot, flags, -1, 0);
    if(memory==MAP_FAILED){
      perror("Error thrown in mapping memory");
      exit(1);
    }
    return memory;
}

void handle_page_fault(int signo, siginfo_t *si, void *context){
    pf++;
    void *addr = si->si_addr;

    int i;
    for(int i = 0; i<ehdr->e_phnum; i++){
        void* start = (void*)phdr[i].p_vaddr;
        void* end = start + phdr[i].p_memsz;

        if(addr>= start && addr<=end){
            int k = 0;
            while((phdr[i].p_memsz - (k*4096))>4096){
                void *memory = memory_mapping((void *)phdr[i].p_vaddr + (k*4096), 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS);
                setting_seek(phdr[i].p_offset);
                ssize_t bytes = read(fd, memory, phdr[i].p_filesz);
                if(bytes == -1){
                    perror("Error reading data into memory");
                    munmap(memory, phdr[i].p_memsz);
                    exit(1);
                }
                pf++;
                k++;
            }
            void *memory = memory_mapping((void *)phdr[i].p_vaddr + (k*4096), phdr[i].p_memsz - (k*4096), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS);
            setting_seek(phdr[i].p_offset);
            ssize_t bytes = read(fd, memory, phdr[i].p_filesz);
            if(bytes == -1){
                perror("Error reading data into memory");
                munmap(memory, phdr[i].p_memsz);
                exit(1);
            }
            k++;

            inf += (k*4096) - phdr[i].p_memsz;
            pa += k;
            break;
        }
    }
    return;
}
//helper function to change the position of the filepointer inside the file
void seek(int offset) {
    if (lseek(fd, offset, SEEK_SET) == -1) {
        perror("lseek failed");
        loader_cleanup();
        exit(1);
    }
}

//helper function to read data from the file
void readfile(void *dest, size_t size) {
    ssize_t bytes_read = read(fd, dest, size);
    if (bytes_read != size) {
        perror("read error");
        loader_cleanup();
        exit(1);
    }
}
void load_and_run_elf(char **exe){
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = handle_page_fault;
  //sigaction error handler  
  if (sigaction(SIGSEGV, &sa, NULL) == -1) { 
    perror("sigaction failed");
    exit(1);
}

fd = open(exe[1], O_RDONLY);
if (fd==-1){
  perror("Error opening file");
  exit(1);
} 
ehdr = malloc(sizeof(Elf32_Ehdr));
if (ehdr == NULL) {
  perror("Corrupt ehdr");
  exit(1);
}

readfile(ehdr, sizeof(Elf32_Ehdr));

phdr = malloc(sizeof(Elf32_Phdr)*ehdr->e_phnum);
if(phdr == NULL){
  perror("Corrupt phdr");
  exit(1);
}

seek(ehdr->e_phoff);
readfile(phdr, sizeof(Elf32_Phdr)*ehdr->e_phnum);

int (*_start)(void) = (int (*)(void))(ehdr->e_entry);
int result;

// Lazy Loading
result = _start();
printf("_start return value = %d\n", result);

//Reporting page faults and page allocations
printf("Page faults = %d\n", pf);
printf("Page allocations = %d\n", pa);
printf("Internal fragmentation = %f KB\n", (float) inf/1024.0);

//cleanup
loader_cleanup();
}

int main(int argc, char **argv){
  if (argc != 2) {
    printf("Usage: %s <ELF executable>\n", argv[0]);
    exit(1);
  }

  load_and_run_elf(argv);

  return 0;
}


