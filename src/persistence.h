#ifndef PERSISTENCE_H
#define PERSISTENCE_H
#include "hashtable.h"

typedef enum {
  RDB_OK = 1,

  RDB_ERR_OPEN,  // fopen failed
  RDB_ERR_MAGIC, // wrong RDBX header
} RdbStatus;

int ht_save(HashTable *ht, const char *filename);
int ht_load(HashTable *ht, const char *filename);

#endif
