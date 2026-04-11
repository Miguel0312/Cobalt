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

#define MSG_BUFFER_SIZE 256

Parser *new_parser(List *tokens) {
  assert(tokens != NULL);

  Parser *parser = malloc(sizeof(Parser));

  parser->tokens = tokens;

  assert(tokens->root != NULL);
  assert(((Token *)tokens->end->data)->type == EOF_TOKEN);

  parser->cur_token = tokens->root;
  parser->hasError = 0;
  parser->break_dst = parser->continue_dst = NULL;
  parser->cfgs = new_list();
  parser->functions = new_hash_map(string_hash, string_cmp);

  parse_program(parser);

  return parser;
}

void parser_free(Parser *parser) {
  if (parser == NULL)
    return;

  Node *cur = parser->cfgs->root;
  while (cur != NULL) {
    cfg_free(cur->data);
    cur = cur->next;
  }

  list_free(parser->cfgs);
  hash_map_free(parser->functions);

  free(parser);
}

void parse_program(Parser *parser) {
  while (!parser_is_at_end(parser)) {
    function(parser);
  }
}

int parser_is_at_end(const Parser *parser) {
  assert(parser != NULL);
  return parser_get_cur(parser)->type == EOF_TOKEN;
}

Token *parser_get_cur(const Parser *parser) {
  assert(parser != NULL);
  return parser->cur_token->data;
}

Token *parser_peek(const Parser *parser) {
  if (parser_is_at_end(parser))
    return parser_get_cur(parser);
  return parser->cur_token->next->data;
}

