#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define PAGE_4k       4096
#define PAGE_2M       (1024*1024*2)
#define TOTAL_ALLOC   (4 * PAGE_2M)
#define ITEM_COUNT    (TOTAL_ALLOC / PAGE_4k)
const char* filepath = "/home/cguevara/mmap_file"; 
const pid_t FAIL_FORK = -1;

void write_stuff() {
  char buf[PAGE_4k] = {'a'};
  int fd = open(filepath, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);
  if (fd == -1) perror("cannot open file : ");
  for (uint64_t i=0; i<ITEM_COUNT; ++i) 
    if (write(fd, buf, PAGE_4k) == -1ll) {
      perror("failed writing : ");
      break;
    }
  if (close(fd)) perror("failed on close : ");
}

void file_shared(int argc) {
  char now = time(NULL);
  int fd = open(filepath, O_RDWR);
  if (fd == -1) perror("cannot open file : ");
  char *addr = mmap(NULL, TOTAL_ALLOC, 
                    PROT_READ|PROT_WRITE, 
                    MAP_SHARED, // MAP_HUGETLB only huge pages for anonymous  !!
                    fd, 0);
  if (addr == MAP_FAILED) perror("mmap failed : ");
  if (close(fd)) perror("failed on close : ");

  addr[1] = now;
  addr[PAGE_4k] = now;
  sleep(60);
  if (munmap(addr, TOTAL_ALLOC)) perror("munmap failed : ");

  fd = open(filepath, O_RDONLY);
  if (fd == -1) perror("cannot open file2 : ");
  char then = 0;
  lseek(fd, 1, SEEK_SET);
  read(fd, &then, 1);
  printf("now %d == then %d\n", now, then);
  if (close(fd)) perror("failed on close2 : ");
}

void file_private(int argc) {
  int fd = open(filepath, O_RDWR);
  if (fd == -1) perror("cannot open file : ");
  char *addr = mmap(NULL, TOTAL_ALLOC, 
                    PROT_READ|PROT_WRITE, 
                    MAP_PRIVATE, // MAP_HUGETLB only huge pages for anonymous  !!
                    fd, 0);
  if (addr == MAP_FAILED) perror("mmap failed : ");
  if (close(fd)) perror("failed on close : ");

  addr[0] = 'b';
  printf("addr[0] = %c\n", addr[0]);
  sleep(60);
  if (munmap(addr, TOTAL_ALLOC)) perror("munmap failed : ");

  fd = open(filepath, O_RDONLY);
  if (fd == -1) perror("cannot open file2 : ");
  char first_char = 0;
  read(fd, &first_char, 1);
  printf("first_char %c\n", first_char);
  if (close(fd)) perror("failed on close2 : ");
}

void anon_private(int argc) {
  uint64_t *addr = mmap(NULL, TOTAL_ALLOC, 
                        PROT_READ|PROT_WRITE, 
                        MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB, 
                        -1, 0);
  if (addr == MAP_FAILED) perror("mmap failed : ");
  addr[argc] = 666;
  printf("addr[0] = %lu\n", addr[0]);
  printf("addr[argc] = %lu\n", addr[argc]);
  sleep(60);
  if (munmap(addr, TOTAL_ALLOC)) perror("munmap failed : ");
}

void anon_private_memfd(int argc) {
  int fd = memfd_create("coco", MFD_HUGETLB);
  if (fd == -1) perror("failed memfd_create : ");
  if (ftruncate(fd, TOTAL_ALLOC)) perror("failed ftruncate : ");

  uint64_t *addr = mmap(NULL, TOTAL_ALLOC, 
                        PROT_READ|PROT_WRITE, 
                        MAP_PRIVATE|MAP_POPULATE, // populate will fault in all pages
                        fd, 0);
  if (addr == MAP_FAILED) perror("mmap failed : ");

  addr[argc] = 666;
  printf("addr[0] = %lu\n", addr[0]);
  printf("addr[argc] = %lu\n", addr[argc]);
  sleep(60);

  if (munmap(addr, TOTAL_ALLOC)) perror("munmap failed : ");
  if (close(fd)) perror("failed on close : ");
}

