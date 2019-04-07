#include <stdlib.h>
#include <stdio.h>

#include <common.h>
#include <logger.h>

int main (void) {
  LOG_WARN("Starting ...");
  LOG_INFO("Testing %d", 666);
  ASSERT(0, "test");
  LOG_INFO("All done !!");
  return 0;
}

