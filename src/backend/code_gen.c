#include "code_gen.h"
#include "utils/constants.h"
#include <assert.h>
#include <stdlib.h>

#define OPERAND(r) &(r).reg[(r).index]

CodeGenerator *new_code_generator(CFG *cfg, FILE *f) {
  assert(cfg != NULL);
  assert(f != NULL);

  CodeGenerator *code_gen = malloc(sizeof(CodeGenerator));

  code_gen->cfg = cfg;
  code_gen->f = f;
  code_gen->hasError = 0;

  code_gen->A[0].val.reg.name = "%al";
  code_gen->A[1].val.reg.name = "%ax";
  code_gen->A[2].val.reg.name = "%eax";
  code_gen->A[3].val.reg.name = "%rax";

  code_gen->B[0].val.reg.name = "%bl";
  code_gen->B[1].val.reg.name = "%bx";
  code_gen->B[2].val.reg.name = "%ebx";
  code_gen->B[3].val.reg.name = "%rbx";

  code_gen->C[0].val.reg.name = "%cl";
  code_gen->C[1].val.reg.name = "%cx";
  code_gen->C[2].val.reg.name = "%ecx";
  code_gen->C[3].val.reg.name = "%rcx";

  code_gen->D[0].val.reg.name = "%dl";
  code_gen->D[1].val.reg.name = "%dx";
  code_gen->D[2].val.reg.name = "%edx";
  code_gen->D[3].val.reg.name = "%rdx";

  for (int i = 0; i < 4; i++) {
    code_gen->A[i].sz = code_gen->B[i].sz = code_gen->C[i].sz =
                                            code_gen->D[i].sz = (1 << i);
    code_gen->A[i].type = code_gen->B[i].type = code_gen->C[i].type =
                                                code_gen->D[i].type = AO_REGISTER;
  }

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

  const Node *cur = code_gen->cfg->bbs->root;
  while (cur != NULL) {
    visit_bb(code_gen, cur->data);
    cur = cur->next;
  }
}

void visit_bb(CodeGenerator *code_gen, const BasicBlock *bb) {
  const Node *cur = bb->expressions->root;

  fprintf(code_gen->f, "%s:\n", bb->label);

  while (cur != NULL) {
    const Expr *expr = cur->data;

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
      case MUL:
      case B_OR:
      case B_AND:
      case B_XOR: {
        visit_binary_op(code_gen, expr);
        break;
      }
      case DIV:
      case MOD: {
        visit_div(code_gen, expr);
        break;
      }
      case LEFT_SHIFT:
      case RIGHT_SHIFT: {
        visit_shift(code_gen, expr);
        break;
      }
      case IS_LESS:
      case IS_LESS_EQUAL:
      case IS_GREATER:
      case IS_GREATER_EQUAL:
      case IS_EQUAL:
      case IS_DIF: {
        visit_cmp(code_gen, expr);
        break;
      }
      case TEST: {
        visit_test(code_gen, expr);
        break;
      }
      default: {
        char msg[MSG_BUFFER_SIZE];
        snprintf(msg, MSG_BUFFER_SIZE, "Operation %s not implemented\n",
                 operation_to_string(expr->op));
        code_gen_report_error(code_gen, msg);
      }
    }

    cur = cur->next;
  }

  if (bb->exit_true != NULL && bb->exit_false == bb->exit_true) {
    fprintf(code_gen->f, "jmp %s\n", bb->exit_true->label);
    return;
  }

  if (bb->exit_true != NULL) {
    fprintf(code_gen->f, "jnz %s\n", bb->exit_true->label);
  }
  if (bb->exit_false != NULL) {
    fprintf(code_gen->f, "jz %s\n", bb->exit_false->label);
  }
}

