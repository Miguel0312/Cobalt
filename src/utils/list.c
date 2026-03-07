#include "list.h"

#include <assert.h>
#include <stdlib.h>

Node *create_node(void *val) {
  Node *node = malloc(sizeof(Node));

  node->data = val;
  node->next = node->prev = NULL;

  return node;
}

List *new_list(void) {
  List *list = malloc(sizeof(List));

  list->root = list->end = NULL;

  return list;
}

void list_append(List *list, void *val) {
  assert(list != NULL);
  Node *node = create_node(val);

  node->prev = list->end;
  if (list->end != NULL)
    list->end->next = node;

  if (list->root == NULL)
    list->root = node;
  list->end = node;
}
