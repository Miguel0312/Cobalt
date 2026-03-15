#include "scanner.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "frontend/token.h"
#include "utils/constants.h"

Scanner *new_scanner(char *src, unsigned long src_len) {
  Scanner *scanner = malloc(sizeof(Scanner));

  scanner->src = src;
  scanner->tokens = new_list();
  scanner->cur = 0;
  scanner->line = 1;
  scanner->start = 0;
  scanner->hasError = 0;

  if (src_len == 0)
    scanner->src_len = strlen(src);
  else
    scanner->src_len = src_len;

  scan_tokens(scanner);
  return scanner;
}

List *scan_tokens(Scanner *scanner) {
  assert(scanner != NULL);
  while (!scanner_is_at_end(scanner)) {
    char c = get_next_char(scanner);
    switch (c) {
    case '(': {
      scanner_add_token(scanner, LEFT_PAREN);
      break;
    }
    case ')': {
      scanner_add_token(scanner, RIGHT_PAREN);
      break;
    }
    case '{': {
      scanner_add_token(scanner, LEFT_BRACE);
      break;
    }
    case '}': {
      scanner_add_token(scanner, RIGHT_BRACE);
      break;
    }
    case '[': {
      scanner_add_token(scanner, LEFT_BRACKET);
      break;
    }
    case ']': {
      scanner_add_token(scanner, RIGHT_BRACKET);
      break;
    }
    case ',': {
      scanner_add_token(scanner, COMMA);
      break;
    }
    case '.': {
      scanner_add_token(scanner, DOT);
      break;
    }
    case '-': {
      scanner_add_token(scanner, MINUS);
      break;
    }
    case '+': {
      scanner_add_token(scanner, PLUS);
      break;
    }
    case ';': {
      scanner_add_token(scanner, SEMICOLON);
      break;
    }
    case '/': {
      if (scanner_peek(scanner) == '/') {
        char comment_char;
        do {
          comment_char = get_next_char(scanner);
        } while (comment_char != '\n' && comment_char != '\0');
      } else if (scanner_peek(scanner) == '*') {
        char comment_char = get_next_char(scanner);
        do {
          comment_char = get_next_char(scanner);
        } while (!scanner_is_at_end(scanner) &&
                 (comment_char != '*' || scanner_peek(scanner) != '/'));
        if (scanner_is_at_end(scanner)) {
          scanner_report_error(scanner, "Multi-line comment must be closed");
        } else {
          scanner_advance(scanner);
        }
      } else {
        scanner_add_token(scanner, SLASH);
      }
      break;
    }
    case '*': {
      scanner_add_token(scanner, STAR);
      break;
    }
    case '!': {
      if (scanner_peek(scanner) == '=') {
        scanner_advance(scanner);
        scanner_add_token(scanner, BANG_EQUAL);
      } else {
        scanner_add_token(scanner, BANG);
      }
      break;
    }
    case '=': {
      if (scanner_peek(scanner) == '=') {
        scanner_advance(scanner);
        scanner_add_token(scanner, EQUAL_EQUAL);
      } else {
        scanner_add_token(scanner, EQUAL);
      }
      break;
    }
    case '>': {
      if (scanner_peek(scanner) == '=') {
        scanner_advance(scanner);
        scanner_add_token(scanner, GREATER_EQUAL);
      } else {
        scanner_add_token(scanner, GREATER);
      }
      break;
    }
    case '<': {
      if (scanner_peek(scanner) == '=') {
        scanner_advance(scanner);
        scanner_add_token(scanner, LESS_EQUAL);
      } else {
        scanner_add_token(scanner, LESS);
      }
      break;
    }
    case ' ':
    case '\t':
    case '\r': {
      break;
    }
    case '\n': {
      scanner->line++;
      continue;
    }
    default: {
      int found = 0;
      char msg[MSG_BUFFER_SIZE];
      if (isalpha(c) || c == '_') {
        found = read_identifier(scanner, msg);
        if (found) {
          TokenType tt = get_keyword(scanner);
          scanner_add_token(scanner, tt);
        }
      } else if (isdigit(c)) {
        found = read_number(scanner, msg);
        if (found) {
          scanner_add_token(scanner, INT_LITERAL);
        }
      } else {
        snprintf(msg, MSG_BUFFER_SIZE, "Unexpected character: %c", c);
      }

      if (found)
        break;

      msg[MSG_BUFFER_SIZE - 1] = '\0';

      scanner_report_error(scanner, msg);
    }
    }
    scanner->start = scanner->cur;
  }

  scanner_add_token(scanner, EOF_TOKEN);

  return scanner->tokens;
}