void visit_shift(CodeGenerator *code_gen, const Expr *expr) {
  const RegRepr ecx = {.index = 2, .reg = code_gen->C};

  RegRepr cl = ecx;
  cl.index -= 2;

  const RegRepr scratch = {.index = 2, .reg = code_gen->A};

  char *instr = expr->op == LEFT_SHIFT ? "sall" : "sarl";
  Operand *dest = expr->params[0], *lhs = expr->params[1],
      *rhs = expr->params[2];

  AssemblyOperand dest_op, lhs_op, rhs_op;
  dest_op.type = AO_ADDRESS;
  lhs_op.type = lhs->op_type == OT_ID ? AO_ADDRESS : AO_CONST;
  rhs_op.type = rhs->op_type == OT_ID ? AO_ADDRESS : AO_CONST;

  if (lhs_op.type == AO_ADDRESS) {
    lhs_op.sz = get_var_size(lhs->data_type);
  }
  if (rhs_op.type == AO_ADDRESS) {
    rhs_op.sz = get_var_size(rhs->data_type);
  }
  dest_op.sz = get_var_size(dest->data_type);

  dest_op.val.operand = dest, lhs_op.val.operand = lhs,
      rhs_op.val.operand = rhs;

  mov(code_gen, &rhs_op, OPERAND(ecx));
  mov(code_gen, &lhs_op, OPERAND(scratch));

  // TODO: make a function to print this from a string and two
  // AssemblyOperands
  fprintf(code_gen->f, "%s ", instr);
  print_assembly_operand(code_gen, OPERAND(cl));
  fprintf(code_gen->f, ", ");
  print_assembly_operand(code_gen, OPERAND(scratch));
  fprintf(code_gen->f, "\n");

  mov(code_gen, OPERAND(scratch), &dest_op);
}

void visit_binary_op(CodeGenerator *code_gen, const Expr *expr) {
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
    case B_AND: {
      instr = "and";
      break;
    }
    case B_OR: {
      instr = "or";
      break;
    }
    case B_XOR: {
      instr = "xor";
      break;
    }
    default: {
      char msg[MSG_BUFFER_SIZE];
      snprintf(msg, MSG_BUFFER_SIZE, "Given operation %s is not binary",
               operation_to_string(expr->op));
      code_gen_report_error(code_gen, msg);
    }
  }

  Operand *dest = expr->params[0], *lhs = expr->params[1],
      *rhs = expr->params[2];

  AssemblyOperand dest_op, lhs_op, rhs_op;
  dest_op.type = AO_ADDRESS;

  lhs_op.type = (lhs->op_type == OT_ID ? AO_ADDRESS : AO_CONST);
  rhs_op.type = (rhs->op_type == OT_ID ? AO_ADDRESS : AO_CONST);
  dest_op.val.operand = dest, lhs_op.val.operand = lhs,
      rhs_op.val.operand = rhs;

  if (lhs_op.type == AO_ADDRESS) {
    lhs_op.sz = get_var_size(lhs->data_type);
  }
  if (rhs_op.type == AO_ADDRESS) {
    rhs_op.sz = get_var_size(rhs->data_type);
  }
  dest_op.sz = get_var_size(dest->data_type);

  const RegRepr scratch = {.reg = code_gen->A, .index = 2};

  mov(code_gen, &lhs_op, OPERAND(scratch));

  const AssemblyOperand *actual_rhs = &rhs_op;

  if (rhs_op.type == AO_ADDRESS && rhs->data_type == CHAR) {
    const RegRepr scratch2 = {.reg = code_gen->D, .index = 2};
    mov(code_gen, &rhs_op, OPERAND(scratch2));
    actual_rhs = OPERAND(scratch2);
  }

  fprintf(code_gen->f, "%s ", instr);
  print_assembly_operand(code_gen, actual_rhs);
  fprintf(code_gen->f, ", ");
  print_assembly_operand(code_gen, OPERAND(scratch));
  fprintf(code_gen->f, "\n");

  mov(code_gen, OPERAND(scratch), &dest_op);
}

