#include "basic_block.h"
#include <stdlib.h>

BasicBlock *new_basic_block(void) {
  BasicBlock *bb = malloc(sizeof(BasicBlock));

  bb->operands = new_hash_map(string_hash, string_cmp);
  bb->offset = 0;

  return bb;
}

void *basic_block_get(BasicBlock *bb, char *name) {
  return hash_map_get(bb->operands, name);
}

Operand *basic_block_add_var(BasicBlock *bb, OperandType type, char *name) {
  bb->offset += 4;
  unsigned long address = bb->offset;
  OperandVal val = {.address = address};
  Operand *operand = new_operand(val, type, name);

  hash_map_insert(bb->operands, name, operand);

  return operand;
}

Operand *basic_block_add_tmp(BasicBlock *bb) {
  bb->offset += 4;
  unsigned long address = bb->offset;
  OperandVal val = {.address = address};
  char *name = malloc(10);

  snprintf(name, 10, "!%lu", bb->offset);
  printf("%s\n", name);

  Operand *operand = new_operand(val, OT_ID, name);

  return operand;
}
