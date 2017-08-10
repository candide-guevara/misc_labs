#pragma once

#include <stdio.h>
#include <stdint.h>

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

void* to_backwards_edge(void* new_value);
void* follow_edge(void* value);
uint32_t is_backwards(void* value);

