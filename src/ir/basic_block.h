#ifndef CO_BASIC_BLOCK_H
#define CO_BASIC_BLOCK_H

#include "utils/hash_map.h"

typedef struct BasicBlock {
  unsigned long stack_space;
  List *expressions;
  char *label;
  struct BasicBlock *exit_true, *exit_false;
} BasicBlock;

BasicBlock *new_basic_block(char *label);

void basic_block_free(BasicBlock *bb);

#endif // !CO_BASIC_BLOCK_H
