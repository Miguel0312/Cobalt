#ifndef CO_CFG_H
#define CO_CFG_H

#include "ir/basic_block.h"
#include "ir/expr.h"
#include "utils/list.h"
#include "utils/stack.h"

typedef struct CFG {
    List *bbs;
    Stack *bb_stack;
    // Keep track of all operands to be able to free them (once and only once)
    Stack *symbol_table;
    List *operands;
    unsigned long offset;
} CFG;

CFG *new_cfg(void);

BasicBlock *cfg_push_bb(const CFG *cfg);

BasicBlock *cfg_pop_bb(const CFG *cfg);

void cfg_push_symbol_table(const CFG *cfg);

void cfg_pop_symbol_table(const CFG *cfg);

Operand *cfg_get_var(const CFG *cfg, char *name);

Operand *cfg_add_var(CFG *cfg, DataType data_type, OperandType op_type,
                     char *name);

int cfg_has_var_in_scope(const CFG *cfg, char *name);

Operand *cfg_add_tmp(CFG *cfg, DataType data_type);

void cfg_free(CFG *cfg);

static inline BasicBlock *cfg_get_cur_bb(const CFG *cfg) {
    return cfg->bb_stack->elements->end->data;
}

#endif // !CO_CFG_H
