#ifndef AOF_H
#define AOF_H

#include "hashtable.h"

void trim_newline(char *s);
int split_tokens(char *line, char *argv[], int max);

int aof_open(const char *filename);
void aof_close(void);

int aof_append_set(const char *key, const char *value);
int aof_append_del(const char *key);

int aof_replay(HashTable *h, const char *filename);

#endif
