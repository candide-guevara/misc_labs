#include <common.h>

#include <assert.h>

#include <logger.h>

IGNORE_WARNING_PUSH("-Wunused-function")
// Use this to set breakpoints to trap assertion violations
void __my_assert__(int condition) {
  assert(condition);
}
IGNORE_WARNING_POP

