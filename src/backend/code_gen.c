#include "code_gen.h"
#include "utils/constants.h"
#include <assert.h>
#include <stdlib.h>

CodeGenerator *new_code_generator(List *expressions, FILE *f) {
  assert(expressions != NULL);
  assert(f != NULL);

  CodeGenerator *code_gen = malloc(sizeof(CodeGenerator));

  code_gen->program = expressions;
  code_gen->f = f;
  code_gen->hasError = 0;

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
    default: {
      char msg[MSG_BUFFER_SIZE];
      snprintf(msg, MSG_BUFFER_SIZE, "Operation %d not implemented\n",
               expr->op);
      code_gen_report_error(code_gen, msg);
    }
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

void code_gen_report_error(CodeGenerator *code_gen, char *msg) {
  assert(code_gen != NULL);

  code_gen->hasError = 1;
  fprintf(stderr, "%s", msg);
}

CodeGenerator *code_gen_free(CodeGenerator *code_gen) {
  if (code_gen == NULL)
    return NULL;

  free(code_gen);

  return NULL;
}
