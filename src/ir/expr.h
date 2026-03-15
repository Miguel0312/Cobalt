#ifndef CO_EXPR_H
#define CO_EXPR_H

#include <stdio.h>
typedef enum Operation { ADD, SUB, DIV, MUL, ASSIGN, RET } Operation;

typedef union OperandVal {
  int int_val;
  unsigned long address;
} OperandVal;

typedef enum OperandType { OT_INT, OT_ID } OperandType;

typedef struct Operand {
  OperandVal val;
  OperandType type;
  char *name;
} Operand;

typedef struct Expr {
  Operation op;
  Operand **params;
} Expr;

Operand *new_operand(OperandVal val, OperandType type, char *name);

Expr *new_expr(Operation op, int n, ...);

Expr *new_expr_v(Operation op, int n, va_list operands);

void print_expr(Expr *expr);

void print_operand(Operand *operand);

Expr *expr_free(Expr *expr);

#endif // !CO_EXPR_H
