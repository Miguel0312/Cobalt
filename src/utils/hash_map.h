#ifndef CO_HASH_MAP_H
#define CO_HASH_MAP_H

#include "utils/list.h"
typedef struct Entry {
  void *key;
  void *value;
} Entry;

#define CO_DEFAULT_HASH_MAP_SIZE 128

typedef struct HashMap {
  unsigned long (*hash_func)(void *);

  int (*key_compare)(void *, void *);

  int N;
  List **data;
} HashMap;

Entry *new_entry(void *key, void *val);

HashMap *new_hash_map(unsigned long (*hash_func)(void *),
                      int (*key_compare)(void *, void *));

void *hash_map_get(HashMap *hash_map, void *key);

void hash_map_insert(HashMap *hash_map, void *key, void *value);

unsigned long string_hash(void *s);

int string_cmp(void *s1, void *s2);

#endif // !CO_HASH_MAP_H
