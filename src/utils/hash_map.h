#ifndef CO_HASH_MAP_H
#define CO_HASH_MAP_H

#include "utils/list.h"

typedef struct Entry {
    void *key;
    void *value;
} Entry;

#define CO_DEFAULT_HASH_MAP_SIZE 128

typedef struct HashMap {
    unsigned long (*hash_func)(const void *);

    int (*key_compare)(const void *, const void *);

    int N;
    List **data;
} HashMap;

HashMap *new_hash_map(unsigned long (*hash_func)(const void *),
                      int (*key_compare)(const void *, const void *));

void hash_map_free(HashMap *hash_map);

Entry *new_entry(void *key, void *val);

void entry_free(Entry *entry);

void *hash_map_get(HashMap *hash_map, const void *key);

void hash_map_insert(HashMap *hash_map, const void *key, void *value);

unsigned long string_hash(const void *s);

int string_cmp(const void *s1, const void *s2);

#endif // !CO_HASH_MAP_H
