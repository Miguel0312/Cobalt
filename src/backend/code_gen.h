#include "ir/expr.h"
#include "utils/list.h"
#include <stdio.h>
#ifndef CO_CODE_GEN_H

typedef struct CodeGenerator {
  FILE *f;
  List *program;
} CodeGenerator;

CodeGenerator *new_code_generator(List *expressions, FILE *f);

void generate_code(CodeGenerator *code_gen);

void visit_ret(CodeGenerator *code_gen, Expr *expr);

#endif // !CO_CODE_GEN_H
