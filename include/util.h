#pragma once

#include <stdio.h>
#include <stdint.h>

#include "graph.h"

typedef struct Stack Stack;

struct Stack {
  void ** items;
  size_t capacity, next;
};

Stack build_stack(size_t capacity);
void free_stack(Stack* stack);
void push(Stack* stack, void* item);
void* pop(Stack* stack);
void* peek(Stack* stack, size_t index);

////////////////////////////////////////////////////////////////////////////

union PointerFlag {
  void* pointer;
  size_t flag;
};
typedef union PointerFlag PointerFlag;

const static size_t SET_BACK_FLAG = 1;
const static size_t DEL_BACK_FLAG = ~0 ^ 1;

#pragma GCC diagnostic ignored "-Wunused-function"
static void* to_backwards_edge(void* new_value) {
  PointerFlag pt_flag;
  pt_flag.pointer = new_value;
  pt_flag.flag |= SET_BACK_FLAG;
  return pt_flag.pointer;
}

static void* follow_edge(void* value) {
  PointerFlag pt_flag;
  pt_flag.pointer = value;
  pt_flag.flag &= DEL_BACK_FLAG;
  return pt_flag.pointer;
}

static uint32_t is_backwards(void* value) {
  PointerFlag pt_flag;
  pt_flag.pointer = value;
  pt_flag.flag &= SET_BACK_FLAG;
  return pt_flag.flag;
}
#pragma GCC diagnostic pop

uint32_t is_leaf_node(Node *node);
uint32_t is_leaf_node_ignore_back(Node *node);
void fprintf_node_dot_format(FILE* dot_file, Node *node, const char* color);
void fprintf_edge_dot_format(FILE* dot_file, Node *node, Node *child, const char* color);

////////////////////////////////////////////////////////////////////////////

GraphHandle build_graph_diamond();
GraphHandle build_graph_triangle();
GraphHandle build_graph_unbalanced_branches();
GraphHandle build_graph_branch_with_fanout();
GraphHandle construct_and_dump_to_file(GraphHandle (*)(uint32_t), uint32_t, const char*);

////////////////////////////////////////////////////////////////////////////

typedef struct CountState CountState;

struct CountState {
  uint32_t counter;
  // Points to the first node visited out of order
  Node *first_out_of_order;
  Node *last_visit_node;
};

VisitorState* new_count_state();
void reset_count_state(VisitorState* visit_state, GraphHandle new_graph);
void free_count_state(VisitorState *visit_state);

void count_visitor (VisitorState* visit_state, Node* node);
void monotonic_count_visitor (VisitorState* visit_state, Node* node);

