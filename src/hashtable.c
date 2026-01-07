#include "hashtable.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

HashTable *ht_create(int size) {
  HashTable *ht =
      malloc(sizeof(HashTable)); // allot memory for one HashTable struct

  ht->size = size;
  ht->count = 0;
  ht->buckets = malloc(size * sizeof(Entry *)); // multiply that one HashTable
                                                // with total enteries and allot

  for (int i = 0; i < size; i++) {
    ht->buckets[i] = NULL; // set all values to NULL (initialization)
  }
  return ht;
}

void ht_resize(HashTable *ht_old, int new_size) {
  HashTable *ht_new = malloc(sizeof(HashTable));
  ht_new->size = new_size;
  ht_new->count = 0;
  ht_new->buckets = malloc(new_size * sizeof(Entry *));

  for (int i = 0; i < new_size; i++) {
    ht_new->buckets[i] = NULL;
  }

  // rehash all entries
  for (int i = 0; i < ht_old->size; i++) {
    Entry *entry = ht_old->buckets[i];
    while (entry != NULL) {
      Entry *next_entry = entry->next;

      unsigned long h = hash(entry->key);
      int new_index = h % ht_new->size;

      entry->next = ht_new->buckets[new_index];
      ht_new->buckets[new_index] = entry;
      ht_new->count++;

      entry = next_entry;
    }
  }

  free(ht_old->buckets);

  ht_old->buckets = ht_new->buckets;
  ht_old->size = ht_new->size;
  ht_old->count = ht_new->count;

  free(ht_new);
}

void ht_set(HashTable *ht, const char *key, const char *value,
            time_t expires_at) {
  // resizing check
  float load = (float)ht->count / ht->size;
  if (load > 0.75f) {
    ht_resize(ht, ht->size * 2);
  }

  unsigned long h = hash(key);
  int index = h % ht->size;

  Entry *entry = ht->buckets[index];
  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      free(entry->value);
      entry->value = strdup(value);
      entry->expires_at = expires_at;
      return;
    }
    entry = entry->next;
  }

  Entry *new_entry = malloc(sizeof(Entry));
  if (!new_entry)
    return;

  new_entry->key = strdup(key);
  new_entry->value = strdup(value);
  new_entry->expires_at = expires_at;
  new_entry->next = ht->buckets[index];

  ht->buckets[index] = new_entry;
  ht->count++;
}

char *ht_get(HashTable *ht, const char *key) {
  unsigned long h = hash(key);
  int index = h % ht->size;

  time_t cur_time = time(NULL);
  Entry *entry = ht->buckets[index];
  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      if (cur_time >= entry->expires_at && entry->expires_at != 0) {
        ht_delete(ht, key);
        return NULL;
      }
      return entry->value;
    }
    entry = entry->next;
  }
  return NULL;
}

long ht_ttl(HashTable *ht, const char *key) {
  unsigned long h = hash(key);
  int index = h % ht->size;

  time_t cur_time = time(NULL);
  Entry *entry = ht->buckets[index];

  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      if (cur_time >= entry->expires_at && entry->expires_at != 0) {
        ht_delete(ht, key);
        return -2;
      } else if (entry->expires_at == 0) {
        return -1;
      }
      return entry->expires_at - cur_time;
    }
    entry = entry->next;
  }
  return -2;
}

int ht_delete(HashTable *ht, const char *key) {
  unsigned long h = hash(key);
  int index = (int)(h % ht->size);

  Entry *entry = ht->buckets[index];
  Entry *prev = NULL;
  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      if (prev == NULL) {
        ht->buckets[index] = entry->next;
      } else {
        prev->next = entry->next;
      }
      free(entry->value);
      free(entry->key);
      free(entry);
      return 1;
    }
    prev = entry;
    entry = entry->next;
  }
  return 0;
}

void ht_free(HashTable *ht) {
  int size = ht->size;
  for (int index = 0; index < size; index++) {
    Entry *entry = ht->buckets[index];
    while (entry != NULL) {
      Entry *next = entry->next;
      free(entry->key);
      free(entry->value);
      free(entry);
      entry = next;
    }
  }
  free(ht->buckets);
  free(ht);
}

unsigned long hash(const char *str) {
  unsigned long hash = 5381;
  for (int i = 0; str[i] != '\0'; i++) {
    hash = hash * 33 + str[i];
  }
  return hash;
}
