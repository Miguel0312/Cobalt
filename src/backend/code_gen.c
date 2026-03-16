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
    case ADD:
    case SUB:
    case MUL: {
      visit_binary_op(code_gen, expr);
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

void visit_binary_op(CodeGenerator *code_gen, Expr *expr) {
  char *instr = "";
  switch (expr->op) {
  case ADD: {
    instr = "add";
    break;
  }
  case SUB: {
    instr = "sub";
    break;
  }
  case MUL: {
    instr = "imul";
    break;
  }
  default: {
    code_gen_report_error(code_gen, "Given operation is not binary");
  }
  }

  Operand *dest = expr->params[0], *lhs = expr->params[1],
          *rhs = expr->params[2];

  // TODO: check three-operand form of the assembly instructions

  AssemblyOperand dest_op, lhs_op, rhs_op;
  dest_op.type = AO_ADDRESS;
  lhs_op.type = (lhs->type == OT_ID ? AO_ADDRESS : AO_CONST);
  rhs_op.type = (rhs->type == OT_ID ? AO_ADDRESS : AO_CONST);
  dest_op.val.operand = dest, lhs_op.val.operand = lhs,
  rhs_op.val.operand = rhs;

  // TODO: save this scratch variable somewhere instead of always instantiating
  // it
  AssemblyOperand scratch;
  scratch.type = AO_REGISTER, scratch.val.reg = "%eax";

  // TODO: when rhs is a constant one of the two moves can be bypassed
  // Also, when using commutative ADD, MUL, lhs and rhs can be swapped to
  // achieve this
  mov(code_gen, &lhs_op, &scratch);
  fprintf(code_gen->f, "%s ", instr);
  print_assembly_operand(code_gen, &rhs_op);
  fprintf(code_gen->f, ", ");
  print_assembly_operand(code_gen, &scratch);
  fprintf(code_gen->f, "\n");

  mov(code_gen, &scratch, &dest_op);
}

void visit_assign(CodeGenerator *code_gen, Expr *expr) {
  assert(code_gen != NULL);
  assert(expr != NULL);
  assert(expr->op == ASSIGN);
  AssemblyOperand op1, op2;

  op1.type = AO_ADDRESS;
  op1.val.operand = expr->params[0];

  op2.type = (expr->params[1]->type == OT_ID ? AO_ADDRESS : AO_CONST);
  op2.val.operand = expr->params[1];

  mov(code_gen, &op2, &op1);
}

void visit_ret(CodeGenerator *code_gen, Expr *expr) {
  assert(code_gen != NULL);
  assert(expr != NULL);
  assert(expr->op == RET);

  AssemblyOperand op1, op2;
  op2.type = AO_REGISTER;
  op1.val.operand = expr->params[0], op2.val.reg = "%eax";
  op1.type = (op1.val.operand->type == OT_ID ? AO_ADDRESS : AO_CONST);

  mov(code_gen, &op1, &op2);
  fprintf(code_gen->f, "popq %%rbp\n"
                       "ret\n");
}

void mov(CodeGenerator *code_gen, AssemblyOperand *src, AssemblyOperand *dest) {
  // TODO: check that the op types are valid. Can't move from operand to operand
  // for example
  AssemblyOperand *middle = NULL;
  AssemblyOperand scratch;
  int use_scratch = src->type == AO_ADDRESS && dest->type == AO_ADDRESS;
  scratch.type = AO_REGISTER, scratch.val.reg = "%eax";
  if (use_scratch) {
    middle = &scratch;
  } else {
    middle = dest;
  }

  fprintf(code_gen->f, "movl ");
  print_assembly_operand(code_gen, src);
  fprintf(code_gen->f, ", ");
  print_assembly_operand(code_gen, middle);
  fprintf(code_gen->f, "\n");

  if (use_scratch) {
    fprintf(code_gen->f, "movl ");
    print_assembly_operand(code_gen, middle);
    fprintf(code_gen->f, ", ");
    print_assembly_operand(code_gen, dest);
    fprintf(code_gen->f, "\n");
  }
}

void print_assembly_operand(CodeGenerator *code_gen, AssemblyOperand *op) {
  if (op->type == AO_REGISTER) {
    fprintf(code_gen->f, "%s", op->val.reg);
    return;
  }

  Operand *operand = op->val.operand;

  if (op->type == AO_ADDRESS) {
    fprintf(code_gen->f, "-%lu(%%rbp)", operand->val.address);
  } else if (op->type == AO_CONST) {
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
