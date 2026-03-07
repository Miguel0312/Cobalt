#include "frontend/scanner.h"
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
  if (scanner->hasError) {
    fprintf(stderr, "Error while scanning. Exiting.");
    return 1;
  }

  assert(scanner != NULL);
  assert(scanner->tokens != NULL);
  assert(scanner->tokens->root != NULL);
  Node *cur = scanner->tokens->root;
  while (cur != NULL) {
    Token *token = (Token *)cur->data;
    if (token->type == EOF_TOKEN)
      break;
    if (token->lexeme == NULL) {
      continue;
    }
    printf("%s %d\n", token->lexeme, token->type);
    cur = cur->next;
  }

  return 0;
}