int scanner_is_at_end(const Scanner *scanner) {
  assert(scanner != NULL);
  return (unsigned long)scanner->cur >= scanner->src_len;
}

char get_next_char(Scanner *scanner) {
  assert(scanner != NULL);
  if (scanner_is_at_end(scanner))
    return '\0';

  char res = scanner->src[scanner->cur];
  scanner_advance(scanner);

  return res;
}

void scanner_add_token(Scanner *scanner, TokenType type) {
  assert(scanner != NULL);
  int lexeme_len = scanner->cur - scanner->start;
  char *lexeme = malloc(lexeme_len + 1);
  lexeme[lexeme_len] = '\0';

  strncpy(lexeme, scanner->src + scanner->start, lexeme_len);

  Token *token = new_token(type, lexeme, scanner->line);

  list_append(scanner->tokens, token);
}

void scanner_advance(Scanner *scanner) {
  assert(scanner != NULL);
  if (scanner_is_at_end(scanner))
    return;

  scanner->cur++;
}

char scanner_peek(Scanner *scanner) {
  assert(scanner != NULL);

  return scanner->src[scanner->cur];
}

void scanner_report_error(Scanner *scanner, char *msg) {
  assert(scanner != NULL);
  fprintf(stderr, "[Line %d]: %s\n", scanner->line, msg);
  scanner->hasError = 1;
}

int read_identifier(Scanner *scanner, char *error_msg) {
  assert(scanner != NULL);
  assert(error_msg != NULL);
  while (isalnum(scanner_peek(scanner)) || scanner_peek(scanner) == '_') {
    scanner_advance(scanner);
  }

  return 1;
}

int read_number(Scanner *scanner, char *error_msg) {
  assert(scanner != NULL);
  assert(error_msg != NULL);
  while (isdigit(scanner_peek(scanner))) {
    scanner_advance(scanner);
  }

  char c = scanner_peek(scanner);
  if (isalpha(c) || c == '_') {
    snprintf(error_msg, MSG_BUFFER_SIZE,
             "Unexpected character while trying to parse number: %c", c);
    return 0;
  }

  return 1;
}

TokenType get_keyword(Scanner *scanner) {
  int lexeme_len = scanner->cur - scanner->start;
  if (lexeme_len >= MSG_BUFFER_SIZE) {
    // Definitely an identifier, no keyword is so big
    return IDENTIFIER;
  }
  char lexeme[MSG_BUFFER_SIZE];
  strncpy(lexeme, scanner->src + scanner->start, lexeme_len);
  lexeme[lexeme_len] = '\0';

  if (strcmp(lexeme, "int") == 0) {
    return INT_TS;
  }
  if (strcmp(lexeme, "if") == 0) {
    return IF;
  }
  if (strcmp(lexeme, "else") == 0) {
    return ELSE;
  }
  if (strcmp(lexeme, "for") == 0) {
    return FOR;
  }
  if (strcmp(lexeme, "while") == 0) {
    return WHILE;
  }
  if (strcmp(lexeme, "return") == 0) {
    return RETURN;
  }

  return IDENTIFIER;
}

Scanner *scanner_free(Scanner *scanner) {
  if (scanner == NULL)
    return NULL;

  free(scanner->src);

  Node *cur = scanner->tokens->root;
  while (cur != NULL) {
    Token *token = cur->data;
    token_free(token);
    cur = cur->next;
  }
  list_free(scanner->tokens);

  free(scanner);

  return NULL;
}
