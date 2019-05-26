#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>

#define PAGE_2M     (1024*1024*2)
#define TOTAL_ALLOC (PAGE_2M * 4)
#define SEGMENT     (PAGE_2M / 64)
#define SEG_CNT     (TOTAL_ALLOC / SEGMENT)
#define ITEM_COUNT  (SEGMENT / 8)

int main(int argc, char *argv[]) {
  printf("parent pid : %d\n", getpid());
  uint64_t *buffer[SEG_CNT] = {NULL};
  uint64_t total = 0;

  if (argc == 1) {
    printf("using malloc\n");
    for (uint64_t i=0; i < SEG_CNT; ++i)
      buffer[i] = malloc(SEGMENT);
  }
  else {
    printf("using memalign\n");
    for (uint64_t i=0; i < SEG_CNT; ++i)
      buffer[i] = memalign(SEGMENT, SEGMENT);
  }

  for (uint64_t i=0; i < SEG_CNT; ++i) {
    //if (i && buffer[i-1] + ITEM_COUNT == buffer[i])
    //  printf("%lu and %lu are contiguous\n", i-1, i);
    //else if (i)
    //  printf("%lu : %p (%p) and %p NOT contiguous\n", 
    //         i, buffer[i-1], buffer[i-1] + ITEM_COUNT, buffer[i]);

    for (uint64_t j=0; j < ITEM_COUNT; ++j) 
      buffer[i][j] = argc;
  }

  while (!sleep(1)) {
    printf("scanning memory at %p (%lu)\n", buffer, total);
    for (uint64_t i=0; i < SEG_CNT; ++i)
    for (uint64_t j=0; j < ITEM_COUNT; ++j)
      total += buffer[i][j];
  }
  return 0;
}

