#include "hash_map.h"
#include "list.h"
#include <stdlib.h>
#include <string.h>

Entry *new_entry(void *key, void *val) {
  Entry *entry = malloc(sizeof(Entry));

  entry->key = key;
  entry->value = val;

  return entry;
}

HashMap *new_hash_map(unsigned long (*hash_func)(void *),
                      int (*key_compare)(void *, void *)) {
  HashMap *hash_map = malloc(sizeof(HashMap));

  hash_map->N = CO_DEFAULT_HASH_MAP_SIZE;
  hash_map->data = malloc(sizeof(List *) * hash_map->N);
  memset(hash_map->data, 0, sizeof(List *) * hash_map->N);
  hash_map->hash_func = hash_func;
  hash_map->key_compare = key_compare;

  return hash_map;
}

void *hash_map_get(HashMap *hash_map, void *key) {
  int index = hash_map->hash_func(key) % hash_map->N;
  List *row = hash_map->data[index];
  if (row == NULL) {
    return NULL;
  }

  Node *cur = row->root;
  while (cur != NULL) {
    Entry *entry = cur->data;
    if (hash_map->key_compare(entry->key, key) != 0) {
      cur = cur->next;
      continue;
    }
    return entry->value;
  }

  return NULL;
}

void hash_map_insert(HashMap *hash_map, void *key, void *value) {
  int index = hash_map->hash_func(key) % hash_map->N;
  List *row = hash_map->data[index];
  if (row == NULL) {
    row = new_list();
    hash_map->data[index] = row;
  }

  Node *cur = row->root;
  int found = 0;
  while (cur != NULL && !found) {
    Entry *entry = cur->data;
    if (hash_map->key_compare(entry->key, key) != 0) {
      cur = cur->next;
      continue;
    }
    found = 1;
    entry->value = value;
  }

  if (found)
    return;

  Entry *entry = new_entry(key, value);
  list_append(row, entry);
}

unsigned long string_hash(void *val) {
  unsigned char *str = val;
  unsigned long hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

int string_cmp(void *s1, void *s2) { return strcmp(s1, s2); }
