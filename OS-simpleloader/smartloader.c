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



