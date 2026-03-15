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
    case ASSIGN: {
      visit_assign(code_gen, expr);
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

void visit_assign(CodeGenerator *code_gen, Expr *expr) {
  assert(code_gen != NULL);
  assert(expr != NULL);
  assert(expr->op == ASSIGN);
  AssemblyOperand op1, op2, op3;
  op1.type = AO_OPERAND, op2.type = AO_REGISTER, op3.type = AO_OPERAND;
  op1.val.operand = expr->params[1], op2.val.reg = "%eax",
  op3.val.operand = expr->params[0];
  mov(code_gen, op1, op2);
  mov(code_gen, op2, op3);
}

void visit_ret(CodeGenerator *code_gen, Expr *expr) {
  assert(code_gen != NULL);
  assert(expr != NULL);
  assert(expr->op == RET);

  AssemblyOperand op1, op2;
  op1.type = AO_OPERAND, op2.type = AO_REGISTER;
  op1.val.operand = expr->params[0], op2.val.reg = "%eax";

  mov(code_gen, op1, op2);
  fprintf(code_gen->f, "popq %%rbp\n"
                       "ret\n");
}

void mov(CodeGenerator *code_gen, AssemblyOperand op1, AssemblyOperand op2) {
  // TODO: check that the op types are valid. Can't move from operand to operand
  // for example
  fprintf(code_gen->f, "movl ");
  print_assembly_operand(code_gen, op1);
  fprintf(code_gen->f, ", ");
  print_assembly_operand(code_gen, op2);
  fprintf(code_gen->f, "\n");
}

void print_assembly_operand(CodeGenerator *code_gen, AssemblyOperand op) {
  if (op.type == AO_REGISTER) {
    fprintf(code_gen->f, "%s", op.val.reg);
    return;
  }

  Operand *operand = op.val.operand;

  if (operand->type == OT_ID) {
    fprintf(code_gen->f, "-%lu(%%rbp)", operand->val.address);
  } else if (operand->type == OT_INT) {
    fprintf(code_gen->f, "$%d", operand->val.int_val);
  }
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
