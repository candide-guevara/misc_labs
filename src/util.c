#include <util.h>

#include <common.h>

Stack build_stack(size_t capacity) {
  Stack stack;
  stack.next = 0;
  stack.capacity = capacity;
  stack.items = calloc(capacity, sizeof(void*));
  ASSERT(stack.items, "Failed to allocate mem for stack");
  return stack;
}

void free_stack(Stack* stack) {
  if (stack->items && stack->capacity)
    free(stack->items);
}

void push(Stack* stack, void* item) {
  ASSERT(stack->next < stack->capacity, "Stack overflow");
  stack->items[stack->next] = item;
  stack->next += 1;
}

void* pop(Stack* stack) {
  ASSERT(stack->next > 0, "Stack underflow");
  stack->next -= 1;
  return stack->items[stack->next];
}

void* peek(Stack* stack, size_t index) {
  ASSERT(index < stack->next, "Invalid stack location");
  return stack->items[index];
}

////////////////////////////////////////////////////////////////////////////

void* to_backwards_edge(void* new_value) {
  PointerFlag pt_flag;
  pt_flag.pointer = new_value;
  pt_flag.flag |= SET_BACK_FLAG;
  return pt_flag.pointer;
}

void* follow_edge(void* value) {
  PointerFlag pt_flag;
  pt_flag.pointer = value;
  pt_flag.flag &= DEL_BACK_FLAG;
  return pt_flag.pointer;
}

uint32_t is_backwards(void* value) {
  PointerFlag pt_flag;
  pt_flag.pointer = value;
  pt_flag.flag &= SET_BACK_FLAG;
  return pt_flag.flag;
}

////////////////////////////////////////////////////////////////////////////

GraphHandle construct_and_dump_to_file(GraphHandle (*constructor)(uint32_t), uint32_t graph_size, const char* filename) {
  char filepath[64];
  snprintf(filepath, sizeof(filepath), "%s.dot", filename);

  LOG_INFO("Building graph %s", filename);
  GraphHandle graph = constructor(graph_size);
  dump_graph_dot_format(graph, filepath);
  return graph;
}

void count_visitor (void* state, Node* node) {
  uint32_t* node_visit_count = (uint32_t*)state;
  (*node_visit_count) += 1;
  LOG_TRACE(" - %d, %p, %s", *node_visit_count, node, print_node(node));
}

GraphHandle build_graph_triangle() {
  GraphHandle graph = build_graph_without_any_edges(3);
  ASSERT(SLOT_COUNT > 1, "Cannot build triangle graph in this configuration");
  graph.root[0].slots[0] = graph.root + 1;
  graph.root[0].slots[1] = graph.root + 2;
  return graph;
}

GraphHandle build_graph_diamond() {
  GraphHandle graph = build_graph_without_any_edges(4);
  ASSERT(SLOT_COUNT > 1, "Cannot build diamond graph in this configuration");
  graph.root[0].slots[0] = graph.root + 1;
  graph.root[0].slots[1] = graph.root + 2;
  graph.root[1].slots[0] = graph.root + 3;
  graph.root[2].slots[0] = graph.root + 3;
  return graph;
}

GraphHandle build_graph_unbalanced_branches() {
  GraphHandle graph = build_graph_without_any_edges(7);
  ASSERT(SLOT_COUNT > 1, "Cannot build multi branch graph in this configuration");
  graph.root[0].slots[0] = graph.root + 1;
  graph.root[1].slots[0] = graph.root + 2;
  graph.root[2].slots[0] = graph.root + 3;
  graph.root[3].slots[0] = graph.root + 4;

  graph.root[0].slots[1] = graph.root + 5;
  graph.root[5].slots[0] = graph.root + 6;
  return graph;
}

GraphHandle build_graph_branch_with_fanout() {
  GraphHandle graph = build_graph_without_any_edges(3 + SLOT_COUNT);
  ASSERT(SLOT_COUNT > 1, "Cannot build triangle graph in this configuration");
  graph.root[0].slots[0] = graph.root + 1;
  graph.root[1].slots[0] = graph.root + 2;
  for(uint32_t i=0; i<SLOT_COUNT; ++i)
    graph.root[2].slots[i] = graph.root + 3 + i;
  return graph;
}

