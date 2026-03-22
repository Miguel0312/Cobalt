#include "expr.h"
#include <stdarg.h>
#include <stdlib.h>

Operand *new_operand(OperandVal val, DataType data_type, OperandType op_type,
                     char *name) {
  Operand *operand = malloc(sizeof(Operand));

  operand->val = val;
  operand->op_type = op_type;
  operand->data_type = data_type;
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
  case MOD:
  case B_OR:
  case B_AND:
  case B_XOR:
  case LEFT_SHIFT:
  case RIGHT_SHIFT: {
    char *op_str = "?";
    switch (expr->op) {
    case ADD: {
      op_str = "+";
      break;
    }
    case SUB: {
      op_str = "-";
      break;
    }
    case MUL: {
      op_str = "*";
      break;
    }
    case DIV: {
      op_str = "/";
      break;
    }
    case MOD: {
      op_str = "%";
      break;
    }
    case B_OR: {
      op_str = "|";
      break;
    }
    case B_AND: {
      op_str = "&";
      break;
    }
    case B_XOR: {
      op_str = "^";
      break;
    }
    case LEFT_SHIFT: {
      op_str = "<<";
      break;
    }
    case RIGHT_SHIFT: {
      op_str = ">>";
    }
    default: {
    }
    }
    print_binary_expr(expr->params[0], expr->params[1], expr->params[2],
                      op_str);
    return;
  }
  }

  fprintf(stderr, "Print has not been defined for operation %d\n", expr->op);
}

void print_binary_expr(Operand *op1, Operand *op2, Operand *op3, char *op_str) {
  print_operand(op1);
  printf(" = ");
  print_operand(op2);
  printf(" %s ", op_str);
  print_operand(op3);
  printf("\n");
}

void print_operand(Operand *operand) {
  if (operand->type == OT_INT)
    printf("%d", operand->val.int_val);
  else if (operand->type == OT_ID)
    printf("%s", operand->name);
}

char *operation_to_string(Operation op) {
  switch (op) {
  case ADD:
    return "ADD";
  case SUB:
    return "SUB";
  case DIV:
    return "DIV";
  case MUL:
    return "MUL";
  case B_OR:
    return "B_OR";
  case B_XOR:
    return "B_XOR";
  case B_AND:
    return "B_AND";
  case ASSIGN:
    return "ASSIGN";
  case MOD:
    return "MOD";
  case LEFT_SHIFT:
    return "LEFT_SHIFT";
  case RIGHT_SHIFT:
    return "RIGHT_SHIFT";
  case RET:
    return "RET";
  }

  return "ERROR";
}

Expr *expr_free(Expr *expr) {
  if (expr == NULL)
    return NULL;

  free(expr->params);
  free(expr);

  return NULL;
}
