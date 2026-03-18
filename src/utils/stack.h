#ifndef CO_STACK_H
#define CO_STACK_H

#include "utils/list.h"

typedef struct Stack {
  List *elements;
} Stack;

Stack *new_stack(void);

void *stack_pop(Stack *stack);

void stack_push(Stack *stack, void *data);

int is_stack_empty(Stack *stack);

#endif // !CO_STACK_H
