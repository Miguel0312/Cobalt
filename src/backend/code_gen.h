#include "ir/expr.h"
#include "utils/list.h"
#include <stdio.h>
#ifndef CO_CODE_GEN_H

typedef struct CodeGenerator {
  FILE *f;
  List *program;

  int hasError;
} CodeGenerator;

CodeGenerator *new_code_generator(List *expressions, FILE *f);

void generate_code(CodeGenerator *code_gen);

void visit_ret(CodeGenerator *code_gen, Expr *expr);

void code_gen_report_error(CodeGenerator *code_gen, char *msg);

CodeGenerator *code_gen_free(CodeGenerator *code_gen);

#endif // !CO_CODE_GEN_H
