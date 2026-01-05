#include "aof.h"
#include "hashtable.h"
#include "persistence.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX_INPUT 256
#define MAX_TOKEN 8 // at max 3 is getting used for now

void trim_newline(char *s) {
  size_t len = strlen(s);
  if (len > 0 && s[len - 1] == '\n') {
    s[len - 1] = '\0';
  }
}

int split_tokens(char *line, char *argv[], int max) {
  int count = 0;
  char *tok = strtok(line, " ");
  while (tok != NULL && count < max) {
    argv[count++] = tok;
    tok = strtok(NULL, " ");
  }
  return count;
}

int main() {
  HashTable *ht = ht_create(8);
  char input[MAX_INPUT];

  // to clear screen before starting the REPL
  system("clear");
  aof_open("aof/aof.txt");
  aof_replay(ht, "aof/aof.txt");

  while (1) {
    printf(">>> ");
    fflush(stdout);

    if (!fgets(input, MAX_INPUT, stdin)) {
      break;
    }
    // taking input
    trim_newline(input);
    char *argv[MAX_TOKEN];
    int argc = split_tokens(input, argv, MAX_TOKEN);
    if (argc == 0) {
      continue;
    }

    if (strcmp(argv[0], "SET") == 0) {
      if (argc != 3) {
        printf("(error) wrong number of arguments for 'SET'\n");
        continue;
      }
      ht_set(ht, argv[1], argv[2]);
      aof_append_set(argv[1], argv[2]);
      printf("OK\n");
    }

    else if (strcmp(argv[0], "BENCH") == 0) {
      if (argc != 2) {
        printf("(error) wrong number of arguments for 'BENCH'\n");
        continue;
      }
      int n = atoi(argv[1]);
      if (n <= 0) {
        printf("(error) invalid number\n");
        continue;
      }
      struct timespec start, end;
      clock_gettime(CLOCK_MONOTONIC, &start);
      char key[64];
      char value[64];
      for (int i = 0; i < n; i++) {
        snprintf(key, sizeof(key), "key%d", i);
        snprintf(value, sizeof(value), "value%d", i);
        ht_set(ht, key, value);
      }
      clock_gettime(CLOCK_MONOTONIC, &end);
      double elapsed =
          (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
      printf("Inserted %d entries in %.2f seconds.\n", n, elapsed);
    }

    else if (strcmp(argv[0], "DEL") == 0) {
      if (argc != 2) {
        printf("(error) wrong number of arguments for 'DEL'\n");
        continue;
      }
      int value = ht_delete(ht, argv[1]);
      if (value)
        printf("%d\n", value);
      else
        printf("(nil)\n");
    }

    else if (strcmp(argv[0], "GET") == 0) {
      if (argc != 2) {
        printf("(error) wrong number of arguments for 'GET'\n");
        continue;
      }

      char *value = ht_get(ht, argv[1]);
      if (value)
        printf("%s\n", value);
      else
        printf("(nil)\n");
    }

    else if (strcmp(argv[0], "SAVE") == 0) {
      if (argc != 2) {
        printf("(error) wrong number of arguments for 'SAVE'\n");
        continue;
      }
      char filename[256];

      if (strstr(argv[1], ".rdbx"))
        snprintf(filename, sizeof(filename), "%s", argv[1]);
      else
        snprintf(filename, sizeof(filename), "%s.rdbx", argv[1]);

      if (ht_save(ht, filename)) {
        printf("OK\n");
      } else
        printf("(error) could not write file\n");
    }

    else if (strcmp(argv[0], "LOAD") == 0) {
      if (argc != 2) {
        printf("(error) wrong number of arguments for 'LOAD'\n");
        continue;
      }

      char *value = argv[1];
      if (value) {
        RdbStatus st = ht_load(ht, value);
        switch (st) {
        case RDB_OK:
          printf("OK\n");
          break;
        case RDB_ERR_OPEN:
          printf("(error) could not open file\n");
          break;
        case RDB_ERR_MAGIC:
          printf("(error) invalid RDBX file\n");
          break;
        default:
          printf("(error) load failed\n");
        }
      } else
        printf("(nil)\n");
    }

    else if (strcmp(argv[0], "COUNT") == 0) {
      printf("(integer) %d\n", ht->count);
    }

    else if (strcmp(argv[0], "CLEAR") == 0) {
      system("clear");
    }

    else if (strcmp(argv[0], "EXIT") == 0) {
      break;
    }

    else {
      printf("(error) Unknown command. \n");
    }
  }

  aof_close();
  ht_free(ht);
  return 0;
}
