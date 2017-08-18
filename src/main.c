#include <logger.h>
#include <common.h>

#include <special_traversals.h>
#include <tests.h>
#include <stress.h>

#pragma GCC diagnostic ignored "-Wunused-variable"
int main (int argc, const char **argv) {
  LOG_WARN("Starting ...");
  uint32_t graph_size = 128 * 1024;
  const char *arg_name = argc > 1 ? argv[1] : NULL;

  test_build_nodes();
  test_graph_image_dump(graph_size);
  test_destructive_std_depth_first_traversal();
  test_destructive_pointer_reversal_traversal(graph_size);
  test_destructive_pointer_back_and_forth_traversal(graph_size);
  test_pointer_reversal_traversal(graph_size);
  test_graph_persist_to_buf(graph_size);
  // stress_runtime_complexity_all_algo(arg_name);
  // stress_profile_algo_on_all_graph_types(graph_size, arg_name);

  LOG_WARN("All done !!");
  return 0;
}
#pragma GCC diagnostic pop

