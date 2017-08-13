#pragma once

#include "node.h"
#include "graph.h"

void pointer_reversal_traversal(Node* node, VisitorState* visit_state, Visitor_t visitor);
// Depth first, after traversal the graph becomes unusable
void destructive_pointer_reversal_traversal(Node* node, VisitorState* visit_state, Visitor_t visitor);
// Inefficient breadth first
void destructive_pointer_back_and_forth_traversal(Node* node, VisitorState* visit_state, Visitor_t visitor);

///////////////////////////////////////////////////////////////////////////////////////////

struct TraverseState {
  Node *tail, *parent;
  uint32_t gen, queue_len, next_tail_gen;
};
typedef struct TraverseState TraverseState;

struct EdgeParams {
  // If min_gen == 0, then the node contains no forward edges
  uint32_t inv_idx, min_idx, min_gen;
};
typedef struct EdgeParams EdgeParams;

// if we do not prune the branch that bears no more children, we will not be able to navigate using the generation
// At the end of the pruning we need to go forwards
uint32_t backward_prune_exhausted_branch__mut(TraverseState *state);

void adjust_state_based_on_nodes_visited__mut(TraverseState *state, uint32_t visit_count);

TraverseState prepare_initial_state__mut(Node *root, VisitorState* visit_state, Visitor_t visitor);

uint32_t visit_childs_tag_generation__mut(Node* node, VisitorState* visit_state, Visitor_t visitor, uint32_t start_gen);

// While going backwards we also adjust the generation of the nodes we traverse to preserve the invariant :
// a node has the same generation as the smallest of its visited ancestors
EdgeParams backward_invert_edges__mut(TraverseState *state);

// The edge pointing to the lowest generation is swapped with the edge pointing to the highest
void pivot_back_forward_edges__mut(TraverseState *state, EdgeParams params);

void forward_invert_edges__mut(TraverseState *state);

EdgeParams get_node_edge_parameters(Node *node);

// Returns true if the node has a loop to itself
// Or we have something like : -- (gen:7) <-- (gen:7) <-- (gen:7)
//                                   ^______________________/
uint32_t pathological_branch_loop_back(Node *node, uint32_t child_edge);

void debug_pointer_back_and_forth_traversal_state(TraverseState state, VisitorState* visit_state);

