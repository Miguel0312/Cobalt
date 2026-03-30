#include "basic_block.h"
#include "ir/expr.h"
#include <stdlib.h>

BasicBlock *new_basic_block(char *label) {
  BasicBlock *bb = malloc(sizeof(BasicBlock));

  bb->stack_space = 0;
  bb->label = label;
  bb->exit_false = bb->exit_true = NULL;
  bb->expressions = new_list();

  return bb;
}

void basic_block_free(BasicBlock *bb) {
  const Node *cur = bb->expressions->root;

  while (cur != NULL) {
    Expr *expr = cur->data;
    expr_free(expr);
    cur = cur->next;
  }

  list_free(bb->expressions);

  free(bb->label);
  free(bb);
}
