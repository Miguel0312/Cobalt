#ifndef CO_CFG_H
#define CO_CFG_H

#include "ir/basic_block.h"
#include "ir/expr.h"
#include "utils/list.h"
#include "utils/stack.h"

typedef struct CFG {
  List *bbs;
  Stack *bb_stack;
  unsigned long offset;
} CFG;

CFG *new_cfg(void);

void cfg_create_bb(CFG *cfg);

Operand *cfg_get_var(CFG *cfg, char *name);

Operand *cfg_add_var(CFG *cfg, OperandType type, char *name);

Operand *cfg_add_tmp(CFG *cfg);

static inline BasicBlock *cfg_get_cur_bb(CFG *cfg) {
  return cfg->bb_stack->elements->end->data;
}

#endif // !CO_CFG_H
