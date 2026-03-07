#include "token.h"

#include <stdlib.h>

Token *new_token(TokenType type, char *lexeme) {
  Token *token = malloc(sizeof(Token));

  token->type = type;
  token->lexeme = lexeme;
  token->value = NULL;

  return token;
}
