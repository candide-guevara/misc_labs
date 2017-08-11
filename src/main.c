#include <common.h>
#include <tests.h>

int main (void) {
  LOG_WARN("Starting ...");
  uint32_t graph_size = 64;

  // test_build_nodes();
  // test_graph_image_dump(graph_size);
  test_destructive_pointer_reversal_traversal(graph_size);
  // test_destructive_pointer_back_and_forth_traversal(graph_size);

  LOG_INFO("All done !!");
  return 0;
}

