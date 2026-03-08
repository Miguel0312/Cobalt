#include "expr.h"
#include <stdarg.h>
#include <stdlib.h>

Operand *new_operand(int val) {
  Operand *operand = malloc(sizeof(Operand));

  operand->val = val;

  return operand;
}

Expr *new_expr(Operation op, int n, ...) {
  va_list operands;
  va_start(operands, n);
  Expr *expr = new_expr_v(op, n, operands);
  va_end(operands);
  return expr;
}

Expr *new_expr_v(Operation op, int n, va_list operands) {
  Expr *expr = malloc(sizeof(Expr));

  expr->op = op;
  expr->params = malloc(n * sizeof(Operand *));

  for (int i = 0; i < n; i++) {
    expr->params[i] = va_arg(operands, Operand *);
  }

  return expr;
}

void print_expr(Expr *expr) {
  if (expr->op == RET) {
    printf("RET %d\n", expr->params[0]->val);
    return;
  }

  fprintf(stderr, "Print has not been defined for operation %d\n", expr->op);
}
