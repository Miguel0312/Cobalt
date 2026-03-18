#include "parser.h"
#include "frontend/token.h"
#include "ir/expr.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MSG_BUFFER_SIZE 256

Parser *new_parser(List *tokens) {
  assert(tokens != NULL);

  Parser *parser = malloc(sizeof(Parser));

  parser->tokens = tokens;

  assert(tokens->root != NULL);
  assert(((Token *)tokens->end->data)->type == EOF_TOKEN);

  parser->cur_token = tokens->root;
  parser->program = new_list();
  parser->hasError = 0;
  parser->bb = new_basic_block();

  parse_program(parser);

  return parser;
}

void parse_program(Parser *parser) {
  parser_consume_token(parser, INT_TS);
  parser_assert_token_type(parser, IDENTIFIER);
  Token *main_token = parser_get_cur(parser);
  if (strcmp(main_token->lexeme, "main") != 0) {
    parser_report_error(parser, "Expected function to be called main");
  }
  parser_advance(parser);
  parser_consume_token(parser, LEFT_PAREN);
  parser_consume_token(parser, RIGHT_PAREN);
  parser_consume_token(parser, LEFT_BRACE);

  TokenType cur_type;
  while (!parser_is_at_end(parser) &&
         (cur_type = parser_get_cur(parser)->type) != RIGHT_BRACE) {
    if (cur_type == INT_TS) {
      var_decl(parser);
    } else if (cur_type == RETURN) {
      return_stmt(parser);
    } else if (cur_type == IDENTIFIER) {
      expr(parser);
      parser_consume_token(parser, SEMICOLON);
    } else {
      parser_report_error(parser, "Unexpected token");
    }
  }

  parser_consume_token(parser, RIGHT_BRACE);
  if (!parser_is_at_end(parser)) {
    parser_report_error(parser, "Expected EOF");
  }
}

int parser_is_at_end(Parser *parser) {
  assert(parser != NULL);
  return parser_get_cur(parser)->type == EOF_TOKEN;
}

Token *parser_get_cur(Parser *parser) {
  assert(parser != NULL);
  return parser->cur_token->data;
}

Token *parser_peek(Parser *parser) {
  if (parser_is_at_end(parser))
    return parser_get_cur(parser);
  return parser->cur_token->next->data;
}

Token *parser_advance(Parser *parser) {
  assert(parser != NULL);

  Token *token = parser->cur_token->data;

  if (!parser_is_at_end(parser)) {
    parser->cur_token = parser->cur_token->next;
  }

  return token;
}

void parser_report_error(Parser *parser, char *msg) {
  assert(parser != NULL);
  fprintf(stderr, "[Line %d]: %s\n", parser_get_cur(parser)->line, msg);
  parser->hasError = 1;
  parser_panic(parser);
}

void parser_panic(Parser *parser) {
  assert(parser != NULL);
  while (!parser_is_at_end(parser) &&
         (parser_consume_if_not(parser, SEMICOLON))) {
  }
  if (!parser_is_at_end(parser)) {
    // Consume semicolon
    parser_advance(parser);
  }
}

int parser_assert_token_type(Parser *parser, TokenType type) {
  assert(parser != NULL);
  if (parser_get_cur(parser)->type == type) {
    return 1;
  }

  char msg[MSG_BUFFER_SIZE];
  snprintf(msg, MSG_BUFFER_SIZE,
           "Unexpected token type. Expected: %s, Got: %s\n",
           token_type_to_string(type),
           token_type_to_string(parser_get_cur(parser)->type));
  parser_report_error(parser, msg);

  return 0;
}

int parser_consume_token(Parser *parser, TokenType type) {
  assert(parser != NULL);
  if (parser_assert_token_type(parser, type)) {
    parser_advance(parser);
    return 1;
  }

  return 0;
}

int parser_consume_if(Parser *parser, TokenType type) {
  assert(parser != NULL);
  if (parser_get_cur(parser)->type == type) {
    parser_advance(parser);
    return 1;
  }

  return 0;
}

int parser_consume_if_not(Parser *parser, TokenType type) {
  assert(parser != NULL);
  if (parser_get_cur(parser)->type != type) {
    parser_advance(parser);
    return 1;
  }

  return 0;
}

void parser_add_expr(Parser *parser, Operation op, int n, ...) {
  va_list args;
  va_start(args, n);
  list_append(parser->program, new_expr_v(op, n, args));
  va_end(args);
}

