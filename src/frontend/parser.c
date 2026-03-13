#include "parser.h"
#include "frontend/token.h"
#include "ir/expr.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Parser *new_parser(List *tokens) {
  assert(tokens != NULL);

  Parser *parser = malloc(sizeof(Parser));

  parser->tokens = tokens;

  assert(tokens->root != NULL);
  assert(((Token *)tokens->end->data)->type == EOF_TOKEN);

  parser->cur_token = tokens->root;
  parser->program = new_list();
  parser->hasError = 0;

  parse_program(parser);

  return parser;
}

void parse_program(Parser *parser) {
  parser_consume_token(parser, INT_TS);
  parser_assert_token_type(parser, IDENTIFIER);
  Token *main_token = parser_peek(parser);
  if (strcmp(main_token->lexeme, "main") != 0) {
    parser_report_error(parser, "Expected function to be called main");
  }
  parser_advance(parser);
  parser_consume_token(parser, LEFT_PAREN);
  parser_consume_token(parser, RIGHT_PAREN);
  parser_consume_token(parser, LEFT_BRACE);

  return_expr(parser);

  parser_consume_token(parser, SEMICOLON);
  parser_consume_token(parser, RIGHT_BRACE);
  if (!parser_is_at_end(parser)) {
    parser_report_error(parser, "Expected EOF");
  }
}

int parser_is_at_end(Parser *parser) {
  assert(parser != NULL);
  return parser_peek(parser)->type == EOF_TOKEN;
}

Token *parser_peek(Parser *parser) {
  assert(parser != NULL);
  return parser->cur_token->data;
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
  fprintf(stderr, "[Line %d]: %s\n", parser_peek(parser)->line, msg);
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
  if (parser_peek(parser)->type == type) {
    return 1;
  }
  printf("Expected: %d, Got: %d\n", type, parser_peek(parser)->type);
  parser_report_error(parser, "Unexpected token type");

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
  if (parser_peek(parser)->type == type) {
    parser_advance(parser);
    return 1;
  }

  return 0;
}

int parser_consume_if_not(Parser *parser, TokenType type) {
  assert(parser != NULL);
  if (parser_peek(parser)->type != type) {
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

Operand *primary_expr(Parser *parser) {
  parser_assert_token_type(parser, INT_LITERAL);
  Token *token = parser_peek(parser);
  parser_advance(parser);

  return new_operand(atoi(token->lexeme));
}

Operand *return_expr(Parser *parser) {
  parser_consume_token(parser, RETURN);

  Operand *ret_val = primary_expr(parser);

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
