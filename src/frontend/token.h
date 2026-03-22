#ifndef CO_TOKEN_H
#define CO_TOKEN_H

typedef enum TokenType {
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  LEFT_BRACKET,
  RIGHT_BRACKET,
  COMMA,
  DOT,
  MINUS,
  PLUS,
  SEMICOLON,
  SLASH,
  STAR,
  PERCENT,
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  B_OR_TOKEN,
  B_XOR_TOKEN,
  B_AND_TOKEN,
  L_OR_TOKEN,
  L_AND_TOKEN,
  LEFT_SHIFT_TOKEN,
  RIGHT_SHIFT_TOKEN,
  LESS,
  LESS_EQUAL,
  IDENTIFIER,
  STRING_LITERAL,
  CHAR_LITERAL,
  INT_LITERAL,
  INT_TS, // int type specifier
  CHAR_TS,
  IF,
  ELSE,
  RETURN,
  FOR,
  WHILE,
  EOF_TOKEN
} TokenType;

typedef struct {
  TokenType type;
  char *lexeme;
  void *value;
  int line;
} Token;

Token *new_token(TokenType type, char *lexeme, int line);

Token *token_free(Token *token);

char *token_type_to_string(TokenType type);

#endif // !TOKEN_H
