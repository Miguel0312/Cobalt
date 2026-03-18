#include "basic_block.h"
#include <stdlib.h>

BasicBlock *new_basic_block(void) {
  BasicBlock *bb = malloc(sizeof(BasicBlock));

  bb->operands = new_hash_map(string_hash, string_cmp);

  return bb;
}

void *basic_block_get(BasicBlock *bb, char *name) {
  return hash_map_get(bb->operands, name);
}
