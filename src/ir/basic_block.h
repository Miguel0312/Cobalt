#ifndef CO_BASIC_BLOCK_H
#define CO_BASIC_BLOCK_H

#include "utils/hash_map.h"

typedef struct BasicBlock {
  HashMap *operands;
} BasicBlock;

BasicBlock *new_basic_block(void);

void *basic_block_get(BasicBlock *bb, char *name);

#endif // !CO_BASIC_BLOCK_H
