#include "ir/expr.h"
#include "utils/list.h"
#include <stdio.h>
#ifndef CO_CODE_GEN_H

typedef enum { AO_CONST, AO_REGISTER, AO_ADDRESS } AssemblyOperandType;

typedef union {
  Operand *operand;
  char *reg;
} AssemblyOperandVal;

typedef struct {
  AssemblyOperandType type;
  AssemblyOperandVal val;
} AssemblyOperand;

typedef struct CodeGenerator {
  FILE *f;
  List *program;

  int hasError;
} CodeGenerator;

CodeGenerator *new_code_generator(List *expressions, FILE *f);

void generate_code(CodeGenerator *code_gen);

void visit_shift(CodeGenerator *code_gen, Expr *expr);

void visit_binary_op(CodeGenerator *code_gen, Expr *expr);

void visit_div(CodeGenerator *code_gen, Expr *expr);

void visit_assign(CodeGenerator *code_gen, Expr *expr);

void visit_ret(CodeGenerator *code_gen, Expr *expr);

void mov(CodeGenerator *code_gen, AssemblyOperand *src, AssemblyOperand *dst);

void print_assembly_operand(CodeGenerator *code_gen, AssemblyOperand *op);

void code_gen_report_error(CodeGenerator *code_gen, char *msg);

CodeGenerator *code_gen_free(CodeGenerator *code_gen);

#endif // !CO_CODE_GEN_H
