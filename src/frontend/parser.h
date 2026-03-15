#ifndef CO_PARSER_H
#define CO_PARSER_H

#include "frontend/token.h"
#include "ir/basic_block.h"
#include "ir/expr.h"
#include <utils/list.h>

typedef struct Parser {
  List *tokens;
  List *program;
  Node *cur_token;
  BasicBlock *bb;

  int hasError;
} Parser;

Parser *new_parser(List *tokens);

void parse_program(Parser *parser);

int parser_is_at_end(Parser *parser);

Token *parser_advance(Parser *parser);

Token *parser_get_cur(Parser *parser);

void parser_report_error(Parser *parser, char *msg);

void parser_panic(Parser *parser);

int parser_assert_token_type(Parser *parser, TokenType type);

int parser_consume_token(Parser *parser, TokenType type);

int parser_consume_if(Parser *parser, TokenType type);

int parser_consume_if_not(Parser *parser, TokenType type);

void parser_add_expr(Parser *parser, Operation op, int n, ...);

Operand *var_decl(Parser *parser);

Operand *var_assignment(Parser *parser);

Operand *primary_expr(Parser *parser);

Operand *return_expr(Parser *parser);

Parser *parser_free(Parser *parser);

#endif // !CO_PARSER_H