Operand *var_decl(Parser *parser) {
  parser_consume_token(parser, INT_TS);
  parser_assert_token_type(parser, IDENTIFIER);
  Token *token = parser_advance(parser);
  if (basic_block_get(parser->bb, token->lexeme) != NULL) {
    char msg[MSG_BUFFER_SIZE];
    snprintf(msg, MSG_BUFFER_SIZE,
             "Variable %s has already been declared in this scope",
             token->lexeme);
    parser_report_error(parser, msg);
    return NULL;
  }

  Operand *var = basic_block_add_var(parser->bb, OT_ID, token->lexeme);

  if (parser_get_cur(parser)->type == SEMICOLON) {
    parser_consume_token(parser, SEMICOLON);
    return var;
  }

  parser_consume_token(parser, EQUAL);

  Operand *rhs = expr(parser);

  parser_consume_token(parser, SEMICOLON);

  parser_add_expr(parser, ASSIGN, 2, var, rhs);

  return var;
}

Operand *expr(Parser *parser) { return var_assignment(parser); }

Operand *var_assignment(Parser *parser) {
  if (parser_peek(parser)->type != EQUAL) {
    return add_sub(parser);
  }

  if (parser_get_cur(parser)->type != IDENTIFIER) {
    parser_report_error(parser, "Expression is not assignable");
  }
  Token *token = parser_advance(parser);
  Operand *lhs = basic_block_get(parser->bb, token->lexeme);
  if (lhs == NULL) {
    char msg[MSG_BUFFER_SIZE];
    snprintf(msg, MSG_BUFFER_SIZE,
             "Variable %s has not been declared in this scope", token->lexeme);
    parser_report_error(parser, msg);
  }

  parser_consume_token(parser, EQUAL);

  Operand *rhs = expr(parser);

  parser_add_expr(parser, ASSIGN, 2, lhs, rhs);

  return lhs;
}

Operand *add_sub(Parser *parser) {
  Operand *lhs = mul_div(parser);

  while (parser_get_cur(parser)->type == PLUS ||
         parser_get_cur(parser)->type == MINUS) {
    TokenType tt = parser_advance(parser)->type;
    Operand *rhs = mul_div(parser);
    Operation op = (tt == PLUS ? ADD : SUB);
    Operand *res = basic_block_add_tmp(parser->bb);
    parser_add_expr(parser, op, 3, res, lhs, rhs);
    lhs = res;
  }

  return lhs;
}

Operand *mul_div(Parser *parser) {
  Operand *lhs = primary_expr(parser);

  while (parser_get_cur(parser)->type == STAR ||
         parser_get_cur(parser)->type == SLASH ||
         parser_get_cur(parser)->type == PERCENT) {
    TokenType tt = parser_advance(parser)->type;
    Operand *rhs = primary_expr(parser);
    Operation op;
    switch (tt) {
    case STAR:
      op = MUL;
      break;
    case SLASH:
      op = DIV;
      break;
    case PERCENT:
      op = MOD;
      break;
    default:
      parser_report_error(parser, "Unexpected token in mul_div");
      return NULL;
    }
    Operand *res = basic_block_add_tmp(parser->bb);
    parser_add_expr(parser, op, 3, res, lhs, rhs);
    lhs = res;
  }

  return lhs;
}

Operand *primary_expr(Parser *parser) {
  Token *token = parser_advance(parser);

  Operand *operand;
  if (token->type == INT_LITERAL) {
    OperandVal val = {.int_val = atoi(token->lexeme)};
    operand = new_operand(val, OT_INT, NULL);
  } else if (token->type == IDENTIFIER) {
    operand = basic_block_get(parser->bb, token->lexeme);
    if (operand == NULL) {
      char msg[MSG_BUFFER_SIZE];
      snprintf(msg, MSG_BUFFER_SIZE,
               "Variable %s has not been declared in this scope",
               token->lexeme);
      parser_report_error(parser, msg);
    }
  } else if (token->type == LEFT_PAREN) {
    operand = expr(parser);
    parser_consume_token(parser, RIGHT_PAREN);
  } else {
    parser_report_error(parser, "Unexpected token while parsing primary_expr");
    return NULL;
  }

  if (parser_consume_if(parser, EQUAL)) {
    if (operand->type != OT_ID) {
      parser_report_error(parser, "Expression is not assignable");
      return operand;
    }

    Operand *rhs = primary_expr(parser);
    parser_add_expr(parser, ASSIGN, 2, operand, rhs);
  }

  return operand;
}

Operand *return_stmt(Parser *parser) {
  parser_consume_token(parser, RETURN);

  Operand *ret_val = expr(parser);

  parser_consume_token(parser, SEMICOLON);

  parser_add_expr(parser, RET, 1, ret_val);

  return NULL;
}

Parser *parser_free(Parser *parser) {
  if (parser == NULL)
    return NULL;

  Node *cur = parser->program->root;
  while (cur != NULL) {
    Expr *expr = cur->data;
    expr_free(expr);
    cur = cur->next;
  }
  list_free(parser->program);

  free(parser);

  return NULL;
}
