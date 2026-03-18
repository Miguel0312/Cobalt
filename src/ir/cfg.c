#include "cfg.h"

#include <stdlib.h>

#include "ir/basic_block.h"
#include "utils/list.h"

CFG *new_cfg(void) {
  CFG *cfg = malloc(sizeof(CFG));

  cfg->bbs = new_list();
  cfg->bb_stack = new_stack();
  cfg->offset = 0;

  return cfg;
}

void cfg_create_bb(CFG *cfg) {
  BasicBlock *bb = new_basic_block();

  list_append(cfg->bbs, bb);
  stack_push(cfg->bb_stack, bb);
}

Operand *cfg_get_var(CFG *cfg, char *name) {
  Node *cur = cfg->bb_stack->elements->end;

  while (cur != NULL) {
    Operand *res = basic_block_get(cur->data, name);
    if (res != NULL) {
      return res;
    }
    cur = cur->prev;
  }

  return NULL;
}

Operand *cfg_add_var(CFG *cfg, OperandType type, char *name) {
  BasicBlock *bb = cfg_get_cur_bb(cfg);
  cfg->offset += 4;

  unsigned long address = cfg->offset;
  OperandVal val = {.address = address};
  Operand *operand = new_operand(val, type, name);

  hash_map_insert(bb->operands, name, operand);

  return operand;
}

Operand *cfg_add_tmp(CFG *cfg) {
  cfg->offset += 4;

  unsigned long address = cfg->offset;
  OperandVal val = {.address = address};
  char *name = malloc(10);

  snprintf(name, 10, "!%lu", cfg->offset);

  Operand *operand = new_operand(val, OT_ID, name);

  return operand;
}