void visit_div(CodeGenerator *code_gen, const Expr *expr) {
  const RegRepr eax = {.reg = code_gen->A, .index = 2};
  const RegRepr edx = {.reg = code_gen->D, .index = 2};

  Operand *dest = expr->params[0], *lhs = expr->params[1],
      *rhs = expr->params[2];

  AssemblyOperand dest_op, lhs_op, rhs_op;
  dest_op.type = AO_ADDRESS;
  lhs_op.type = lhs->op_type == OT_ID ? AO_ADDRESS : AO_CONST;
  rhs_op.type = rhs->op_type == OT_ID ? AO_ADDRESS : AO_CONST;
  dest_op.val.operand = dest, lhs_op.val.operand = lhs,
      rhs_op.val.operand = rhs;

  if (lhs_op.type == AO_ADDRESS) {
    lhs_op.sz = get_var_size(lhs->data_type);
  }
  if (rhs_op.type == AO_ADDRESS) {
    rhs_op.sz = get_var_size(rhs->data_type);
  }
  dest_op.sz = get_var_size(dest->data_type);

  mov(code_gen, &lhs_op, OPERAND(eax));
  fprintf(code_gen->f, "cdq\n");

  if (rhs_op.type == AO_CONST) {
    const RegRepr ecx = {.reg = code_gen->C, .index = 2};
    mov(code_gen, &rhs_op, OPERAND(ecx));
    rhs_op = ecx.reg[ecx.index];
  }

  fprintf(code_gen->f, "div ");
  print_assembly_operand(code_gen, &rhs_op);
  fprintf(code_gen->f, "\n");

  if (expr->op == DIV) {
    mov(code_gen, OPERAND(eax), &dest_op);
  } else if (expr->op == MOD) {
    mov(code_gen, OPERAND(edx), &dest_op);
  }
}

void visit_assign(CodeGenerator *code_gen, const Expr *expr) {
  assert(code_gen != NULL);
  assert(expr != NULL);
  assert(expr->op == ASSIGN);
  AssemblyOperand op1, op2;

  op1.type = AO_ADDRESS;
  op1.val.operand = expr->params[0];

  op2.type = expr->params[1]->op_type == OT_ID ? AO_ADDRESS : AO_CONST;
  op2.val.operand = expr->params[1];

  if (op2.type == AO_ADDRESS) {
    op2.sz = get_var_size(expr->params[1]->data_type);
  }
  op1.sz = get_var_size(expr->params[0]->data_type);

  mov(code_gen, &op2, &op1);
}

void visit_ret(CodeGenerator *code_gen, const Expr *expr) {
  assert(code_gen != NULL);
  assert(expr != NULL);
  assert(expr->op == RET);

  AssemblyOperand op1;
  op1.val.operand = expr->params[0];
  op1.type = (op1.val.operand->op_type == OT_ID ? AO_ADDRESS : AO_CONST);
  const RegRepr op2 = {.reg = code_gen->A, .index = 2};

  if (op1.type == AO_ADDRESS) {
    op1.sz = get_var_size(expr->params[0]->data_type);
  }

  mov(code_gen, &op1, OPERAND(op2));
  fprintf(code_gen->f, "popq %%rbp\n"
          "ret\n");
}

void visit_test(CodeGenerator *code_gen, const Expr *expr) {
  const RegRepr eax = {.reg = code_gen->A, .index = 2};

  AssemblyOperand op1;
  op1.val.operand = expr->params[0];
  op1.type = op1.val.operand->op_type == OT_ID ? AO_ADDRESS : AO_CONST;
  if (op1.type == AO_ADDRESS) {
    op1.sz = get_var_size(expr->params[0]->data_type);
  }

  mov(code_gen, &op1, OPERAND(eax));

  fprintf(code_gen->f, "testl %%eax, %%eax\n");
}

