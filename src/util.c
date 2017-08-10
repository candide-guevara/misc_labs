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

