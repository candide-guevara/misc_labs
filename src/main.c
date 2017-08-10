#include <common.h>
#include <graph.h>

void test_build_nodes();
void test_destructive_traversal(uint32_t);
void test_graph_image_dump(uint32_t);
GraphHandle construct_and_dump_to_file(GraphHandle (*)(uint32_t), uint32_t, const char*);

int main (void) {
  LOG_WARN("Starting ...");
  uint32_t graph_size = 64;

  // test_build_nodes();
  // test_graph_image_dump(graph_size);
  test_destructive_traversal(graph_size);

  LOG_INFO("All done !!");
  return 0;
}

void test_build_nodes () {
  for(int i=0; i<5; ++i) {
    Node n2 = build_node(i);
    LOG_INFO("Created %s", print_node(&n2));
  }
}

void count_visits (void* state, Node* node) {
  uint32_t* node_visit_count = (uint32_t*)state;
  (*node_visit_count) += 1;
  LOG_INFO(" - %d, %p, %s", *node_visit_count, node, print_node(node));
}

void test_destructive_traversal (uint32_t graph_size) {
  GraphHandle graphs[5];
  Node* start_node = NULL;
  uint32_t node_visit_count = 0;

  graphs[0] = construct_and_dump_to_file(build_graph_dag, graph_size, "test_build_graph_dag");
  destructive_pointer_reversal_traversal(graphs[0].root, &node_visit_count, count_visits);
  ASSERT(node_visit_count == graph_size, "visited the wrong number of nodes");

  node_visit_count = 0;
  graphs[1] = construct_and_dump_to_file(build_graph_dag_maybe_disconnected, graph_size, "test_build_graph_dag_maybe_disconnected");
  start_node = graphs[1].root + rand() % graph_size;
  destructive_pointer_reversal_traversal(start_node, &node_visit_count, count_visits);
  ASSERT(node_visit_count <= graph_size, "visited the wrong number of nodes");

  node_visit_count = 0;
  graphs[2] = construct_and_dump_to_file(build_graph_with_cycles, graph_size, "test_build_graph_with_cycles");
  destructive_pointer_reversal_traversal(graphs[2].root, &node_visit_count, count_visits);
  ASSERT(node_visit_count == graph_size, "visited the wrong number of nodes");

  node_visit_count = 0;
  graphs[3] = construct_and_dump_to_file(build_graph_with_undirected_cycles, graph_size, "test_build_graph_with_undirected_cycles");
  destructive_pointer_reversal_traversal(graphs[3].root, &node_visit_count, count_visits);
  ASSERT(node_visit_count == graph_size, "visited the wrong number of nodes");

  node_visit_count = 0;
  graphs[4] = construct_and_dump_to_file(build_graph_amorphous, graph_size, "test_build_graph_amorphous");
  start_node = graphs[4].root + rand() % graph_size;
  destructive_pointer_reversal_traversal(start_node, &node_visit_count, count_visits);
  ASSERT(node_visit_count <= graph_size, "visited the wrong number of nodes");

  for(int i=0; i<sizeof(graphs)/sizeof(GraphHandle); ++i)
    free_graph(graphs[i]);
}

void test_graph_image_dump(uint32_t graph_size) {
  GraphHandle graphs[5];
  graphs[0] = construct_and_dump_to_file(build_graph_dag, graph_size, "test_build_graph_dag");
  graphs[1] = construct_and_dump_to_file(build_graph_dag_maybe_disconnected, graph_size, "test_build_graph_dag_maybe_disconnected");
  graphs[2] = construct_and_dump_to_file(build_graph_with_cycles, graph_size, "test_build_graph_with_cycles");
  graphs[3] = construct_and_dump_to_file(build_graph_with_undirected_cycles, graph_size, "test_build_graph_with_undirected_cycles");
  graphs[4] = construct_and_dump_to_file(build_graph_amorphous, graph_size, "test_build_graph_amorphous");

  for(int i=0; i<sizeof(graphs)/sizeof(GraphHandle); ++i)
    free_graph(graphs[i]);
}

GraphHandle construct_and_dump_to_file(GraphHandle (*constructor)(uint32_t), uint32_t graph_size, const char* filename) {
  char filepath[64];
  snprintf(filepath, sizeof(filepath), "%s.dot", filename);

  LOG_INFO("Building graph %s", filename);
  GraphHandle graph = constructor(graph_size);
  dump_graph_dot_format(graph, filepath);
  return graph;
}


