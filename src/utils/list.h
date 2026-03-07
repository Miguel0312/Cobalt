#ifndef CO_LIST_H
#define CO_LIST_H

typedef struct Node {
  void *data;
  struct Node *next;
  struct Node *prev;
} Node;

typedef struct List {
  Node *root;
  Node *end;
} List;

Node *create_node(void *val);

List *new_list(void);

void list_append(List *list, void *val);

#endif // !CO_LIST_H
