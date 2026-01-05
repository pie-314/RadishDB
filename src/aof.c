#include "aof.h"
#include <stdio.h>
#include <string.h>

static FILE *aof_file = NULL;
static FILE *aof_read = NULL;

// just to open aof file
int aof_open(const char *filename) {
  aof_file = fopen(filename, "a+");
  if (!aof_file)
    return 0;
  return 1;
}

// append value to file
int aof_append_set(const char *key, const char *value) {
  if (fprintf(aof_file, "SET %s %s\n", key, value) >= 0) {
    fflush(aof_file);
    return 1;
  } else {
    return 0;
  }
}

// delete value
int aof_append_del(const char *key) {
  if (fprintf(aof_file, "DEL %s\n", key) >= 0) {
    fflush(aof_file);
    return 1;
  } else {
    return 0;
  }
}

int aof_replay(HashTable *ht, const char *filename) {
  FILE *f = fopen(filename, "r");

  // No AOF yet is NOT an error (fresh database)
  if (!f) {
    return 1;
  }

  char buffer[256];

  while (fgets(buffer, sizeof(buffer), f)) {
    trim_newline(buffer);

    char *argv[3];
    int argc = split_tokens(buffer, argv, 3);

    if (argc == 0) {
      continue;
    }

    if (strcmp(argv[0], "SET") == 0) {
      if (argc == 3) {
        ht_set(ht, argv[1], argv[2]);
      }
    } else if (strcmp(argv[0], "DEL") == 0) {
      if (argc == 2) {
        ht_delete(ht, argv[1]);
      }
    }
    // Unknown or malformed lines are ignored safely
  }

  fclose(f);
  return 1;
}
// close aof file
void aof_close(void) {
  if (aof_file) {
    fclose(aof_file);
    aof_file = NULL;
  }
}
