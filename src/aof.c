#include "aof.h"
#include "utils.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define AOF_MAGIC "AOFX1"
#define AOF_MAGIC_LEN 5

static FILE *aof_file = NULL;

// just to open aof file
int aof_open(const char *filename) {
  aof_file = fopen(filename, "ab+");
  if (!aof_file)
    return 0;

  return 1;
}

// append value to file
void aof_append_set(const char *key, const char *value, const char *expire_at) {
  char buffer[256];
  if (expire_at == NULL) {
    snprintf(buffer, sizeof(buffer), "SET %s %s", key, value);
  } else {
    snprintf(buffer, sizeof(buffer), "SET %s %s EX %s", key, value, expire_at);
  }

  uint32_t length = (uint32_t)strlen(buffer);

  fwrite(&length, sizeof(uint32_t), 1, aof_file);
  fwrite(buffer, length, 1, aof_file);

  fflush(aof_file);
  fsync(fileno(aof_file));
}

// delete value
void aof_append_del(const char *key) {
  char buffer[256];
  snprintf(buffer, sizeof(buffer), "DEL %s", key);
  uint32_t length = (uint32_t)strlen(buffer);

  fwrite(&length, sizeof(uint32_t), 1, aof_file);
  fwrite(buffer, length, 1, aof_file);

  fflush(aof_file);
  fsync(fileno(aof_file));
}

// aof replay
int aof_replay(HashTable *ht, const char *filename) {
  FILE *f = fopen(filename, "rb");

  if (!f) {
    return 1;
  }

  char magic[6] = {0};
  uint64_t base_size = 0;

  if (fread(magic, 1, 5, f) == 5 && memcmp(magic, "AOFX1", 5) == 0) {
    fread(&base_size, sizeof(uint64_t), 1, f);
  } else {
    fseek(f, 0, SEEK_SET);
  }
  while (1) {
    uint32_t len;

    size_t n = fread(&len, sizeof(uint32_t), 1, f);
    if (n != 1) {
      break;
    }

    // Sanity check
    if (len == 0 || len > 1024 * 1024) {
      break;
    }

    char *cmd = malloc(len + 1);
    if (!cmd)
      break;

    size_t got = fread(cmd, 1, len, f);
    if (got != len) {
      free(cmd);
      break;
    }

    cmd[len] = '\0';

    char *argv[8];
    int argc = split_tokens(cmd, argv, 8);

    if (argc > 0) {
      if (strcmp(argv[0], "SET") == 0 && argc >= 3) {
        if (argc == 5 && strcmp(argv[3], "EX") == 0) {
          time_t expires = time(NULL) + (time_t)strtol(argv[4], NULL, 10);
          ht_set(ht, argv[1], argv[2], expires);
        } else {
          ht_set(ht, argv[1], argv[2], 0);
        }
      } else if (strcmp(argv[0], "DEL") == 0 && argc == 2) {
        ht_delete(ht, argv[1]);
      }
    }

    free(cmd);
  }

  fclose(f);
  return 1;
}

void aof_rewrite(HashTable *ht, const char *filename) {
  FILE *tmp = fopen("aof/radish.aof.tmp", "wb");
  if (!tmp)
    return;

  // Write header
  fwrite(AOF_MAGIC, 1, AOF_MAGIC_LEN, tmp);
  uint64_t base_size = 0;
  fwrite(&base_size, sizeof(uint64_t), 1, tmp);

  time_t now = time(NULL);

  for (size_t i = 0; i < ht->size; i++) {
    Entry *e = ht->buckets[i];
    while (e) {
      if (e->expires_at && e->expires_at <= now) {
        e = e->next;
        continue;
      }

      char cmd[256];
      if (e->expires_at) {
        int ttl = (int)(e->expires_at - now);
        snprintf(cmd, sizeof(cmd), "SET %s %s EX %d", e->key, e->value, ttl);
      } else {
        snprintf(cmd, sizeof(cmd), "SET %s %s", e->key, e->value);
      }

      uint32_t len = strlen(cmd);
      fwrite(&len, sizeof(uint32_t), 1, tmp);
      fwrite(cmd, 1, len, tmp);

      e = e->next;
    }
  }

  fflush(tmp);
  fsync(fileno(tmp));

  uint64_t final_size = ftell(tmp);

  fseek(tmp, 5, SEEK_SET);
  fwrite(&final_size, sizeof(uint64_t), 1, tmp);

  fflush(tmp);
  fsync(fileno(tmp));

  fclose(tmp);

  rename("aof/radish.aof.tmp", filename);

  fclose(aof_file);
  aof_file = fopen(filename, "ab");
}

// close aof file
void aof_close(void) {
  if (aof_file) {
    fclose(aof_file);
    aof_file = NULL;
  }
}
