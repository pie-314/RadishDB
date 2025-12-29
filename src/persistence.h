#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "hashtable.h"

int ht_save(HashTable *ht, const char *filename);
int ht_load(HashTable *ht, const char *filename);

#endif
