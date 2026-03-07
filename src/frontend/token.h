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
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,
  IDENTIFIER,
  STRING_LITERAL,
  INT_LITERAL,
  INT_TS, // int type specifier
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
} Token;

Token *new_token(TokenType type, char *lexeme);

#endif // !TOKEN_H
