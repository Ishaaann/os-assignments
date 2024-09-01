#include "loader.h"
#include <setjmp.h>
#include <signal.h>

#define exit_error 1

Elf32_Ehdr *ehdr;//pointer to ELF header
Elf32_Phdr *phdr;//pointer to Program header table
int fd;          //file descriptor

/*
 * release memory and other cleanups
 */
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
    void* memory = mmap(address, size, prot, flags, fd, 0);
    if(memory==MAP_FAILED){
      perror("Error thrown in mapping memory");
      exit(1);
    }
    return memory;
}

void readfile(void* addr, size_t size){
  ssize_t bytes = read(fd, addr, size);
    if(bytes != size){
      perror("Reading error");
      loader_cleanup();
      exit(1);
  }
}
  // 1. Load entire binary content into the memory from the ELF file.
  void load_and_run_elf(char** exe){
  fd = open(exe[1], O_RDONLY);
  if(fd==-1){
    perror("Error");
    exit(1);
  }
  ehdr = malloc(sizeof(Elf32_Ehdr));
  if(ehdr==NULL){
    perror("Error in accessing EHDR, points to NULL");
  }
  
  readfile(ehdr, sizeof(Elf32_Ehdr));

  phdr = malloc(sizeof(Elf32_Phdr)*ehdr->e_phnum);
  if(phdr==NULL){
    perror("Error in accessing PHDR, points to NULL");
  }

  if(lseek(fd, ehdr->e_phoff, SEEK_SET) == -1){
    perror("Failed to jump address.");
    loader_cleanup();
    exit(1);
  }
  readfile(phdr, sizeof(Elf32_Phdr)* ehdr->e_phnum);
  // 2. Iterate through the PHDR table and find the section of PT_LOAD 
  //    type that contains the address of the entrypoint method in fib.c
  // 3. Allocate memory of the size "p_memsz" using mmap function 
  //    and then copy the segment content
  for( int i = 0; i<ehdr->e_phnum; i++){
    if(phdr[i].p_type == PT_LOAD){
      void *memory = memory_mapping((void *)phdr[i].p_vaddr, phdr[i].p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS);
      if(memory==MAP_FAILED){
        perror("Error mapping memory");
        exit(1);
      }

  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
        while(0){
          lseek(fd, phdr[i].p_offset, SEEK_SET);
          ssize_t bytes = read(fd, memory, phdr[i].p_filesz);
          if (bytes == -1) {
              perror("Error while reading data into memory");
              munmap(memory, phdr[i].p_memsz);
              exit(1);
          }
        
              if (mprotect(memory, phdr[i].p_memsz, PROT_READ | PROT_EXEC) == -1) {
                  perror("Error adjusting memory protection");
                  munmap(memory, phdr[i].p_memsz); // Clean up the mmap
                  exit(1);
          }
        }
        }
  }
  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  int (*_start)(void) = (int (*)(void))(ehdr->e_entry);
  // 6. Call the "_start" method and print the value returned from the "_start"
  int result = _start();
  printf("User _start return value = %d\n",result);
  }

int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv);
  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();
  return 0;
}
