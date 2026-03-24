#ifndef CO_BASIC_BLOCK_H
#define CO_BASIC_BLOCK_H

#include "utils/hash_map.h"

typedef struct BasicBlock {
  HashMap *operands;
  unsigned long stack_space;
} BasicBlock;

BasicBlock *new_basic_block(void);

void *basic_block_get(BasicBlock *bb, char *name);

void basic_block_free(BasicBlock *bb);

#endif // !CO_BASIC_BLOCK_H
