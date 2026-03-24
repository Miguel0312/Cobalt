#include "cfg.h"

#include <assert.h>
#include <stdlib.h>

#include "ir/basic_block.h"
#include "utils/list.h"

static int bb_cnt = 0;

CFG *new_cfg(void) {
  CFG *cfg = malloc(sizeof(CFG));

  cfg->bbs = new_list();
  cfg->bb_stack = new_stack();
  cfg->symbol_table = new_stack();
  cfg->operands = new_list();
  cfg->offset = 0;

  return cfg;
}

BasicBlock *cfg_push_bb(CFG *cfg) {
  char *label = malloc(8);
  snprintf(label, 8, ".BB%d", bb_cnt++);

  BasicBlock *bb = new_basic_block(label);

  list_append(cfg->bbs, bb);
  stack_push(cfg->bb_stack, bb);

  return bb;
}

BasicBlock *cfg_pop_bb(CFG *cfg) {
  // TODO: needs to find a difference between a block that is actually popped
  // and when an if causes the creation of two blocks cfg->offset -=

  // cfg_get_cur_bb(cfg)->stack_space;
  return stack_pop(cfg->bb_stack);
}

void cfg_push_symbol_table(CFG *cfg) {
  HashMap *symbol_table = new_hash_map(string_hash, string_cmp);

  stack_push(cfg->symbol_table, symbol_table);
}

void cfg_pop_symbol_table(CFG *cfg) {
  // TODO: extract symbol table into a struct, keep track of the stack space
  // occupied by the operands in it and free it here

  // cfg_get_cur_bb(cfg)->stack_space;
  hash_map_free(stack_pop(cfg->symbol_table));
}

Operand *cfg_get_var(CFG *cfg, char *name) {
  Node *cur = cfg->symbol_table->elements->end;

  while (cur != NULL) {
    Operand *res = hash_map_get(cur->data, name);
    if (res != NULL) {
      return res;
    }
    cur = cur->prev;
  }

  return NULL;
}

int cfg_has_var_in_scope(CFG *cfg, char *name) {
  Node *cur = cfg->symbol_table->elements->end;
  return hash_map_get(cur->data, name) != NULL;
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

  hash_map_insert(cfg->symbol_table->elements->end->data, name, operand);

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

  stack_free(cfg->symbol_table);

  free(cfg);
}
