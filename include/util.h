#pragma once

#include <stdio.h>

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

void* set_and_toggle(void* new_value);
void* get_and_toggle(void* value);
void* follow_edge(void* value);
uint32_t is_backwards(void* value);

