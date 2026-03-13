#include "backend/code_gen.h"
#include "frontend/parser.h"
#include "frontend/scanner.h"
#include "ir/expr.h"
#include "utils/list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CALL_ERROR 1
#define FILE_NOT_FOUND_ERROR 2
#define SCANNING_ERROR 3
#define PARSING_ERROR 4
#define CODE_GEN_ERROR 5

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "%d\n", argc);
    fprintf(stderr, "Usage: cobalt <file_name> [output_file]");
    return CALL_ERROR;
  }

  FILE *f = fopen(argv[1], "r");

  if (f == NULL) {
    fprintf(stderr, "File %s not found", argv[1]);
    return FILE_NOT_FOUND_ERROR;
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
    scanner_free(scanner);
    fprintf(stderr, "Error while scanning. Exiting.");
    return SCANNING_ERROR;
  }

  Parser *parser = new_parser(scanner->tokens);

  assert(parser != NULL);
  assert(parser->program != NULL);

  if (parser->hasError) {
    parser_free(parser);
    scanner_free(scanner);
    fprintf(stderr, "Error while parsing. Exiting.");
    return PARSING_ERROR;
  }

  Node *cur = parser->program->root;
  while (cur != NULL) {
    Expr *expr = cur->data;

    print_expr(expr);

    cur = cur->next;
  }

  char *assembly_file;
  int assembly_file_in_heap = 0;
  if (argc < 3) {
    int filename_size = strlen(argv[1]);
    assembly_file = malloc(filename_size + 1);
    assembly_file_in_heap = 1;
    strcpy(assembly_file, argv[1]);
    assembly_file[filename_size - 1] = 's';
  } else {
    assembly_file = argv[2];
  }

  f = fopen(assembly_file, "w+");
  if (assembly_file_in_heap) {
    free(assembly_file);
  }
  CodeGenerator *code_gen = new_code_generator(parser->program, f);
  fclose(f);

  int hasError = code_gen->hasError;

  code_gen_free(code_gen);
  parser_free(parser);
  scanner_free(scanner);

  if (hasError) {
    return CODE_GEN_ERROR;
  }

  return 0;
}
