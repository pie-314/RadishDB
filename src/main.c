#include "aof.h"
#include "expires.h"
#include "hashtable.h"
#include "persistence.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_INPUT 256
#define MAX_TOKEN 8 // at max 3 is getting used for now

int main() {
  HashTable *ht = ht_create(8);

  expire_init(ht);
  char input[MAX_INPUT];

  // to clear screen before starting the REPL
  system("clear");

  time_t start_time = time(NULL);
  aof_open("aof/radish.aof");
  aof_replay(ht, "aof/radish.aof");
  expire_init(ht);

  // Rewrite AOF if too big
  size_t aof_base_size = aof_header_filesize("aof/radish.aof");

  // aof_rewrite(ht, "aof/radish.aof");
  size_t aof_size = aof_filesize("aof/radish.aof");

  if (aof_size > aof_base_size * 2 || aof_base_size == 0) {
    printf("[AOF] startup rewrite (%zu bytes)\n", aof_size);
    aof_rewrite(ht, "aof/radish.aof");
    aof_base_size = aof_header_filesize("aof/radish.aof");
  }

  system("clear");

  // REPL loop
  while (1) {
    expire_sweep(ht, 10);

    size_t aof_base_size = aof_header_filesize("aof/radish.aof");
    aof_size = aof_filesize("aof/radish.aof");
    if (aof_size > aof_base_size * 2) {
      printf("[AOF] rewrite (%zu bytes)\n", aof_size);
      aof_rewrite(ht, "aof/radish.aof");
      aof_base_size = aof_header_filesize("aof/radish.aof");
    }

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
      if (argc != 3 && argc != 5) {
        printf("(error) wrong number of arguments for 'SET'\n");
        continue;
      }
      if (argc == 3) {
        ht_set(ht, argv[1], argv[2], 0);
        aof_append_set(argv[1], argv[2], 0);
        printf("OK\n");
      } else if (argc == 5 && (strcmp(argv[3], "EX") == 0)) {
        if (strtol(argv[4], NULL, 10) >= 0) {
          time_t cur_time = time(NULL);
          time_t ttl_expiry = cur_time + (long)strtol(argv[4], NULL, 10);
          ht_set(ht, argv[1], argv[2], ttl_expiry);
          aof_append_set(argv[1], argv[2], argv[4]);
          printf("OK\n");
        }
      }
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
        ht_set(ht, key, value, 0);
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
        expire_init(ht);
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
    } else if (strcmp(argv[0], "INFO") == 0) {
      Info info = ht_info(ht);
      printf("\nVersion : 0.1\n");
      printf("Uptime Seconds : %ld\n\n", time(NULL) - start_time);

      printf("Keys : %d\n", info.keys);
      printf("Keys with TTL : %d\n", info.keys_with_ttl);
      printf("Expired Keys : %d\n\n", info.expired_keys);
      // printf("Expired keys deleted : %d\n\n", info.expired_keys);

      printf("Buckets : %d\n", info.buckets);
      printf("Load Factor : %3f\n", info.load_factor);
      printf("Resizes : %d\n", info.resizes);
      printf("Max chain : %d\n", info.max_chain);
    }

    else if (strcmp(argv[0], "TTL") == 0) {
      if (argc != 2) {
        printf("(error) wrong number of arguments for 'TTL'\n");
        continue;
      }
      long val = ht_ttl(ht, argv[1]);
      printf("(staus) %ld\n", val);
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
