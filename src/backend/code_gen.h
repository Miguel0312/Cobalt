#include "ir/expr.h"
#include "utils/list.h"
#include <stdio.h>
#ifndef CO_CODE_GEN_H

typedef enum { AO_CONST, AO_REGISTER, AO_ADDRESS } AssemblyOperandType;

typedef struct Register {
  char *name;
} Register;

typedef union {
  Operand *operand;
  Register reg;
} AssemblyOperandVal;

typedef struct {
  AssemblyOperandType type;
  AssemblyOperandVal val;
  unsigned int sz;
} AssemblyOperand;

typedef struct RegRepr {
  AssemblyOperand *reg;
  int index;
} RegRepr;

typedef struct CodeGenerator {
  FILE *f;
  List *program;

  int hasError;

  AssemblyOperand A[4], B[4], C[4], D[4];
} CodeGenerator;

CodeGenerator *new_code_generator(List *expressions, FILE *f);

void code_gen_free(CodeGenerator *code_gen);

void generate_code(CodeGenerator *code_gen);

void visit_shift(CodeGenerator *code_gen, Expr *expr);

void visit_binary_op(CodeGenerator *code_gen, Expr *expr);

void visit_div(CodeGenerator *code_gen, Expr *expr);

void visit_assign(CodeGenerator *code_gen, Expr *expr);

void visit_ret(CodeGenerator *code_gen, Expr *expr);

void mov(CodeGenerator *code_gen, AssemblyOperand *src, AssemblyOperand *dst);

void print_assembly_operand(CodeGenerator *code_gen, AssemblyOperand *op);

void code_gen_report_error(CodeGenerator *code_gen, char *msg);

#endif // !CO_CODE_GEN_H
