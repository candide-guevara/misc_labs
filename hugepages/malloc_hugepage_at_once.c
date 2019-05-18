#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>

const uint64_t PAGE_2M = 1024*1024*2;
const uint64_t TOTAL_ALLOC = 4 * PAGE_2M;
const uint64_t ITEM_COUNT = TOTAL_ALLOC / 8;

int main(int argc, char *argv[]) {
  uint64_t *buffer = NULL;
  uint64_t total = 0;

  if (argc == 1) {
    buffer = malloc(TOTAL_ALLOC);
    printf("using malloc\n");
  }
  else {
    buffer = memalign(PAGE_2M, TOTAL_ALLOC);
    printf("using memalign\n");
  }

  if (madvise(buffer, TOTAL_ALLOC, MADV_HUGEPAGE)) {
    perror("madvise_huge");
  }

  for (uint64_t i=0; i < ITEM_COUNT; ++i)
    buffer[i] = argc;

  while (!sleep(1)) {
    printf("scanning memory at %p (%lu)\n", buffer, total);
    for (uint64_t i=0; i < ITEM_COUNT; ++i)
      total += buffer[i];
  }
  return 0;
}

