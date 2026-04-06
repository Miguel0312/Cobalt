#include "expr.h"
#include <stdarg.h>
#include <stdlib.h>

Operand *new_operand(const OperandVal val, const DataType data_type, const OperandType op_type,
                     char *name) {
  Operand *operand = malloc(sizeof(Operand));

  operand->val = val;
  operand->op_type = op_type;
  operand->data_type = data_type;
  operand->name = name;

  return operand;
}

Expr *new_expr(const Operation op, const int n, ...) {
  va_list operands;
  va_start(operands, n);
  Expr *expr = new_expr_v(op, n, operands);
  va_end(operands);
  return expr;
}

Expr *new_expr_v(const Operation op, const int n, va_list operands) {
  Expr *expr = malloc(sizeof(Expr));

  expr->op = op;
  expr->params = malloc(n * sizeof(Operand *));

  for (int i = 0; i < n; i++) {
    expr->params[i] = va_arg(operands, Operand *);
  }

  return expr;
}

void print_expr(const Expr *expr) {
  switch (expr->op) {
    case RET: {
      const Operand *operand = expr->params[0];
      printf("RET ");
      print_operand(operand);
      printf("\n");
      return;
    }
    case ASSIGN:
    case L_NOT: {
      const Operand *lhs = expr->params[0];
      const Operand *rhs = expr->params[1];
      print_operand(lhs);
      printf(" = ");
      if (expr->op == L_NOT) {
        printf("!");
      }
      print_operand(rhs);
      printf("\n");
      return;
    }
    case INC:
    case DEC: {
      const Operand *lhs = expr->params[0];
      char *instr = (expr->op == INC ? "++" : "--");
      print_operand(lhs);
      printf("%s\n", instr);
      return;
    }
    case TEST: {
      const Operand *operand = expr->params[0];
      printf("TEST ");
      print_operand(operand);
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
    case RIGHT_SHIFT:
    case IS_LESS:
    case IS_LESS_EQUAL:
    case IS_GREATER:
    case IS_GREATER_EQUAL:
    case IS_EQUAL:
    case IS_DIF:
    case L_AND:
    case L_OR: {
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
          break;
        }
        case IS_LESS: {
          op_str = "<";
          break;
        }
        case IS_LESS_EQUAL: {
          op_str = "<=";
          break;
        }
        case IS_EQUAL: {
          op_str = "==";
          break;
        }
        case IS_DIF: {
          op_str = "!=";
          break;
        }
        case IS_GREATER: {
          op_str = ">";
          break;
        }
        case IS_GREATER_EQUAL: {
          op_str = ">=";
          break;
        }
        case L_AND: {
          op_str = "&&";
          break;
        }
        case L_OR: {
          op_str = "||";
          break;
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

void print_binary_expr(const Operand *op1, const Operand *op2, const Operand *op3, char *op_str) {
  print_operand(op1);
  printf(" = ");
  print_operand(op2);
  printf(" %s ", op_str);
  print_operand(op3);
  printf("\n");
}

void print_operand(const Operand *operand) {
  if (operand->op_type == OT_INT)
    printf("%d", operand->val.int_val);
  else if (operand->op_type == OT_CHAR)
    printf("'%c'", operand->val.int_val);
  else if (operand->op_type == OT_ID)
    printf("%s", operand->name);
}

char *operation_to_string(const Operation op) {
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
    case L_AND:
      return "L_AND";
    case L_OR:
      return "L_OR";
    case L_NOT:
      return "L_NOT";
    case INC:
      return "INC";
    case DEC:
      return "DEC";
    case ASSIGN:
      return "ASSIGN";
    case MOD:
      return "MOD";
    case LEFT_SHIFT:
      return "LEFT_SHIFT";
    case RIGHT_SHIFT:
      return "RIGHT_SHIFT";
    case IS_LESS:
      return "IS_LESS";
    case IS_LESS_EQUAL:
      return "IS_LESS_EQUAL";
    case IS_GREATER:
      return "IS_GREATER";
    case IS_GREATER_EQUAL:
      return "IS_GREATER_EQUAL";
    case IS_EQUAL:
      return "IS_EQUAL";
    case IS_DIF:
      return "IS_DIF";
    case TEST:
      return "TEST";
    case RET:
      return "RET";
  }

  return "ERROR";
}

void expr_free(Expr *expr) {
  if (expr == NULL)
    return;

  free(expr->params);
  free(expr);
}

void operand_free(Operand *operand) {
  // Is tmp?
  if (operand->name != NULL && operand->name[0] == '!')
    free(operand->name);
  free(operand);
}

DataType get_data_type_from_operands(const DataType type1, const DataType type2) {
  if (type1 == INT || type2 == INT) {
    return INT;
  }

  return CHAR;
}
