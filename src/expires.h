#include "hashtable.h"

void expire_init(HashTable *ht);
void expire_sweep(HashTable *ht, size_t max_checks);
