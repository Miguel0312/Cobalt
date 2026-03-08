#ifndef CO_EXPR_H
#define CO_EXPR_H

#include <stdio.h>
typedef enum Operation { ADD, SUB, DIV, MUL, ASSIGN, RET } Operation;

typedef struct Operand {
  int val;
} Operand;

typedef struct Expr {
  Operation op;
  Operand **params;
} Expr;

Operand *new_operand(int val);

Expr *new_expr(Operation op, int n, ...);

Expr *new_expr_v(Operation op, int n, va_list operands);

void print_expr(Expr *expr);

#endif // !CO_EXPR_H