void child_routine(int argc, uint64_t *addr) {
  printf("child pid : %d\n", getpid());
  sleep(1);
  printf("addr[argc] = %lu\n", addr[argc]);
  addr[argc] = 999;
  if (munmap(addr, TOTAL_ALLOC)) perror("munmap failed : ");
  sleep(60);
  printf("CHILD done !\n");
}

void anon_shared_forked(int argc) {
  uint64_t *addr = mmap(NULL, TOTAL_ALLOC, 
                        PROT_READ|PROT_WRITE, 
                        MAP_SHARED|MAP_ANONYMOUS|MAP_HUGETLB, 
                        -1, 0);
  if (addr == MAP_FAILED) perror("mmap failed : ");
  addr[argc] = 666;
  
  pid_t child = fork();
  if (child == FAIL_FORK) perror("failed fork : ");

  if (child) {
    int child_stx;
    addr[argc] = 333;

    if (wait(&child_stx) == FAIL_FORK) perror("failed wait : ");
    if (!WIFEXITED(child_stx) || WEXITSTATUS(child_stx)) perror("child error exit : ");

    printf("addr[argc] = %lu\n", addr[argc]);
    if (munmap(addr, TOTAL_ALLOC)) perror("munmap failed : ");
  }
  else child_routine(argc, addr);
}

void child_routine_shmopen(int fd, uint64_t *addr) {
  fd = shm_open("/coco", O_RDWR, S_IRUSR|S_IWUSR);

  uint64_t *addr2 = mmap(NULL, TOTAL_ALLOC, 
                         PROT_READ|PROT_WRITE, 
                         MAP_SHARED,
                         fd, 0);
  if (addr2 == MAP_FAILED) perror("mmap child failed : ");

  printf("addr2[0] = %lu\n", addr2[0]);

  if (munmap(addr2, TOTAL_ALLOC)) perror("munmap failed3 : ");
  if (close(fd)) perror("failed on close2 : ");
  printf("CHILD done !\n");
}

void anon_shared_shmopen(int argc) {
  //if (shm_unlink("/coco")) perror("failed shm_unlink : ");
  int fd = shm_open("/coco", O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
  if (fd == -1) perror("failed shm_open : ");
  if (ftruncate(fd, TOTAL_ALLOC)) perror("failed ftruncate : ");

  uint64_t *addr = mmap(NULL, TOTAL_ALLOC, 
                        PROT_READ|PROT_WRITE, 
                        MAP_SHARED|MAP_POPULATE, // cannot use huge pages !
                        fd, 0);
  if (addr == MAP_FAILED) perror("mmap failed : ");
  addr[0] = 666;
  sleep(60);
  if (munmap(addr, TOTAL_ALLOC)) perror("munmap failed : ");
  if (close(fd)) perror("failed on close : ");
  
  pid_t child = fork();
  if (child == FAIL_FORK) perror("failed fork : ");

  if (child) {
    int child_stx;
    if (wait(&child_stx) == FAIL_FORK) perror("failed wait : ");
    if (!WIFEXITED(child_stx) || WEXITSTATUS(child_stx)) perror("child error exit : ");
    if (shm_unlink("/coco")) perror("failed shm_unlink : ");
  }
  else child_routine_shmopen(fd, addr);
}

int main(int argc, char *argv[]) {
  int par_pid = getpid();
  printf("parent pid : %d\n", par_pid);
  //write_stuff();
  //anon_private(argc);
  file_private(argc);
  //file_shared(argc);
  //anon_shared_forked(argc);
  //anon_private_memfd(argc);
  //anon_shared_shmopen(argc);
  if (par_pid == getpid()) printf("ALL done !\n");
  return 0;
}

