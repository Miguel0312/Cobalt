#ifndef CO_BASIC_BLOCK_H
#define CO_BASIC_BLOCK_H

#include "ir/expr.h"
#include "utils/hash_map.h"

typedef struct BasicBlock {
  HashMap *operands;
  unsigned long offset;
} BasicBlock;

BasicBlock *new_basic_block(void);

void *basic_block_get(BasicBlock *bb, char *name);

Operand *basic_block_add_var(BasicBlock *bb, OperandType type, char *name);

#endif // !CO_BASIC_BLOCK_H