CFG *get_cur_cfg(Parser *parser) {
  if (parser->cfgs->root == NULL) {
    parser_report_error(parser, "There are no declared CFGs");
    return NULL;
  }

  return parser->cfgs->end->data;
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

int parser_assert_token_type(Parser *parser, const TokenType type) {
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

int parser_consume_token(Parser *parser, const int n, ...) {
  assert(parser != NULL);
  va_list args;
  va_start(args, n);

  const TokenType cur_type = parser_get_cur(parser)->type;
  int offset = 0;
  char token_list_str[MSG_BUFFER_SIZE];

  for (int i = 0; i < n; i++) {
    const TokenType type = va_arg(args, TokenType);
    if (type == cur_type) {
      va_end(args);
      parser_advance(parser);
      return 0;
    }

    if (i == 0) {
      offset += snprintf(token_list_str + offset, MSG_BUFFER_SIZE - offset,
                         "%s", token_type_to_string(type));
    } else {
      offset += snprintf(token_list_str + offset, MSG_BUFFER_SIZE - offset,
                         ", %s", token_type_to_string(type));
    }
  }

  va_end(args);

  char msg[MSG_BUFFER_SIZE];
  snprintf(msg, MSG_BUFFER_SIZE - offset, "Token %s not in %s",
           token_type_to_string(cur_type), token_list_str);

  return 1;
}

int parser_consume_if(Parser *parser, const TokenType type) {
  assert(parser != NULL);
  if (parser_get_cur(parser)->type == type) {
    parser_advance(parser);
    return 1;
  }

  return 0;
}

int parser_consume_if_not(Parser *parser, const TokenType type) {
  assert(parser != NULL);
  if (parser_get_cur(parser)->type != type) {
    parser_advance(parser);
    return 1;
  }

  return 0;
}

void parser_add_expr(Parser *parser, const Operation op, const int n, ...) {
  va_list args;
  va_start(args, n);
  list_append(cfg_get_cur_bb(get_cur_cfg(parser))->expressions,
              new_expr_v(op, n, args));
  va_end(args);
}

void function(Parser *parser) {
  parser_consume_token(parser, 1, INT_TS);
  parser_assert_token_type(parser, IDENTIFIER);
  const char *name = parser_get_cur(parser)->lexeme;
  if (hash_map_get(parser->functions, name) != NULL) {
    char msg[MSG_BUFFER_SIZE];
    snprintf(msg, MSG_BUFFER_SIZE, "Function %s has already been declared", name);
    parser_report_error(parser, msg);
    return;
  }
  
  CFG *cfg = new_cfg(name);
  list_append(parser->cfgs, cfg);
  hash_map_insert(parser->functions, name, cfg);

  parser_advance(parser);
  parser_consume_token(parser, 1, LEFT_PAREN);
  parser_consume_token(parser, 1, RIGHT_PAREN);

  block(parser);
}

Operand *stmt(Parser *parser, int consume_semicolon) {
  switch (parser_get_cur(parser)->type) {
    case INT_TS:
    case CHAR_TS:
      var_decl(parser);
      break;
    case RETURN:
      return_stmt(parser);
      break;
    case IDENTIFIER: {
      Operand *operand = expr(parser);
      if (consume_semicolon) {
        parser_consume_token(parser, 1, SEMICOLON);
      }
      return operand;
    }
    case LEFT_BRACE:
      block(parser);
      break;
    case IF:
      if_stmt(parser);
      break;
    case WHILE:
      while_stmt(parser);
      break;
    case FOR:
      for_stmt(parser);
      break;
    case BREAK:
    case CONTINUE:
      break_cont_stmt(parser);
      break;
    default:
      parser_report_error(parser, "Unexpected token");
  }

  return NULL;
}

BasicBlock *block(Parser *parser) {
  parser_consume_token(parser, 1, LEFT_BRACE);
  BasicBlock *bb = cfg_push_bb(get_cur_cfg(parser));
  cfg_push_symbol_table(get_cur_cfg(parser));

  while (!parser_is_at_end(parser) &&
         parser_get_cur(parser)->type != RIGHT_BRACE) {
    stmt(parser, 1);
  }

  assert(!is_stack_empty(get_cur_cfg(parser)->bb_stack));

  parser_consume_token(parser, 1, RIGHT_BRACE);

  cfg_pop_bb(get_cur_cfg(parser));
  cfg_pop_bb(get_cur_cfg(parser));

  cfg_pop_symbol_table(get_cur_cfg(parser));

  cfg_push_bb(get_cur_cfg(parser));

  return bb;
}

Operand *var_decl(Parser *parser) {
  const DataType data_type = (parser_get_cur(parser)->type == INT_TS ? INT : CHAR);
  parser_consume_token(parser, 2, INT_TS, CHAR_TS);
  parser_assert_token_type(parser, IDENTIFIER);
  const Token *token = parser_advance(parser);
  if (cfg_has_var_in_scope(get_cur_cfg(parser), token->lexeme)) {
    char msg[MSG_BUFFER_SIZE];
    snprintf(msg, MSG_BUFFER_SIZE,
             "Variable %s has already been declared in this scope",
             token->lexeme);
    parser_report_error(parser, msg);
    return NULL;
  }

  Operand *var = cfg_add_var(get_cur_cfg(parser), data_type, OT_ID, token->lexeme);

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

BasicBlock *if_stmt(Parser *parser) {
  parser_consume_token(parser, 1, IF);
  parser_consume_token(parser, 1, LEFT_PAREN);
  Operand *cond = expr(parser);
  parser_consume_token(parser, 1, RIGHT_PAREN);

  BasicBlock *cur_bb = cfg_get_cur_bb(get_cur_cfg(parser));
  parser_add_expr(parser, TEST, 1, cond);

  BasicBlock *true_bb = block(parser);

  BasicBlock *false_bb = cfg_get_cur_bb(get_cur_cfg(parser));

  cur_bb->exit_false = false_bb;

  if (parser_consume_if(parser, ELSE)) {
    if (parser_get_cur(parser)->type == IF) {
      cur_bb->exit_false = if_stmt(parser);
    } else {
      block(parser);
    }
    BasicBlock *finally_block = cfg_get_cur_bb(get_cur_cfg(parser));
    true_bb->exit_true = true_bb->exit_false = finally_block;
  }

  return cur_bb;
}

void while_stmt(Parser *parser) {
  parser_consume_token(parser, 1, WHILE);

  parser_consume_token(parser, 1, LEFT_PAREN);

  BasicBlock *cond_bb = cfg_push_bb(get_cur_cfg(parser));
  Operand *cond = expr(parser);
  parser_add_expr(parser, TEST, 1, cond);

  parser_consume_token(parser, 1, RIGHT_PAREN);

  cfg_pop_bb(get_cur_cfg(parser));

  BasicBlock *cur_break = parser->break_dst, *cur_continue = parser->continue_dst;
  parser->break_dst = cond_bb, parser->continue_dst = cond_bb;

  BasicBlock *loop_bb = block(parser);

  parser->break_dst = cur_break, parser->continue_dst = cur_continue;

  // Bottom basic block of the loop block
  BasicBlock *exit_bb = get_cur_cfg(parser)->bbs->end->prev->data;

  BasicBlock *finally_bb = cfg_get_cur_bb(get_cur_cfg(parser));

  cond_bb->exit_true = loop_bb;
  cond_bb->exit_false = finally_bb;

  exit_bb->exit_true = exit_bb->exit_false = cond_bb;
}

void for_stmt(Parser *parser) {
  parser_consume_token(parser, 1, FOR);

  parser_consume_token(parser, 1, LEFT_PAREN);

  cfg_push_symbol_table(get_cur_cfg(parser));

  cfg_push_bb(get_cur_cfg(parser));

  if (parser_get_cur(parser)->type != SEMICOLON) {
    stmt(parser, 0);
  }
  cfg_pop_bb(get_cur_cfg(parser));
  parser_consume_token(parser, 1, SEMICOLON);

  BasicBlock *cond_bb = cfg_push_bb(get_cur_cfg(parser));
  Operand *cond = NULL;
  if (parser_get_cur(parser)->type != SEMICOLON) {
    cond = stmt(parser, 0);
  }
  if (cond != NULL) {
    parser_add_expr(parser, TEST, 1, cond);
  }
  cfg_pop_bb(get_cur_cfg(parser));
  parser_consume_token(parser, 1, SEMICOLON);

  BasicBlock *inc_bb = cfg_push_bb(get_cur_cfg(parser));
  if (parser_get_cur(parser)->type != RIGHT_PAREN) {
    stmt(parser, 0);
  }
  cfg_pop_bb(get_cur_cfg(parser));

  parser_consume_token(parser, 1, RIGHT_PAREN);

  BasicBlock *cur_break = parser->break_dst, *cur_continue = parser->continue_dst;
  parser->break_dst = cond_bb, parser->continue_dst = inc_bb;

  BasicBlock *loop_bb = block(parser);

  parser->break_dst = cur_break, parser->continue_dst = cur_continue;
  // Bottom basic block of the loop block
  BasicBlock *exit_bb = get_cur_cfg(parser)->bbs->end->prev->data;

  BasicBlock *finally_bb = cfg_get_cur_bb(get_cur_cfg(parser));

  cond_bb->exit_true = loop_bb;
  cond_bb->exit_false = finally_bb;

  exit_bb->exit_true = exit_bb->exit_false = inc_bb;
  inc_bb->exit_true = inc_bb->exit_false = cond_bb;

  cfg_pop_symbol_table(get_cur_cfg(parser));
}

void break_cont_stmt(Parser *parser) {
  const TokenType tt = parser_get_cur(parser)->type;

  parser_consume_token(parser, 2, BREAK, CONTINUE);

  if (parser->break_dst == NULL) {
    parser_report_error(parser, "break and continue can only be used inside for or while");
  }

  Operation op;
  OperandVal val;
  if (tt == BREAK) {
    op = JMP_FALSE;
    val.bb = parser->break_dst;
  } else {
    op = JMP;
    val.bb = parser->continue_dst;
  }


  Operand *operand = new_operand(val, BB, OT_BB, NULL);

  parser_add_expr(parser, op, 1, operand);
  parser_consume_token(parser, 1, SEMICOLON);
}

Operand *expr(Parser *parser) { return var_assignment(parser); }

Operand *var_assignment(Parser *parser) {
  const TokenType tt = parser_peek(parser)->type;
  if (tt != EQUAL && tt != PLUS_EQUAL && tt != MINUS_EQUAL && tt != STAR_EQUAL && tt != SLASH_EQUAL && tt !=
      PERCENT_EQUAL) {
    return logical_or(parser);
  }

  if (parser_get_cur(parser)->type != IDENTIFIER) {
    parser_report_error(parser, "Expression is not assignable");
  }
  const Token *token = parser_advance(parser);
  Operand *lhs = cfg_get_var(get_cur_cfg(parser), token->lexeme);
  if (lhs == NULL) {
    char msg[MSG_BUFFER_SIZE];
    snprintf(msg, MSG_BUFFER_SIZE,
             "Variable %s has not been declared in this scope", token->lexeme);
    parser_report_error(parser, msg);
    return NULL;
  }

  parser_consume_token(parser, 6, EQUAL, PLUS_EQUAL, MINUS_EQUAL, STAR_EQUAL, SLASH_EQUAL, PERCENT_EQUAL);

  Operand *rhs = expr(parser);

  switch (tt) {
    case PLUS_EQUAL: {
      Operand *tmp = cfg_add_tmp(get_cur_cfg(parser), lhs->data_type);
      parser_add_expr(parser, ADD, 3, tmp, lhs, rhs);
      parser_add_expr(parser, ASSIGN, 2, lhs, tmp);
      break;
    }
    case MINUS_EQUAL: {
      Operand *tmp = cfg_add_tmp(get_cur_cfg(parser), lhs->data_type);
      parser_add_expr(parser, SUB, 3, tmp, lhs, rhs);
      parser_add_expr(parser, ASSIGN, 2, lhs, tmp);
      break;
    }
    case STAR_EQUAL: {
      Operand *tmp = cfg_add_tmp(get_cur_cfg(parser), lhs->data_type);
      parser_add_expr(parser, MUL, 3, tmp, lhs, rhs);
      parser_add_expr(parser, ASSIGN, 2, lhs, tmp);
      break;
    }
    case SLASH_EQUAL: {
      Operand *tmp = cfg_add_tmp(get_cur_cfg(parser), lhs->data_type);
      parser_add_expr(parser, DIV, 3, tmp, lhs, rhs);
      parser_add_expr(parser, ASSIGN, 2, lhs, tmp);
      break;
    }
    case PERCENT_EQUAL: {
      Operand *tmp = cfg_add_tmp(get_cur_cfg(parser), lhs->data_type);
      parser_add_expr(parser, MOD, 3, tmp, lhs, rhs);
      parser_add_expr(parser, ASSIGN, 2, lhs, tmp);
      break;
    }
    case EQUAL: {
      parser_add_expr(parser, ASSIGN, 2, lhs, rhs);
      break;
    }
    default:
      assert(0 && "Unreachable");
  }

  return lhs;
}

Operand *logical_or(Parser *parser) {
  Operand *lhs = logical_and(parser);
  if (lhs == NULL)
    return NULL;

  while (parser_get_cur(parser)->type == L_OR_TOKEN) {
    parser_advance(parser);
    BasicBlock *cur_bb = cfg_get_cur_bb(get_cur_cfg(parser));
    parser_add_expr(parser, TEST, 1, lhs);

    BasicBlock *false_bb = cfg_push_bb(get_cur_cfg(parser));

    Operand *rhs = logical_and(parser);

    cur_bb->exit_false = false_bb;

    parser_add_expr(parser, TEST, 1, rhs);
    cfg_pop_bb(get_cur_cfg(parser));
    BasicBlock *one_bb = cfg_push_bb(get_cur_cfg(parser));

    cur_bb->exit_true = false_bb->exit_true = one_bb;

    const DataType data_type =
        get_data_type_from_operands(lhs->data_type, rhs->data_type);
    Operand *res = cfg_add_tmp(get_cur_cfg(parser), data_type);

    const OperandVal one_val = {.int_val = 1};
    Operand *one = new_operand(one_val, data_type, OT_INT, NULL);
    list_append(get_cur_cfg(parser)->operands, one);
    parser_add_expr(parser, ASSIGN, 2, res, one);

    cfg_pop_bb(get_cur_cfg(parser));

    BasicBlock *zero_bb = cfg_push_bb(get_cur_cfg(parser));
    false_bb->exit_false = zero_bb;
    const OperandVal zero_val = {.int_val = 0};
    Operand *zero = new_operand(zero_val, data_type, OT_INT, NULL);
    list_append(get_cur_cfg(parser)->operands, zero);
    parser_add_expr(parser, ASSIGN, 2, res, zero);
    cfg_pop_bb(get_cur_cfg(parser));

    BasicBlock *finally_block = cfg_push_bb(get_cur_cfg(parser));
    one_bb->exit_false = one_bb->exit_true = finally_block;

    lhs = res;
  }

  return lhs;
}

Operand *logical_and(Parser *parser) {
  Operand *lhs = bitwise_or(parser);
  if (lhs == NULL)
    return NULL;

  while (parser_get_cur(parser)->type == L_AND_TOKEN) {
    parser_advance(parser);
    BasicBlock *cur_bb = cfg_get_cur_bb(get_cur_cfg(parser));
    parser_add_expr(parser, TEST, 1, lhs);

    BasicBlock *true_bb = cfg_push_bb(get_cur_cfg(parser));

    Operand *rhs = bitwise_or(parser);

    cur_bb->exit_true = true_bb;

    parser_add_expr(parser, TEST, 1, rhs);
    cfg_pop_bb(get_cur_cfg(parser));
    BasicBlock *zero_bb = cfg_push_bb(get_cur_cfg(parser));

    cur_bb->exit_false = true_bb->exit_false = zero_bb;

    const DataType data_type =
        get_data_type_from_operands(lhs->data_type, rhs->data_type);
    Operand *res = cfg_add_tmp(get_cur_cfg(parser), data_type);

    const OperandVal zero_val = {.int_val = 0};
    Operand *zero = new_operand(zero_val, data_type, OT_INT, NULL);
    list_append(get_cur_cfg(parser)->operands, zero);
    parser_add_expr(parser, ASSIGN, 2, res, zero);

    cfg_pop_bb(get_cur_cfg(parser));

    BasicBlock *one_bb = cfg_push_bb(get_cur_cfg(parser));
    true_bb->exit_true = one_bb;

    const OperandVal one_val = {.int_val = 1};
    Operand *one = new_operand(one_val, data_type, OT_INT, NULL);
    list_append(get_cur_cfg(parser)->operands, one);
    parser_add_expr(parser, ASSIGN, 2, res, one);

    cfg_pop_bb(get_cur_cfg(parser));

    BasicBlock *finally_block = cfg_push_bb(get_cur_cfg(parser));
    zero_bb->exit_false = zero_bb->exit_true = finally_block;

    lhs = res;
  }

  return lhs;
}

Operand *bitwise_or(Parser *parser) {
  Operand *lhs = bitwise_xor(parser);
  if (lhs == NULL)
    return NULL;

  while (parser_get_cur(parser)->type == B_OR_TOKEN) {
    parser_advance(parser);
    Operand *rhs = bitwise_xor(parser);

    const DataType data_type =
        get_data_type_from_operands(lhs->data_type, rhs->data_type);

    Operand *res = cfg_add_tmp(get_cur_cfg(parser), data_type);
    parser_add_expr(parser, B_OR, 3, res, lhs, rhs);
    lhs = res;
  }

  return lhs;
}

Operand *bitwise_xor(Parser *parser) {
  Operand *lhs = bitwise_and(parser);
  if (lhs == NULL)
    return NULL;

  while (parser_get_cur(parser)->type == B_XOR_TOKEN) {
    parser_advance(parser);
    Operand *rhs = bitwise_and(parser);

    const DataType data_type =
        get_data_type_from_operands(lhs->data_type, rhs->data_type);

    Operand *res = cfg_add_tmp(get_cur_cfg(parser), data_type);
    parser_add_expr(parser, B_XOR, 3, res, lhs, rhs);
    lhs = res;
  }

  return lhs;
}

Operand *bitwise_and(Parser *parser) {
  Operand *lhs = cmp(parser);
  if (lhs == NULL)
    return NULL;

  while (parser_get_cur(parser)->type == B_AND_TOKEN) {
    parser_advance(parser);
    Operand *rhs = cmp(parser);

    const DataType data_type =
        get_data_type_from_operands(lhs->data_type, rhs->data_type);

    Operand *res = cfg_add_tmp(get_cur_cfg(parser), data_type);

    parser_add_expr(parser, B_AND, 3, res, lhs, rhs);
    lhs = res;
  }

  return lhs;
}

Operand *cmp(Parser *parser) {
  Operand *lhs = order(parser);
  if (lhs == NULL)
    return NULL;

  while (parser_get_cur(parser)->type == EQUAL_EQUAL ||
         parser_get_cur(parser)->type == BANG_EQUAL) {
    const TokenType tt = parser_advance(parser)->type;
    Operand *rhs = order(parser);
    const Operation op = (tt == EQUAL_EQUAL ? IS_EQUAL : IS_DIF);

    const DataType data_type =
        get_data_type_from_operands(lhs->data_type, rhs->data_type);

    Operand *res = cfg_add_tmp(get_cur_cfg(parser), data_type);
    parser_add_expr(parser, op, 3, res, lhs, rhs);
    lhs = res;
  }

  return lhs;
}

Operand *order(Parser *parser) {
  Operand *lhs = shift(parser);
  if (lhs == NULL)
    return NULL;

  TokenType tt = parser_get_cur(parser)->type;
  while (tt == LESS || tt == LESS_EQUAL || tt == GREATER ||
         tt == GREATER_EQUAL) {
    parser_advance(parser);
    Operand *rhs = shift(parser);
    Operation op;
    switch (tt) {
      case LESS:
        op = IS_LESS;
        break;
      case LESS_EQUAL:
        op = IS_LESS_EQUAL;
        break;
      case GREATER:
        op = IS_GREATER;
        break;
      case GREATER_EQUAL:
        op = IS_GREATER_EQUAL;
        break;
      default:
        assert(0);
    }

    const DataType data_type =
        get_data_type_from_operands(lhs->data_type, rhs->data_type);

    Operand *res = cfg_add_tmp(get_cur_cfg(parser), data_type);
    parser_add_expr(parser, op, 3, res, lhs, rhs);
    lhs = res;
    tt = parser_get_cur(parser)->type;
  }

  return lhs;
}

Operand *shift(Parser *parser) {
  Operand *lhs = add_sub(parser);
  if (lhs == NULL) {
    return NULL;
  }

  while (parser_get_cur(parser)->type == LEFT_SHIFT_TOKEN ||
         parser_get_cur(parser)->type == RIGHT_SHIFT_TOKEN) {
    const TokenType tt = parser_advance(parser)->type;
    const Operand *rhs = add_sub(parser);
    const Operation op = tt == LEFT_SHIFT_TOKEN ? LEFT_SHIFT : RIGHT_SHIFT;

    const DataType data_type =
        get_data_type_from_operands(lhs->data_type, rhs->data_type);

    Operand *res = cfg_add_tmp(get_cur_cfg(parser), data_type);
    parser_add_expr(parser, op, 3, res, lhs, rhs);
    lhs = res;
  }

  return lhs;
}

Operand *add_sub(Parser *parser) {
  Operand *lhs = mul_div(parser);
  if (lhs == NULL)
    return NULL;

  while (parser_get_cur(parser)->type == PLUS ||
         parser_get_cur(parser)->type == MINUS) {
    const TokenType tt = parser_advance(parser)->type;
    const Operand *rhs = mul_div(parser);
    const Operation op = (tt == PLUS ? ADD : SUB);

    const DataType data_type =
        get_data_type_from_operands(lhs->data_type, rhs->data_type);

    Operand *res = cfg_add_tmp(get_cur_cfg(parser), data_type);
    parser_add_expr(parser, op, 3, res, lhs, rhs);
    lhs = res;
  }

  return lhs;
}

Operand *mul_div(Parser *parser) {
  Operand *lhs = inc_dec(parser);
  if (lhs == NULL)
    return NULL;

  while (parser_get_cur(parser)->type == STAR ||
         parser_get_cur(parser)->type == SLASH ||
         parser_get_cur(parser)->type == PERCENT) {
    const TokenType tt = parser_advance(parser)->type;
    Operand *rhs = inc_dec(parser);
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

    const DataType data_type =
        get_data_type_from_operands(lhs->data_type, rhs->data_type);

    Operand *res = cfg_add_tmp(get_cur_cfg(parser), data_type);
    parser_add_expr(parser, op, 3, res, lhs, rhs);
    lhs = res;
  }

  return lhs;
}

Operand *inc_dec(Parser *parser) {
  TokenType tt = parser_get_cur(parser)->type;
  if (tt == INCREMENT || tt == DECREMENT) {
    parser_advance(parser);
    Operand *operand = primary_expr(parser);
    if (operand->op_type != OT_ID) {
      parser_report_error(parser, "Expression is not assignable");
      return NULL;
    }

    const Operation op = (tt == INCREMENT ? INC : DEC);

    parser_add_expr(parser, op, 1, operand);

    return operand;
  }

  if (tt == BANG) {
    parser_advance(parser);
    Operand *operand = primary_expr(parser);

    parser_add_expr(parser, TEST, 1, operand);

    Operand *result = cfg_add_tmp(get_cur_cfg(parser), CHAR);

    parser_add_expr(parser, L_NOT, 2, result, operand);

    return result;
  }

  tt = parser_peek(parser)->type;

  if (tt == INCREMENT || tt == DECREMENT) {
    Operand *operand = primary_expr(parser);
    assert(operand != NULL);
    if (operand->op_type != OT_ID) {
      parser_report_error(parser, "Expression is not assignable");
      return NULL;
    }
    Operand *old_res = cfg_add_tmp(get_cur_cfg(parser), operand->data_type);

    parser_add_expr(parser, ASSIGN, 2, old_res, operand);

    const Operation op = (tt == INCREMENT ? INC : DEC);

    parser_add_expr(parser, op, 1, operand);

    parser_advance(parser);

    return old_res;
  }

  return primary_expr(parser);
}

Operand *primary_expr(Parser *parser) {
  const Token *token = parser_advance(parser);

  Operand *operand;
  if (token->type == INT_LITERAL) {
    const OperandVal val = {.int_val = (int) strtol(token->lexeme, NULL, 10)};
    operand = new_operand(val, INT, OT_INT, NULL);
    list_append(get_cur_cfg(parser)->operands, operand);
  } else if (token->type == CHAR_LITERAL) {
    const OperandVal val = {.int_val = char_literal_value(parser, token->lexeme)};
    operand = new_operand(val, CHAR, OT_CHAR, NULL);
    list_append(get_cur_cfg(parser)->operands, operand);
  } else if (token->type == IDENTIFIER) {
    operand = cfg_get_var(get_cur_cfg(parser), token->lexeme);
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
  } else if (token->type == PLUS) {
    return expr(parser);
  } else if (token->type == MINUS) {
    Operand *rhs = primary_expr(parser);
    Operand *res = cfg_add_tmp(get_cur_cfg(parser), rhs->data_type);
    parser_add_expr(parser, NEG, 2, res, rhs);
    return res;
  } else {
    char msg[MSG_BUFFER_SIZE];
    snprintf(msg, MSG_BUFFER_SIZE, "Unexpected token while parsing primary_expr, got %s",
             token_type_to_string(parser_get_cur(parser)->type));
    parser_report_error(parser, msg);
    return NULL;
  }

  if (parser_consume_if(parser, EQUAL)) {
    if (operand != NULL && operand->op_type != OT_ID) {
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

int char_literal_value(Parser *parser, const char *lexeme) {
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
    default:
      break;
  }

  parser_report_error(parser, "Char literal has invalid escape sequence");
  return 0;
}
