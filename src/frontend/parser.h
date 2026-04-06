#ifndef CO_PARSER_H
#define CO_PARSER_H

#include "frontend/token.h"
#include "ir/cfg.h"
#include "ir/expr.h"
#include <utils/list.h>

typedef struct Parser {
  List *tokens;
  Node *cur_token;
  CFG *cfg;

  int hasError;
} Parser;

Parser *new_parser(List *tokens);

void parser_free(Parser *parser);

void parse_program(Parser *parser);

int parser_is_at_end(const Parser *parser);

Token *parser_advance(Parser *parser);

Token *parser_get_cur(const Parser *parser);

Token *parser_peek(const Parser *parser);

void parser_report_error(Parser *parser, char *msg);

void parser_panic(Parser *parser);

int parser_assert_token_type(Parser *parser, TokenType type);

int parser_consume_token(Parser *parser, int n, ...);

int parser_consume_if(Parser *parser, TokenType type);

int parser_consume_if_not(Parser *parser, TokenType type);

void parser_add_expr(const Parser *parser, Operation op, int n, ...);

int char_literal_value(Parser *parser, const char *lexeme);

Operand *stmt(Parser *parser, int consume_semicolon);

BasicBlock *block(Parser *parser);

Operand *var_decl(Parser *parser);

BasicBlock *if_stmt(Parser *parser);

void while_stmt(Parser *parser);

void for_stmt(Parser *parser);

Operand *return_stmt(Parser *parser);

Operand *expr(Parser *parser);

Operand *var_assignment(Parser *parser);

Operand *inc_dec(Parser *parser);

Operand *logical_or(Parser *parser);

Operand *logical_and(Parser *parser);

Operand *bitwise_or(Parser *parser);

Operand *bitwise_xor(Parser *parser);

Operand *bitwise_and(Parser *parser);

Operand *cmp(Parser *parser);

Operand *order(Parser *parser);

Operand *shift(Parser *parser);

Operand *add_sub(Parser *parser);

Operand *mul_div(Parser *parser);

Operand *primary_expr(Parser *parser);

#endif // !CO_PARSER_H
