#ifndef CO_EXPR_H
#define CO_EXPR_H

#include <stdio.h>
typedef enum Operation {
  ADD,
  SUB,
  DIV,
  MUL,
  B_OR,
  B_XOR,
  B_AND,
  L_AND,
  L_OR,
  LEFT_SHIFT,
  RIGHT_SHIFT,
  ASSIGN,
  MOD,
  TEST,
  IS_LESS,
  IS_LESS_EQUAL,
  IS_GREATER,
  IS_GREATER_EQUAL,
  IS_EQUAL,
  IS_DIF,
  RET
} Operation;

typedef union OperandVal {
  int int_val;
  unsigned long address;
} OperandVal;

typedef enum OperandType { OT_INT, OT_CHAR, OT_ID } OperandType;

typedef enum DataType { CHAR, INT, LABEL } DataType;

typedef struct Operand {
  OperandVal val;
  OperandType op_type;
  DataType data_type;
  char *name;
} Operand;

typedef struct Expr {
  Operation op;
  Operand **params;
} Expr;

Operand *new_operand(OperandVal val, DataType data_type, OperandType op_type,
                     char *name);

Expr *new_expr(Operation op, int n, ...);

Expr *new_expr_v(Operation op, int n, va_list operands);

void expr_free(Expr *expr);

void print_expr(Expr *expr);

void print_binary_expr(Operand *op1, Operand *op2, Operand *op3, char *op_str);

void print_operand(Operand *operand);

char *operation_to_string(Operation op);

void operand_free(Operand *operand);

DataType get_data_type_from_operands(DataType type1, DataType type2);

static inline int get_var_size(DataType type) {
  switch (type) {
  case INT:
    return 4;
  case CHAR:
    return 1;
  case LABEL:
    return 0;
  }
  return 0;
}

#endif // !CO_EXPR_H
