#include <common.h>
#include <util.h>
#include <graph.h>
#include <special_traversals.h>
#include <tests.h>

void test_build_nodes () {
  for(int i=0; i<5; ++i) {
    Node n2 = build_node(i);
    LOG_INFO("Created %s", print_node(&n2));
  }
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

void helper_traversal_on_real_graphs (uint32_t graph_size, TraversalAlgo_t traversal_algo) {
  GraphHandle graphs[5];
  Node* start_node = NULL;
  uint32_t node_visit_count = 0;

  graphs[0] = construct_and_dump_to_file(build_graph_dag, graph_size, "test_build_graph_dag");
  traversal_algo(graphs[0].root, &node_visit_count, count_visitor);
  ASSERT(node_visit_count == graph_size, "visited the wrong number of nodes");

  node_visit_count = 0;
  graphs[1] = construct_and_dump_to_file(build_graph_dag_maybe_disconnected, graph_size, "test_build_graph_dag_maybe_disconnected");
  start_node = graphs[1].root + rand() % graph_size;
  traversal_algo(start_node, &node_visit_count, count_visitor);
  ASSERT(node_visit_count <= graph_size, "visited the wrong number of nodes");

  node_visit_count = 0;
  graphs[2] = construct_and_dump_to_file(build_graph_with_cycles, graph_size, "test_build_graph_with_cycles");
  traversal_algo(graphs[2].root, &node_visit_count, count_visitor);
  ASSERT(node_visit_count == graph_size, "visited the wrong number of nodes");

  node_visit_count = 0;
  graphs[3] = construct_and_dump_to_file(build_graph_with_undirected_cycles, graph_size, "test_build_graph_with_undirected_cycles");
  traversal_algo(graphs[3].root, &node_visit_count, count_visitor);
  ASSERT(node_visit_count == graph_size, "visited the wrong number of nodes");

  node_visit_count = 0;
  graphs[4] = construct_and_dump_to_file(build_graph_amorphous, graph_size, "test_build_graph_amorphous");
  start_node = graphs[4].root + rand() % graph_size;
  traversal_algo(start_node, &node_visit_count, count_visitor);
  ASSERT(node_visit_count <= graph_size, "visited the wrong number of nodes");

  for(int i=0; i<sizeof(graphs)/sizeof(GraphHandle); ++i)
    free_graph(graphs[i]);
}

void helper_traversal_on_three_four_nodes(TraversalAlgo_t traversal_algo) {
  uint32_t node_visit_count = 0;
  GraphHandle graph = build_graph_triangle();
  LOG_INFO("Building triangle graph");
  traversal_algo(graph.root, &node_visit_count, count_visitor);
  ASSERT(node_visit_count <= graph.vertex_count, "visited the wrong number of nodes");
  free_graph(graph);

  node_visit_count = 0;
  graph = build_graph_diamond();
  LOG_INFO("Building diamond graph");
  traversal_algo(graph.root, &node_visit_count, count_visitor);
  ASSERT(node_visit_count <= graph.vertex_count, "visited the wrong number of nodes");
  free_graph(graph);

  node_visit_count = 0;
  graph = build_graph_diamond();
  graph.root[1].slots[1] = graph.root + 0;
  graph.root[2].slots[1] = graph.root + 0;
  graph.root[3].slots[0] = graph.root + 1;
  graph.root[3].slots[1] = graph.root + 2;
  LOG_INFO("Building double linked diamond graph");
  traversal_algo(graph.root, &node_visit_count, count_visitor);
  ASSERT(node_visit_count <= graph.vertex_count, "visited the wrong number of nodes");
  free_graph(graph);

  node_visit_count = 0;
  graph = build_graph_diamond();
  graph.root[0].slots[2] = graph.root + 3;
  graph.root[1].slots[1] = graph.root + 0;
  graph.root[1].slots[2] = graph.root + 2;
  graph.root[2].slots[1] = graph.root + 0;
  graph.root[2].slots[2] = graph.root + 1;
  graph.root[3].slots[0] = graph.root + 0;
  graph.root[3].slots[1] = graph.root + 1;
  graph.root[3].slots[2] = graph.root + 2;
  LOG_INFO("Building triple linked diamond graph");
  traversal_algo(graph.root, &node_visit_count, count_visitor);
  ASSERT(node_visit_count <= graph.vertex_count, "visited the wrong number of nodes");
  free_graph(graph);
}

void helper_traversal_on_branches(TraversalAlgo_t traversal_algo) {
  uint32_t node_visit_count = 0;
  GraphHandle graph = build_graph_unbalanced_branches();
  LOG_INFO("Building unbalanced branches graph");
  traversal_algo(graph.root, &node_visit_count, count_visitor);
  ASSERT(node_visit_count <= graph.vertex_count, "visited the wrong number of nodes");
  free_graph(graph);

  node_visit_count = 0;
  graph = build_graph_unbalanced_branches();
  graph.root[6].slots[0] = graph.root + 1;
  LOG_INFO("Building unbalanced branches with cycle undirected graph");
  traversal_algo(graph.root, &node_visit_count, count_visitor);
  ASSERT(node_visit_count <= graph.vertex_count, "visited the wrong number of nodes");
  free_graph(graph);

  node_visit_count = 0;
  graph = build_graph_unbalanced_branches();
  graph.root[6].slots[0] = graph.root + 1;
  graph.root[4].slots[0] = graph.root + 6;
  LOG_INFO("Building unbalanced branches with cycle directed graph");
  traversal_algo(graph.root, &node_visit_count, count_visitor);
  ASSERT(node_visit_count <= graph.vertex_count, "visited the wrong number of nodes");
  free_graph(graph);
}

void helper_traversal_on_single_branch(TraversalAlgo_t traversal_algo) {
  uint32_t node_visit_count = 0;
  GraphHandle graph = build_graph_single_branch(6);
  LOG_INFO("Building single branch graph");
  traversal_algo(graph.root, &node_visit_count, count_visitor);
  ASSERT(node_visit_count <= graph.vertex_count, "visited the wrong number of nodes");
  free_graph(graph);

  node_visit_count = 0;
  graph = build_graph_branch_with_fanout();
  LOG_INFO("Building single branch with fanout graph");
  traversal_algo(graph.root, &node_visit_count, count_visitor);
  ASSERT(node_visit_count <= graph.vertex_count, "visited the wrong number of nodes");
  free_graph(graph);

  node_visit_count = 0;
  graph = build_graph_single_branch(6);
  graph.root[5].slots[0] = graph.root + 1;
  graph.root[4].slots[1] = graph.root + 1;
  graph.root[3].slots[1] = graph.root;
  LOG_INFO("Building single branch with backward cycle");
  traversal_algo(graph.root, &node_visit_count, count_visitor);
  ASSERT(node_visit_count <= graph.vertex_count, "visited the wrong number of nodes");
  free_graph(graph);
}

void helper_traversal_on_loop_to_itself(TraversalAlgo_t traversal_algo) {
  uint32_t node_visit_count = 0;
  GraphHandle graph = build_graph_single_branch(1);
  graph.root[0].slots[0] = graph.root;
  graph.root[0].slots[1] = graph.root;
  LOG_INFO("Building single node with loops to itself");
  traversal_algo(graph.root, &node_visit_count, count_visitor);
  ASSERT(node_visit_count <= graph.vertex_count, "visited the wrong number of nodes");
  free_graph(graph);

  node_visit_count = 0;
  graph = build_graph_single_branch(5);
  graph.root[2].slots[1] = graph.root + 2;
  graph.root[4].slots[0] = graph.root + 4;
  graph.root[4].slots[1] = graph.root + 4;
  LOG_INFO("Building single branch with looping nodes");
  traversal_algo(graph.root, &node_visit_count, count_visitor);
  ASSERT(node_visit_count <= graph.vertex_count, "visited the wrong number of nodes");
  free_graph(graph);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_destructive_pointer_reversal_traversal(uint32_t size) {
  helper_traversal_on_three_four_nodes(destructive_pointer_reversal_traversal);
  helper_traversal_on_branches(destructive_pointer_reversal_traversal);
  helper_traversal_on_single_branch(destructive_pointer_reversal_traversal);
  helper_traversal_on_loop_to_itself(destructive_pointer_reversal_traversal);
  helper_traversal_on_real_graphs(size, destructive_pointer_reversal_traversal);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void test_destructive_pointer_back_and_forth_traversal(uint32_t size) {
  helper_traversal_on_three_four_nodes(destructive_pointer_back_and_forth_traversal);
  helper_traversal_on_branches(destructive_pointer_back_and_forth_traversal);
  helper_traversal_on_single_branch(destructive_pointer_back_and_forth_traversal);
  helper_traversal_on_loop_to_itself(destructive_pointer_back_and_forth_traversal);
  helper_traversal_on_real_graphs(size, destructive_pointer_back_and_forth_traversal);
}

