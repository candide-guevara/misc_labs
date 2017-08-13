#include <special_traversals.h>

#include <common.h>
#include <util.h>

void pointer_reversal_traversal(Node* node, VisitorState* visit_state, Visitor_t visitor) {
}

// We expected all node in the graph have count == 0
void destructive_pointer_reversal_traversal(Node* node, VisitorState* visit_state, Visitor_t visitor) {
  // we use poison to avoid an end of traversal condition branch
  Node poison = build_node(0);
  poison.slots[0] = &poison;
  poison.count = 1;
  // a pointer to the start of the linked list containing the visited nodes
  Node* previous = &poison;

  // Invariants :
  // 1. node points to the current node
  // 2. node->count is the index of the first un-traversed edge of the current node
  // 3. previous points to the node visited just before
  // 4. previous>slots[previous->count - 1] points to the node visited 2 steps before
  while (node != &poison) {
    uint32_t next_slot = node->count;
    // If count > 0 we have already visited this node, we also use it to index slots[]
    if (node->count == 0)
      visitor(visit_state, node);

    for(; next_slot < SLOT_COUNT && !node->slots[next_slot]; ++next_slot);
    if (next_slot == SLOT_COUNT) {
      node->count = next_slot;
      for(next_slot=previous->count-1; next_slot >= 0 && !previous->slots[next_slot]; --next_slot);
      
      Node* tmp_node = previous;
      node = previous;
      previous = tmp_node->slots[next_slot];
      // we can only go back through this edge once
      tmp_node->slots[next_slot] = NULL;
    }
    else {  
      Node* tmp_node = node;
      node = tmp_node->slots[next_slot];
      tmp_node->slots[next_slot] = previous;
      previous = tmp_node;
      tmp_node->count = next_slot + 1;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void destructive_pointer_back_and_forth_traversal(Node* node, VisitorState* visit_state, Visitor_t visitor) {
  TraverseState state = prepare_initial_state__mut(node, visit_state, visitor);
  if (!state.queue_len) return;

  while (1) {
    debug_pointer_back_and_forth_traversal_state(state, visit_state);
    uint32_t visit_count = visit_childs_tag_generation__mut(state.tail, visit_state, visitor, state.gen);
    adjust_state_based_on_nodes_visited__mut(&state, visit_count);
    if (!state.queue_len) return;

    // we go backward then we pivot and move forward looking for the new tail node
    EdgeParams params = {0};
    uint32_t tail_gen = 0;

    if (!visit_count)
      tail_gen = backward_prune_exhausted_branch__mut(&state);
    if (tail_gen != state.next_tail_gen)
      params = backward_invert_edges__mut(&state);

    pivot_back_forward_edges__mut(&state, params);
    forward_invert_edges__mut(&state);
  }
  debug_pointer_back_and_forth_traversal_state(state, visit_state);
}

uint32_t backward_prune_exhausted_branch__mut(TraverseState *state) {
  EdgeParams params = {0};
  // Invariant : the tail node has no forward edges
  while (!params.min_gen) {
    ASSERT(state->parent, "We should not go past the root while pruning backwards");
    params = get_node_edge_parameters(state->parent);
    ASSERT(is_backwards(state->parent->slots[params.inv_idx]), "There should always be a backward edge");

    state->tail = state->parent;
    state->parent = follow_edge(state->parent->slots[params.inv_idx]);
  }
  // we reached a node with ancestors in the visit queue, recalculate generation after prune
  state->tail->count = params.min_gen;
  // we keep the invariant that between parent and tail there is no edge
  state->tail->slots[params.inv_idx] = NULL;
  ASSERT(state->tail->count >= state->next_tail_gen, "When we cannot prune further we should be at a higher generation");
  return state->tail->count;
}

void adjust_state_based_on_nodes_visited__mut(TraverseState *state, uint32_t visit_count) {
  state->gen += visit_count;
  state->queue_len += visit_count - 1;
  state->next_tail_gen += 1;
  ASSERT(state->queue_len < state->gen, "generation > queue_len");
  ASSERT(state->queue_len >= 0 , "We should have stopped before queue_len < 0");
}

TraverseState prepare_initial_state__mut(Node *root, VisitorState* visit_state, Visitor_t visitor) {
  TraverseState state = {0};
  if (!root) return state;
  debug_pointer_back_and_forth_traversal_state(state, visit_state);

  visitor(visit_state, root);
  state.parent = root;
  // we set the root generation now in case it has edges pointing to itself
  state.parent->count = 1;
  uint32_t visit_count = visit_childs_tag_generation__mut(root, visit_state, visitor, 1);

  if (visit_count) {
    uint32_t tail_idx=0;
    for(; tail_idx<SLOT_COUNT && root->slots[tail_idx] == NULL; ++tail_idx);

    state.tail = root->slots[tail_idx];
    state.queue_len = visit_count;
    state.gen = visit_count + 1;
    state.next_tail_gen = 2;

    root->slots[tail_idx] = to_backwards_edge(NULL);
  }
  return state;
}

uint32_t visit_childs_tag_generation__mut(Node* node, VisitorState* visit_state, Visitor_t visitor, uint32_t start_gen) {
  uint32_t visit_count = 0;

  for(uint32_t i=0; i<SLOT_COUNT; ++i) {
    ASSERT(!is_backwards(node->slots[i]), "The tail node can only have edges going forward");
    if (node->slots[i]) {
      Node *child = node->slots[i];
      // cycle detected, remove edge to avoid recursion
      if (child->count) {
        ASSERT(child->count <= start_gen, "Weird cycle to the future");
        node->slots[i] = NULL;
      }
      else {
        visit_count += 1;
        child->count = start_gen + visit_count;
        visitor(visit_state, child);
      }
    }
  }
  // A node has the same generation as the smallest of its visited ancestors
  if(visit_count)
    node->count = start_gen + 1;
  return visit_count;
}

EdgeParams backward_invert_edges__mut(TraverseState *state) {
  EdgeParams params = {0};
  Node *grand_pa = NULL;

  // Invariant : (grand_pa / NULL) <-- (parent)   (tail)
  while(1) {
    params = get_node_edge_parameters(state->parent);
    ASSERT(state->parent->count <= state->tail->count, "While going backwards children always hold a higher gen than parents");
    ASSERT(is_backwards(state->parent->slots[params.inv_idx]), "There should always be a backward edge");
    grand_pa = follow_edge(state->parent->slots[params.inv_idx]);

    state->parent->count = params.min_gen ? params.min_gen : state->tail->count;
    ASSERT(state->parent->count >= state->next_tail_gen, "While going backwards we never go under the target generation");

    // we have found the closest ancestor of the node we are looking for
    if (state->parent->count == state->next_tail_gen) break;

    state->parent->slots[params.inv_idx] = state->tail;
    state->tail = state->parent;
    state->parent = grand_pa;
    ASSERT(state->parent, "We should not go past the root while going backwards");
  }
  // The edge params of the node where the traversal must change direction
  // Or the default value on a degenerate case : no need to backward to reach the next tail node
  return params;
}

void pivot_back_forward_edges__mut(TraverseState *state, EdgeParams params) {
  // degenerate case : no need to backward to reach the next tail node
  if (!params.min_gen) return;

  Node *grand_pa = follow_edge(state->parent->slots[params.inv_idx]);
  state->parent->slots[params.inv_idx] = state->tail;
  state->tail = state->parent->slots[params.min_idx];
  state->parent->slots[params.min_idx] = to_backwards_edge(grand_pa);
  ASSERT(state->tail->count == state->next_tail_gen, "The pivoted edge should have pointed to the next tail node");
}

void forward_invert_edges__mut(TraverseState *state) {
  while(1) {
    ASSERT(state->tail->count == state->next_tail_gen, "Going forward we only traverse ancestors of the target tail");
    EdgeParams params = get_node_edge_parameters(state->tail);

    // we have found a node whose children have not been visited that has the generation we want 
    if (!params.min_gen 
        // or whose children lowest gen does not match (this indicates a cycle to an already visited node)
        || params.min_gen != state->next_tail_gen
        || pathological_branch_loop_back(state->tail, params.min_idx) ) 
      // => this is the tail
      break;

    Node *grand_pa = state->parent;
    state->parent = state->tail;
    state->tail = state->tail->slots[params.min_idx];
    state->parent->slots[params.min_idx] = to_backwards_edge(grand_pa);
  }
}

EdgeParams get_node_edge_parameters(Node *node) {
  EdgeParams params = {0};

  for(uint32_t i=0; i<SLOT_COUNT; ++i) {
    if (is_backwards(node->slots[i])) {
      ASSERT(params.inv_idx == i || !is_backwards(node->slots[params.inv_idx]), "There can only be one backward edge");
      params.inv_idx = i;
    }
    else if (node->slots[i] && (!params.min_gen || params.min_gen > node->slots[i]->count)) {
      params.min_idx = i;
      params.min_gen = node->slots[i]->count;
    }
  }
  return params;
}

uint32_t pathological_branch_loop_back(Node *node, uint32_t child_edge) {
  Node *child = node->slots[child_edge];
  uint32_t is_a_loop = child == node;
  // The inverted edge we write while traversing forward serves as a "breadcrumb" 
  // to identify the previous nodes of this branch
  for(uint32_t i=0; i<SLOT_COUNT && !is_a_loop; ++i) 
    if (is_backwards(child->slots[i])) 
      is_a_loop = 1;
  return is_a_loop;
}

void debug_pointer_back_and_forth_traversal_state(TraverseState state, VisitorState* visit_state) {
  //#define TRAVERSE_TRACE
  #ifdef TRAVERSE_TRACE
  char filepath[128];
  Node *root = visit_state->graph.root;
  snprintf(filepath, sizeof(filepath), "graph_%s_%u_%u_%u.dot", 
            root->name, state.gen, state.next_tail_gen, state.queue_len);
  dump_graph_dot_format(visit_state->graph, filepath);

  LOG_INFO("state : %p, %p, %u, %u, %u", state.tail, state.parent, state.gen, state.next_tail_gen, state.queue_len);
  LOG_INFO("tail : %s", print_node(state.tail));
  LOG_INFO("parent : %s\n", print_node(state.parent));
  #endif
}