void visit_cmp(CodeGenerator *code_gen, const Expr *expr) {
  const RegRepr eax = {.reg = code_gen->A, .index = 2};
  const RegRepr al = {.reg = code_gen->A, .index = 0};

  Operand *dest = expr->params[0], *lhs = expr->params[1],
      *rhs = expr->params[2];

  AssemblyOperand dest_op, lhs_op, rhs_op;
  dest_op.type = AO_ADDRESS;
  lhs_op.type = (lhs->op_type == OT_ID ? AO_ADDRESS : AO_CONST);
  rhs_op.type = (rhs->op_type == OT_ID ? AO_ADDRESS : AO_CONST);
  dest_op.val.operand = dest, lhs_op.val.operand = lhs,
      rhs_op.val.operand = rhs;

  if (lhs_op.type == AO_ADDRESS) {
    lhs_op.sz = get_var_size(lhs->data_type);
  }
  if (rhs_op.type == AO_ADDRESS) {
    rhs_op.sz = get_var_size(rhs->data_type);
  }
  dest_op.sz = get_var_size(dest->data_type);

  mov(code_gen, &lhs_op, OPERAND(eax));

  fprintf(code_gen->f, "cmpl ");
  print_assembly_operand(code_gen, &rhs_op);
  fprintf(code_gen->f, ", ");
  print_assembly_operand(code_gen, OPERAND(eax));
  fprintf(code_gen->f, "\n");

  char *instr;
  switch (expr->op) {
    case IS_EQUAL:
      instr = "sete";
      break;
    case IS_DIF:
      instr = "setne";
      break;
    case IS_GREATER:
      instr = "setg";
      break;
    case IS_GREATER_EQUAL:
      instr = "setge";
      break;
    case IS_LESS:
      instr = "setl";
      break;
    case IS_LESS_EQUAL:
      instr = "setle";
      break;
    default:
      assert(0);
  }

  fprintf(code_gen->f, "%s ", instr);
  print_assembly_operand(code_gen, OPERAND(al));
  fprintf(code_gen->f, "\n");

  mov(code_gen, OPERAND(al), OPERAND(eax));
  mov(code_gen, OPERAND(eax), &dest_op);
}

void mov(CodeGenerator *code_gen, const AssemblyOperand *src, const AssemblyOperand *dst) {
  // TODO: check that the op types are valid. Can't move from operand to
  // operand for example
  const AssemblyOperand *middle = NULL, *middle2 = NULL;
  const int use_scratch = src->type == AO_ADDRESS && dst->type == AO_ADDRESS;

  const RegRepr eax = {.reg = code_gen->A, .index = 2};
  const RegRepr al = {.reg = code_gen->A, .index = 0};

  if (use_scratch) {
    middle = middle2 = OPERAND((src->sz == 1) ? al : eax);
  } else {
    middle = dst;
  }

  char *instr1, *instr2 = "movl";
  if (src->type == AO_CONST) {
    instr1 = (dst->sz == 4 ? "movl" : "movb");
  } else if (src->sz == dst->sz) {
    instr1 = (src->sz == 4 ? "movl" : "movb");
    if (use_scratch) {
      instr2 = instr1;
    }
  } else if (src->sz == 1 && dst->sz == 4) {
    instr1 = (src->type == AO_REGISTER ? "movzbl" : "movsbl");
    if (use_scratch) {
      instr2 = "movl";
    }
  } else {
    if (src->type == AO_ADDRESS) {
      instr1 = "movl";
      if (dst->type == AO_ADDRESS) {
        instr2 = "movb";
        middle2 = OPERAND(al);
      } else {
        AssemblyOperand *reg = code_gen->A + 4 * (dst->val.reg.name[2] - 'a');
        const RegRepr reg_reduc = {.reg = reg, .index = 0};
        dst = OPERAND(reg_reduc);
      }
    } else {
      instr1 = "movb";
      AssemblyOperand *reg = code_gen->A + 4 * (src->val.reg.name[2] - 'a');
      const RegRepr reg_reduc = {.reg = reg, .index = 0};
      src = OPERAND(reg_reduc);
    }
  }

  fprintf(code_gen->f, "%s ", instr1);
  print_assembly_operand(code_gen, src);
  fprintf(code_gen->f, ", ");
  print_assembly_operand(code_gen, middle);
  fprintf(code_gen->f, "\n");

  if (use_scratch) {
    fprintf(code_gen->f, "%s ", instr2);
    print_assembly_operand(code_gen, middle2);
    fprintf(code_gen->f, ", ");
    print_assembly_operand(code_gen, dst);
    fprintf(code_gen->f, "\n");
  }
}

void print_assembly_operand(const CodeGenerator *code_gen, const AssemblyOperand *op) {
  if (op->type == AO_REGISTER) {
    fprintf(code_gen->f, "%s", op->val.reg.name);
    return;
  }

  const Operand *operand = op->val.operand;

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

void code_gen_free(CodeGenerator *code_gen) { free(code_gen); }
