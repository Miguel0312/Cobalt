#include "frontend/parser.h"
#include "frontend/scanner.h"
#include "ir/expr.h"
#include "utils/list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: cobalt <file_name>");
    return 1;
  }

  FILE *f = fopen(argv[1], "r");

  if (f == NULL) {
    fprintf(stderr, "File %s not found", argv[1]);
    return 1;
  }

  fseek(f, 0, SEEK_END);
  long length = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *buffer = malloc(length + 1);
  buffer[length] = '\0';

  fread(buffer, 1, length, f);
  fclose(f);

  Scanner *scanner = new_scanner(buffer, length);

  assert(scanner != NULL);
  assert(scanner->tokens != NULL);

  if (scanner->hasError) {
    fprintf(stderr, "Error while scanning. Exiting.");
    return 1;
  }

  Parser *parser = new_parser(scanner->tokens);

  assert(parser != NULL);
  assert(parser->program != NULL);

  if (parser->hasError) {
    fprintf(stderr, "Error while parsing. Exiting.");
    return 1;
  }

  Node *cur = parser->program->root;
  while (cur != NULL) {
    Expr *expr = cur->data;

    print_expr(expr);

    cur = cur->next;
  }

  return 0;
}
