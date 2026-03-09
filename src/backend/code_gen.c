#include "code_gen.h"
#include <assert.h>
#include <stdlib.h>

CodeGenerator *new_code_generator(List *expressions, FILE *f) {
  assert(expressions != NULL);
  assert(f != NULL);

  CodeGenerator *code_gen = malloc(sizeof(CodeGenerator));

  code_gen->program = expressions;
  code_gen->f = f;

  generate_code(code_gen);

  return code_gen;
}

void generate_code(CodeGenerator *code_gen) {
  assert(code_gen != NULL);

  fprintf(code_gen->f, ".text\n"
                       ".globl main\n"
                       "main:\n"
                       "pushq %%rbp\n"
                       "movq %%rsp, %%rbp\n");

  Node *cur = code_gen->program->root;
  while (cur != NULL) {
    Expr *expr = cur->data;

    switch (expr->op) {
    case RET: {
      visit_ret(code_gen, expr);
      break;
    }
    default:
      assert(0);
    }

    cur = cur->next;
  }
}

void visit_ret(CodeGenerator *code_gen, Expr *expr) {
  assert(code_gen != NULL);
  assert(expr != NULL);
  assert(expr->op == RET);
  fprintf(code_gen->f,
          "movl $%d, %%eax\n"
          "popq %%rbp\n"
          "ret\n",
          expr->params[0]->val);
}
