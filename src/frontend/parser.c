#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "frontend/token.h"
#include "ir/basic_block.h"
#include "ir/cfg.h"
#include "ir/expr.h"
#include "parser.h"
#include "utils/hash_map.h"

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
  parser->cfg = new_cfg();

  parse_program(parser);

  return parser;
}

void parse_program(Parser *parser) {
  parser_consume_token(parser, 1, INT_TS);
  parser_assert_token_type(parser, IDENTIFIER);
  Token *main_token = parser_get_cur(parser);
  if (strcmp(main_token->lexeme, "main") != 0) {
    parser_report_error(parser, "Expected function to be called main");
  }
  parser_advance(parser);
  parser_consume_token(parser, 1, LEFT_PAREN);
  parser_consume_token(parser, 1, RIGHT_PAREN);

  block(parser);

  if (!parser_is_at_end(parser)) {
    parser_report_error(parser, "Expected EOF");
  }
  // Make sure that all basic blocks stack memory is freed
  assert(parser->cfg->offset == 0);
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
  snprintf(msg, MSG_BUFFER_SIZE, "Unexpected token type. Expected: %s, Got: %s",
           token_type_to_string(type),
           token_type_to_string(parser_get_cur(parser)->type));
  parser_report_error(parser, msg);

  return 0;
}

