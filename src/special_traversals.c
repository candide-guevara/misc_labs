#include <graph.h>

#include <common.h>

void pointer_reversal_traversal(Node* node, void* state, void (*visitor)(void*, Node*)) {
}

// We expected all node in the graph have count == 0
void destructive_pointer_reversal_traversal(Node* node, void* state, void (*visitor)(void*, Node*)) {
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
      visitor(state, node);

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

struct TraverseState {
  Node *tail, *parent;
  uint32_t gen, queue_len;
};
typedef struct TraverseState TraverseState;

struct EdgeParams {
  uint32_t inv_idx, min_idx, min_gen;
};
typedef struct EdgeParams EdgeParams;

struct NodeAndIdx {
  uint32_t idx;
  Node* node;
};
typedef struct NodeAndIdx NodeAndIdx;

void pointer_back_and_forth_traversal(Node* node, void* visitor_state, void (*visitor)(void*, Node*)) {
  TraverseState state = prepare_initial_state__mut(node, visitor_state);

  while (state.queue_len) {
    uint32_t increment = visit_childs_tag_generation__mut(state.tail, visitor_state, state.gen);
    state.gen += increment;
    state.queue_len += increment - 1;
    uint32_t tail_gen = state.gen + 1 - state.queue_len;

    tail = traverse_back_tag_generation__mut(state.tail, state.parent, tail_gen);
  }
}

TraverseState prepare_initial_state__mut(Node *root, void* visitor_state) {
  TraverseState state = {0};
  if (!root) return state;

  visitor(visitor_state, root);
  uint32_t increment = visit_childs_tag_generation__mut(root, visitor_state, 0);

  if (increment) {
    uint32_t tail_idx=0;
    for(; tail_idx<SLOT_COUNT && root->slots[tail_idx] == NULL; ++tail_idx);

    state.tail = root->slots[tail_idx];
    state.parent = root;
    state.parent->count = increment;
    state.queue_len = increment;
    state.gen = increment;

    root->slots[tail_idx] = set_and_toggle(NULL);
  }
  return state;
}

uint32_t visit_childs_tag_generation__mut(Node* node, void* state, uint32_t start_gen) {
  uint32_t increment = 0;

  for(uint32_t i=0; i<SLOT_COUNT; ++i)
    if (node->slots[i]) {
      Node *child = node->slots[i];
      // cycle detected, remove edge to avoid recursion
      if (child->count) {
        ASSERT(child->count <= start_gen, "Weird cycle to the future");
        node->slots[i] = NULL;
      }
      else {
        increment += 1;
        child->count = start_gen + increment;
        visitor(state, child);
      }
    }

  // A node count is the minimum generation reachable from it
  if(increment)
    node->count = start_gen + 1;
  return increment;
}

Node* traverse_back_tag_generation__mut(Node *tail, Node *parent, uint32_t tail_gen) {
  EdgeParams params = {0};
  Node *grand_pa = NULL;

  // go backwards, update generation and invert edges
  // Invariant : (grand_pa / NULL) <-- (parent)   (tail)
  while(1) {
    params = get_node_edge_parameters(parent);
    grand_pa = follow_edge(parent->slots[params.inv_idx]);

    if (params.min_gen)
      parent->count = params.min_gen;
    else
      parent->count = tail->count;

    // no need to keep going backwards, parent->count < tail_gen when a branch did not yield any new nodes to append to the queue
    if (parent->count <= tail_gen) break;

    parent->slots[params.inv_idx] = tail;
    tail = parent;
    parent = grand_pa;
    ASSERT(parent, "Reached root without finding tail");
  }

  // pivot edges : the edge pointing to the lowest generation is swapped with the edge pointinng to the highest
  parent->slots[params.inv_idx] = tail;
  tail = parent->slots[params.min_idx];
  parent->slots[params.min_idx] = set_and_toggle(grand_pa);

  // go forwards and invert edges
  while(1) {
    NodeAndIdx child = get_child_matching_gen(parent, tail_gen);
    grand_pa = parent;
    parent = tail;

    if (!child.node) {
      ASSERT(tail->count == tail_gen, "Here parent should be the node further down in the tree matching tail_gen");
      break;
    }

    tail = child.node;
    parent->slots[child.idx] = set_and_toggle(grand_pa);
  }
  return tail;
}

EdgeParams get_node_edge_parameters(Node *node) {
  EdgeParams params = {0};
  for(uint32_t i=0; i<SLOT_COUNT; ++i) {
    if (is_backwards(node->slots[i]))
      params.inv_idx = i;
    else if (node->slots[i] && (!params.min_gen || params.min_gen > node->slots[i]->count)) {
      params.min_idx = i;
      params.min_gen = node->slots[i]->count;
    }
  }
  return params;
}

NodeAndIdx get_child_matching_gen(Node *node, uint32_t generation) {
  NodeAndIdx nodeAndIdx = {0};
  for(uint32_t i=0; i<SLOT_COUNT; ++i) 
    if (node->slots[i] && node->slots[i]->count == generation) {
      nodeAndIdx.node = node->slots[i];
      nodeAndIdx.idx = i;
      break;
    }
  return nodeAndIdx;
}

