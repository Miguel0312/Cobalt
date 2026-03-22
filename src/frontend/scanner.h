#ifndef CO_SCANNER_H
#define CO_SCANNER_H

#include "token.h"
#include "utils/list.h"

typedef struct Scanner {
  char *src;
  unsigned long src_len;
  List *tokens;

  int start;
  int cur;
  int line;
  int hasError;
} Scanner;

Scanner *new_scanner(char *src, unsigned long src_len);

List *scan_tokens(Scanner *scanner);

int scanner_is_at_end(const Scanner *scanner);

char get_next_char(Scanner *scanner);

void scanner_add_token(Scanner *scanner, TokenType type);

void scanner_advance(Scanner *scanner);

char scanner_peek(Scanner *scanner);

void scanner_report_error(Scanner *scanner, char *msg);

int read_identifier(Scanner *scanner);

int read_number(Scanner *scanner, char *error_msg);

int read_char(Scanner *scanner);

TokenType get_keyword(Scanner *scanner);

Scanner *scanner_free(Scanner *scanner);

#endif // !CO_SCANNER_H
