#include "hashtable.h"
#include <stdlib.h>
#include <string.h>

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

void ht_set(HashTable *ht, const char *key, const char *value) {
  unsigned long h = hash(key);
  int index = (int)(h % ht->size);
  Entry *entry = ht->buckets[index]; // head of the linked list for that bucket

  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      free(entry->value);
      entry->value = strdup(value);
      return;
    }
    entry = entry->next;
  }

  Entry *new_entry = malloc(sizeof(Entry));
  if (!new_entry)
    return;
  new_entry->key = strdup(key);
  new_entry->value = strdup(value);
  new_entry->next = ht->buckets[index];
  ht->buckets[index] = new_entry;
  ht->count++;
}

char *ht_get(HashTable *ht, const char *key) {
  unsigned long h = hash(key);
  int index = h % ht->size;

  Entry *entry = ht->buckets[index];
  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      return entry->value;
    }
    entry = entry->next;
  }
  return NULL;
}

int ht_delete(HashTable *ht, const char *key) {
  unsigned long h = hash(key);
  int index = (int)(h % ht->size);
  int flag;

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
