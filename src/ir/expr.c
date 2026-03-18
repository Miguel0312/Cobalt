#include "expr.h"
#include <stdarg.h>
#include <stdlib.h>

Operand *new_operand(OperandVal val, OperandType type, char *name) {
  Operand *operand = malloc(sizeof(Operand));

  operand->val = val;
  operand->type = type;
  operand->name = name;

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
  switch (expr->op) {
  case RET: {
    Operand *operand = expr->params[0];
    printf("RET ");
    print_operand(operand);
    printf("\n");
    return;
  }
  case ASSIGN: {
    Operand *lhs = expr->params[0];
    Operand *rhs = expr->params[1];
    print_operand(lhs);
    printf(" = ");
    print_operand(rhs);
    printf("\n");
    return;
  }
  case ADD:
  case SUB:
  case MUL:
  case DIV:
  case MOD: {
    char op_char = '?';
    switch (expr->op) {
    case ADD: {
      op_char = '+';
      break;
    }
    case SUB: {
      op_char = '-';
      break;
    }
    case MUL: {
      op_char = '*';
      break;
    }
    case DIV: {
      op_char = '/';
      break;
    }
    case MOD: {
      op_char = '%';
      break;
    }
    default: {
    }
    }
    print_binary_expr(expr->params[0], expr->params[1], expr->params[2],
                      op_char);
    return;
  }
  }

  fprintf(stderr, "Print has not been defined for operation %d\n", expr->op);
}

void print_binary_expr(Operand *op1, Operand *op2, Operand *op3, char op_char) {
  print_operand(op1);
  printf(" = ");
  print_operand(op2);
  printf(" %c ", op_char);
  print_operand(op3);
  printf("\n");
}

void print_operand(Operand *operand) {
  if (operand->type == OT_INT)
    printf("%d", operand->val.int_val);
  else if (operand->type == OT_ID)
    printf("%s", operand->name);
}

Expr *expr_free(Expr *expr) {
  if (expr == NULL)
    return NULL;

  free(expr->params);
  free(expr);

  return NULL;
}
