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
