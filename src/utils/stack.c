#include "stack.h"

#include <stdio.h>
#include <stdlib.h>

Stack *new_stack(void) {
  Stack *stack = malloc(sizeof(Stack));

  stack->elements = new_list();

  return stack;
}

void *stack_pop(Stack *stack) {
  if (is_stack_empty(stack))
    return NULL;

  void *ans = stack->elements->end->data;

  Node *new_end = stack->elements->end->prev;

  free(stack->elements->end);

  if (new_end != NULL) {
    new_end->next = NULL;
  } else {
    stack->elements->root = NULL;
  }

  stack->elements->end = new_end;

  return ans;
}

void stack_push(Stack *stack, void *data) {
  list_append(stack->elements, data);
}

int is_stack_empty(Stack *stack) { return stack->elements->root == NULL; }