int parser_consume_token(Parser *parser, int n, ...) {
  assert(parser != NULL);
  va_list args;
  va_start(args, n);

  TokenType cur_type = parser_peek(parser)->type;
  int offset = 0;
  char token_list_str[MSG_BUFFER_SIZE];

  for (int i = 0; i < n; i++) {
    TokenType type = va_arg(args, TokenType);
    if (type == cur_type) {
      va_end(args);
      parser_advance(parser);
      return 0;
    } else {
      if (i == 0) {
        offset += snprintf(token_list_str + offset, MSG_BUFFER_SIZE - offset,
                           "%s", token_type_to_string(type));
      } else {
        offset += snprintf(token_list_str + offset, MSG_BUFFER_SIZE - offset,
                           ", %s", token_type_to_string(type));
      }
    }
  }

  va_end(args);

  char msg[MSG_BUFFER_SIZE];
  snprintf(msg, MSG_BUFFER_SIZE - offset, "Token %s not in %s",
           token_type_to_string(cur_type), token_list_str);

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
void block(Parser *parser) {
  parser_consume_token(parser, 1, LEFT_BRACE);

  cfg_push_bb(parser->cfg);

  TokenType cur_type;
  while (!parser_is_at_end(parser) &&
         (cur_type = parser_get_cur(parser)->type) != RIGHT_BRACE) {
    switch (cur_type) {
    case INT_TS:
    case CHAR_TS:
      var_decl(parser);
      break;
    case RETURN:
      return_stmt(parser);
      break;
    case IDENTIFIER:
      expr(parser);
      parser_consume_token(parser, 1, SEMICOLON);
      break;
    case LEFT_BRACE:
      block(parser);
      break;
    default:
      parser_report_error(parser, "Unexpected token");
    }
  }

  assert(!is_stack_empty(parser->cfg->bb_stack));
  cfg_pop_bb(parser->cfg);

  parser_consume_token(parser, 1, RIGHT_BRACE);
}
Operand *var_decl(Parser *parser) {
  DataType data_type = (parser_peek(parser)->type == INT_TS ? INT : CHAR);
  parser_consume_token(parser, 2, INT_TS, CHAR_TS);
  parser_assert_token_type(parser, IDENTIFIER);
  Token *token = parser_advance(parser);
  if (hash_map_get(cfg_get_cur_bb(parser->cfg)->operands, token->lexeme) !=
      NULL) {
    char msg[MSG_BUFFER_SIZE];
    snprintf(msg, MSG_BUFFER_SIZE,
             "Variable %s has already been declared in this scope",
             token->lexeme);
    parser_report_error(parser, msg);
    return NULL;
  }

  Operand *var = cfg_add_var(parser->cfg, data_type, OT_ID, token->lexeme);

  if (parser_get_cur(parser)->type == SEMICOLON) {
    parser_consume_token(parser, 1, SEMICOLON);
    return var;
  }

  parser_consume_token(parser, 1, EQUAL);

  Operand *rhs = expr(parser);

  parser_consume_token(parser, 1, SEMICOLON);

  parser_add_expr(parser, ASSIGN, 2, var, rhs);

  return var;
}

Operand *expr(Parser *parser) { return var_assignment(parser); }

Operand *var_assignment(Parser *parser) {
  if (parser_peek(parser)->type != EQUAL) {
    return bitwise_or(parser);
  }

  if (parser_get_cur(parser)->type != IDENTIFIER) {
    parser_report_error(parser, "Expression is not assignable");
  }
  Token *token = parser_advance(parser);
  Operand *lhs = cfg_get_var(parser->cfg, token->lexeme);
  if (lhs == NULL) {
    char msg[MSG_BUFFER_SIZE];
    snprintf(msg, MSG_BUFFER_SIZE,
             "Variable %s has not been declared in this scope", token->lexeme);
    parser_report_error(parser, msg);
  }

  parser_consume_token(parser, 1, EQUAL);

  Operand *rhs = expr(parser);

  parser_add_expr(parser, ASSIGN, 2, lhs, rhs);

  return lhs;
}

Operand *bitwise_or(Parser *parser) {
  Operand *lhs = bitwise_xor(parser);

  while (parser_get_cur(parser)->type == B_OR_TOKEN) {
    parser_advance(parser);
    Operand *rhs = bitwise_xor(parser);
    Operand *res = cfg_add_tmp(parser->cfg);
    parser_add_expr(parser, B_OR, 3, res, lhs, rhs);
    lhs = res;
  }

  return lhs;
}

Operand *bitwise_xor(Parser *parser) {
  Operand *lhs = bitwise_and(parser);

  while (parser_get_cur(parser)->type == B_XOR_TOKEN) {
    parser_advance(parser);
    Operand *rhs = bitwise_and(parser);
    Operand *res = cfg_add_tmp(parser->cfg);
    parser_add_expr(parser, B_XOR, 3, res, lhs, rhs);
    lhs = res;
  }

  return lhs;
}

Operand *bitwise_and(Parser *parser) {
  Operand *lhs = shift(parser);

  while (parser_get_cur(parser)->type == B_AND_TOKEN) {
    parser_advance(parser);
    Operand *rhs = shift(parser);
    Operand *res = cfg_add_tmp(parser->cfg);
    parser_add_expr(parser, B_AND, 3, res, lhs, rhs);
    lhs = res;
  }

  return lhs;
}

Operand *shift(Parser *parser) {
  Operand *lhs = add_sub(parser);

  while (parser_get_cur(parser)->type == LEFT_SHIFT_TOKEN ||
         parser_get_cur(parser)->type == RIGHT_SHIFT_TOKEN) {
    TokenType tt = parser_advance(parser)->type;
    Operand *rhs = add_sub(parser);
    Operation op = (tt == LEFT_SHIFT_TOKEN ? LEFT_SHIFT : RIGHT_SHIFT);
    Operand *res = cfg_add_tmp(parser->cfg);
    parser_add_expr(parser, op, 3, res, lhs, rhs);
    lhs = res;
  }

  return lhs;
}

Operand *add_sub(Parser *parser) {
  Operand *lhs = mul_div(parser);

  while (parser_get_cur(parser)->type == PLUS ||
         parser_get_cur(parser)->type == MINUS) {
    TokenType tt = parser_advance(parser)->type;
    Operand *rhs = mul_div(parser);
    Operation op = (tt == PLUS ? ADD : SUB);
    Operand *res = cfg_add_tmp(parser->cfg);
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
    Operand *res = cfg_add_tmp(parser->cfg);
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
    operand = new_operand(val, INT, OT_INT, NULL);
  } else if (token->type == CHAR_LITERAL) {
    OperandVal val = {.int_val = char_literal_value(parser, token->lexeme)};
    operand = new_operand(val, CHAR, OT_CHAR, NULL);
  } else if (token->type == IDENTIFIER) {
    operand = basic_block_get(cfg_get_cur_bb(parser->cfg), token->lexeme);
    if (operand == NULL) {
      char msg[MSG_BUFFER_SIZE];
      snprintf(msg, MSG_BUFFER_SIZE,
               "Variable %s has not been declared in this scope",
               token->lexeme);
      parser_report_error(parser, msg);
    }
  } else if (token->type == LEFT_PAREN) {
    operand = expr(parser);
    parser_consume_token(parser, 1, RIGHT_PAREN);
  } else {
    parser_report_error(parser, "Unexpected token while parsing primary_expr");
    return NULL;
  }

  if (parser_consume_if(parser, EQUAL)) {
    if (operand->op_type != OT_ID) {
      parser_report_error(parser, "Expression is not assignable");
      return operand;
    }

    Operand *rhs = primary_expr(parser);
    parser_add_expr(parser, ASSIGN, 2, operand, rhs);
  }

  return operand;
}

Operand *return_stmt(Parser *parser) {
  parser_consume_token(parser, 1, RETURN);

  Operand *ret_val = expr(parser);

  parser_consume_token(parser, 1, SEMICOLON);

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

int char_literal_value(Parser *parser, char *lexeme) {
  if (lexeme[1] != '\\') {
    return lexeme[1];
  }

  switch (lexeme[2]) {
  case '0':
    return 0;
  case 'a':
    return 7;
  case 'b':
    return 8;
  case 't':
    return 9;
  case 'n':
    return 10;
  case 'v':
    return 11;
  case 'f':
    return 12;
  case 'r':
    return 13;
  // Single Quote and anti slash
  case 39:
  case 92:
    return lexeme[2];
  }

  parser_report_error(parser, "Char literal has invalid escape sequence");
  return 0;
}
