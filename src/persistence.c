#include "persistence.h"
#include "hashtable.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ht_save(HashTable *ht, const char *filename) {
  FILE *f = fopen(filename, "wb");
  if (!f)
    return 0;

  fwrite("RDBX1", 1, 5, f);
  uint32_t count = ht->count;
  fwrite(&count, sizeof(uint32_t), 1, f);

  for (size_t i = 0; i < ht->size; i++) {
    Entry *entry = ht->buckets[i];

    while (entry != NULL) {
      uint32_t klen = strlen(entry->key);
      fwrite(&klen, sizeof(uint32_t), 1, f);
      fwrite(entry->key, 1, klen, f);

      uint32_t vlen = strlen(entry->value);
      fwrite(&vlen, sizeof(uint32_t), 1, f);
      fwrite(entry->value, 1, vlen, f);

      fwrite(&entry->expires_at, sizeof(time_t), 1, f);

      entry = entry->next;
    }
  }
  fclose(f);
  return 1;
}

int ht_load(HashTable *ht, const char *filename) {
  FILE *f = fopen(filename, "rb");
  if (!f)
    return RDB_ERR_OPEN;

  char magic[6];
  fread(magic, 1, 5, f);
  magic[5] = '\0';

  if (strcmp(magic, "RDBX1") != 0) {
    fclose(f);
    return RDB_ERR_MAGIC;
  }

  ht_free(ht);
  ht_create(8);

  uint32_t count;
  fread(&count, sizeof(uint32_t), 1, f);

  for (uint32_t i = 0; i < count; i++) {

    uint32_t klen;
    fread(&klen, sizeof(uint32_t), 1, f);

    // stack can't be used because we will know length at runtime
    char *key = malloc(klen + 1);
    fread(key, 1, klen, f);
    key[klen] = '\0';

    uint32_t vlen;
    fread(&vlen, sizeof(uint32_t), 1, f);
    char *value = malloc(vlen + 1);
    fread(value, 1, vlen, f);
    value[vlen] = '\0';

    time_t expires_at;
    fread(&expires_at, sizeof(time_t), 1, f);
    ht_set(ht, key, value, expires_at);

    // temp heap cleanup
    free(key);
    free(value);
  }
  fclose(f);
  return RDB_OK;
}
