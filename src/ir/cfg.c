#include "cfg.h"

#include <stdlib.h>

#include "ir/basic_block.h"
#include "utils/list.h"

CFG *new_cfg(void) {
  CFG *cfg = malloc(sizeof(CFG));

  cfg->bbs = new_list();
  cfg->bb_stack = new_stack();
  cfg->operands = new_list();
  cfg->offset = 0;

  return cfg;
}

void cfg_push_bb(CFG *cfg) {
  BasicBlock *bb = new_basic_block();

  list_append(cfg->bbs, bb);
  stack_push(cfg->bb_stack, bb);
}

void cfg_pop_bb(CFG *cfg) {
  cfg->offset -= cfg_get_cur_bb(cfg)->stack_space;
  stack_pop(cfg->bb_stack);
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

Operand *cfg_add_var(CFG *cfg, DataType data_type, OperandType op_type,
                     char *name) {
  BasicBlock *bb = cfg_get_cur_bb(cfg);

  int delta = get_var_size(data_type);

  cfg->offset += delta;
  bb->stack_space += delta;

  unsigned long address = cfg->offset;
  OperandVal val = {.address = address};
  Operand *operand = new_operand(val, data_type, op_type, name);

  list_append(cfg->operands, operand);

  hash_map_insert(bb->operands, name, operand);

  return operand;
}

Operand *cfg_add_tmp(CFG *cfg, DataType data_type) {
  BasicBlock *bb = cfg_get_cur_bb(cfg);

  int delta = get_var_size(data_type);

  cfg->offset += delta;
  bb->stack_space += delta;

  unsigned long address = cfg->offset;
  OperandVal val = {.address = address};
  char *name = malloc(10);

  snprintf(name, 10, "!%lu", cfg->offset);

  Operand *operand = new_operand(val, data_type, OT_ID, name);

  list_append(cfg->operands, operand);

  return operand;
}

void cfg_free(CFG *cfg) {
  Node *cur = cfg->bbs->root;

  while (cur != NULL) {
    BasicBlock *bb = cur->data;
    basic_block_free(bb);
    cur = cur->next;
  }

  list_free(cfg->bbs);

  cur = cfg->operands->root;
  while (cur != NULL) {
    Operand *operand = cur->data;
    operand_free(operand);
    cur = cur->next;
  }
  list_free(cfg->operands);

  stack_free(cfg->bb_stack);

  free(cfg);
}
