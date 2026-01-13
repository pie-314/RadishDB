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

time_t engine_start_time;

extern time_t engine_start_time;
void execute_command(HashTable *ht, char *line, FILE *out) {

  // taking input
  trim_newline(line);
  char *argv[MAX_TOKEN];
  int argc = split_tokens(line, argv, MAX_TOKEN);
  if (argc == 0) {
  }

  if (strcmp(argv[0], "SET") == 0) {
    if (argc != 3 && argc != 5) {
      fprintf(out, "(error) wrong number of arguments for 'SET'\n");
      return;
    }
    if (argc == 3) {
      ht_set(ht, argv[1], argv[2], 0);
      aof_append_set(argv[1], argv[2], 0);
      fprintf(out, "OK\n");
    } else if (argc == 5 && (strcmp(argv[3], "EX") == 0)) {
      if (strtol(argv[4], NULL, 10) >= 0) {
        time_t cur_time = time(NULL);
        time_t ttl_expiry = cur_time + (long)strtol(argv[4], NULL, 10);
        ht_set(ht, argv[1], argv[2], ttl_expiry);
        aof_append_set(argv[1], argv[2], argv[4]);
        fprintf(out, "OK\n");
      }
    }
  }

  else if (strcmp(argv[0], "BENCH") == 0) {
    if (argc != 2) {
      printf("(error) wrong number of arguments for 'BENCH'\n");

      return;
    }
    int n = atoi(argv[1]);
    if (n <= 0) {
      fprintf(out, "(error) invalid number\n");
      return;
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
    fprintf(out, "Inserted %d entries in %.2f seconds.\n", n, elapsed);
  }

  else if (strcmp(argv[0], "DEL") == 0) {
    if (argc != 2) {
      fprintf(out, "(error) wrong number of arguments for 'DEL'\n");
      return;
    }
    int value = ht_delete(ht, argv[1]);
    if (value)
      fprintf(out, "%d\n", value);
    else
      fprintf(out, "(nil)\n");
  }

  else if (strcmp(argv[0], "GET") == 0) {
    if (argc != 2) {
      fprintf(out, "(error) wrong number of arguments for 'GET'\n");
      return;
    }

    char *value = ht_get(ht, argv[1]);
    if (value)
      fprintf(out, "%s\n", value);
    else
      fprintf(out, "(nil)\n");
  }

  else if (strcmp(argv[0], "SAVE") == 0) {
    if (argc != 2) {
      fprintf(out, "(error) wrong number of arguments for 'SAVE'\n");
    }
    char filename[256];

    if (strstr(argv[1], ".rdbx"))
      snprintf(filename, sizeof(filename), "%s", argv[1]);
    else
      snprintf(filename, sizeof(filename), "%s.rdbx", argv[1]);

    if (ht_save(ht, filename)) {
      fprintf(out, "OK\n");
    } else
      fprintf(out, "(error) could not write file\n");
  }

  else if (strcmp(argv[0], "LOAD") == 0) {
    if (argc != 2) {
      fprintf(out, "(error) wrong number of arguments for 'LOAD'\n");
    }

    char *value = argv[1];
    if (value) {
      RdbStatus st = ht_load(ht, value);
      expire_init(ht);
      switch (st) {
      case RDB_OK:
        fprintf(out, "OK\n");
        break;
      case RDB_ERR_OPEN:
        fprintf(out, "(error) could not open file\n");
        break;
      case RDB_ERR_MAGIC:
        fprintf(out, "(error) invalid RDBX file\n");
        break;
      default:
        fprintf(out, "(error) load failed\n");
      }
    } else
      fprintf(out, "(nil)\n");
  }

  else if (strcmp(argv[0], "COUNT") == 0) {
    fprintf(out, "(integer) %d\n", ht->count);
  }

  else if (strcmp(argv[0], "INFO") == 0) {
    Info info = ht_info(ht);
    fprintf(out, "\nVersion : 0.1\n");
    fprintf(out, "Uptime Seconds : %ld\n\n", time(NULL) - engine_start_time);

    fprintf(out, "Keys : %d\n", info.keys);
    fprintf(out, "Keys with TTL : %d\n", info.keys_with_ttl);
    fprintf(out, "Expired Keys : %d\n\n", info.expired_keys);
    // printf("Expired keys deleted : %d\n\n", info.expired_keys);

    fprintf(out, "Buckets : %d\n", info.buckets);
    fprintf(out, "Load Factor : %3f\n", info.load_factor);
    fprintf(out, "Resizes : %d\n", info.resizes);
    fprintf(out, "Max chain : %d\n", info.max_chain);
  }

  else if (strcmp(argv[0], "TTL") == 0) {
    if (argc != 2) {
      fprintf(out, "(error) wrong number of arguments for 'TTL'\n");
      return;
    }
    long val = ht_ttl(ht, argv[1]);
    fprintf(out, "(staus) %ld\n", val);
  }

  else if (strcmp(argv[0], "CLEAR") == 0) {
    if (argc != 1) {
      fprintf(out, "(error) wrong number of arguments for 'TTL'\n");
      return;
    }

    system("clear");
  }

  else {
    fprintf(out, "(error) Unknown command. \n");
  }
}
