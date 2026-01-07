#ifndef HASHTABLE_H
#define HASHTABLE_H
#include <time.h>

#define HASHTABLE_H
typedef struct Entry {
  char *key;
  char *value;
  time_t expires_at;
  struct Entry *next;
} Entry;

typedef struct HashTable {
  Entry **buckets;
  int size;
  int count;
} HashTable;

HashTable *ht_create(int size);
unsigned long hash(const char *str);
void ht_set(HashTable *ht, const char *key, const char *value,
            time_t expires_at);
char *ht_get(HashTable *ht, const char *key);
int ht_delete(HashTable *ht, const char *key);
void ht_free(HashTable *ht);
void ht_resize(HashTable *ht, int new_size);
long ht_ttl(HashTable *ht, const char *key);

#endif
