#include "token.h"

#include <stdlib.h>

Token *new_token(TokenType type, char *lexeme, int line) {
  Token *token = malloc(sizeof(Token));

  token->type = type;
  token->lexeme = lexeme;
  token->value = NULL;
  token->line = line;

  return token;
}

Token *token_free(Token *token) {
  if (token == NULL)
    return NULL;

  free(token->lexeme);
  free(token);

  return NULL;
}

char *token_type_to_string(TokenType type) {
  switch (type) {
  case LEFT_PAREN:
    return "(";
  case RIGHT_PAREN:
    return ")";
  case LEFT_BRACE:
    return "{";
  case RIGHT_BRACE:
    return "}";
  case LEFT_BRACKET:
    return "[";
  case RIGHT_BRACKET:
    return "]";
  case COMMA:
    return ",";
  case DOT:
    return ".";
  case MINUS:
    return "-";
  case PLUS:
    return "+";
  case SEMICOLON:
    return ";";
  case SLASH:
    return "/";
  case STAR:
    return "*";
  case PERCENT:
    return "%";
  case BANG:
    return "!";
  case BANG_EQUAL:
    return "!=";
  case EQUAL:
    return "=";
  case EQUAL_EQUAL:
    return "==";
  case GREATER:
    return ">";
  case GREATER_EQUAL:
    return ">=";
  case B_OR_TOKEN:
    return "|";
  case B_XOR_TOKEN:
    return "^";
  case B_AND_TOKEN:
    return "&";
  case L_OR_TOKEN:
    return "||";
  case L_AND_TOKEN:
    return "&&";
  case LEFT_SHIFT_TOKEN:
    return "<<";
  case RIGHT_SHIFT_TOKEN:
    return ">>";
  case LESS:
    return "<";
  case LESS_EQUAL:
    return "<=";
  case IDENTIFIER:
    return "ID";
  case STRING_LITERAL:
    return "STRING_LITERAL";
  case INT_LITERAL:
    return "INT_LITERAL";
  case INT_TS:
    return "INT_TS";
  case CHAR_LITERAL:
    return "CHAR_LITERAL";
  case CHAR_TS:
    return "CHAR_TS";
  case IF:
    return "IF";
  case ELSE:
    return "ELSE";
  case RETURN:
    return "RETURN";
  case FOR:
    return "FOR";
  case WHILE:
    return "WHILE";
  case EOF_TOKEN:
    return "EOF";
  }

  return "UNKNOWN_TOKEN_TYPE";
}
