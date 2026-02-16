#include "engine.h"
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

Result result_ok(void) {
  Result r;
  r.type = RES_OK;
  return r;
}
Result result_clean(void) {
  Result r;
  r.type = RES_CLEAN;
  return r;
}

Result result_nil(void) {
  Result r;
  r.type = RES_NIL;
  return r;
}

Result result_int(long v) {
  Result r;
  r.type = RES_INTEGER;
  r.value.integer = v;
  return r;
}

Result result_string(const char *s) {
  Result r;
  r.type = RES_STRING;
  r.value.string = strdup(s);
  return r;
}
Result result_exit(void) {
  Result r;
  r.type = RES_EXIT;
  return r;
}

Result result_error(const char *msg) {
  Result r;
  r.type = RES_ERROR;
  r.value.string = strdup(msg);
  return r;
}

Result execute_command(HashTable *ht, char *line) {

  // taking input
  trim_newline(line);
  char *argv[MAX_TOKEN];
  int argc = split_tokens(line, argv, MAX_TOKEN);
  if (argc == 0) {
    return result_nil();
  }

  if (strcmp(argv[0], "SET") == 0) {
    if (argc != 3 && argc != 5) {
      return result_error("(error) wrong number of arguments for 'SET'");
    }
    if (argc == 3) {
      ht_set(ht, argv[1], argv[2], 0);
      aof_append_set(argv[1], argv[2], 0);

      return result_ok();
    } else if (argc == 5 && (strcmp(argv[3], "EX") == 0)) {
      if (strtol(argv[4], NULL, 10) >= 0) {
        time_t cur_time = time(NULL);
        time_t ttl_expiry = cur_time + (long)strtol(argv[4], NULL, 10);
        ht_set(ht, argv[1], argv[2], ttl_expiry);
        aof_append_set(argv[1], argv[2], argv[4]);
        return result_ok();
      }
    }
  }

  else if (strcmp(argv[0], "BENCH") == 0) {
    if (argc != 2) {
      return result_error("(error) wrong number of arguments for 'BENCH'");
    }
    int n = atoi(argv[1]);
    if (n <= 0) {
      return result_error("(error) invalid number");
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
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Inserted %d entries in %.2f seconds.", n,
             elapsed);
    return result_string(buffer);
  }

  else if (strcmp(argv[0], "DEL") == 0) {
    if (argc != 2) {
      return result_error("(error) wrong number of arguments for 'DEL'");
    }
    int value = ht_delete(ht, argv[1]);
    if (value) {
      aof_append_del(argv[1]);
      return result_int(value);
    } else
      return result_nil();
  }

  else if (strcmp(argv[0], "GET") == 0) {
    if (argc != 2) {
      return result_error("(error) wrong number of arguments for 'GET'");
    }

    char *value = ht_get(ht, argv[1]);
    if (value)
      return result_string(value);
    else
      return result_nil();
  }

  else if (strcmp(argv[0], "SAVE") == 0) {
    if (argc != 2) {
      return result_error("(error) wrong number of arguments for 'SAVE'");
    }
    char filename[256];

    if (strstr(argv[1], ".rdbx"))
      snprintf(filename, sizeof(filename), "%s", argv[1]);
    else
      snprintf(filename, sizeof(filename), "%s.rdbx", argv[1]);

    if (ht_save(ht, filename)) {
      return result_ok();
    } else
      return result_error("(error) could not write file");
  }

  else if (strcmp(argv[0], "LOAD") == 0) {
    if (argc != 2) {
      return result_error("(error) wrong number of arguments for 'LOAD'");
    }

    char *value = argv[1];
    if (value) {
      RdbStatus st = ht_load(&ht, value);
      expire_init(ht);
      switch (st) {
      case RDB_OK:
        return result_ok();
        break;
      case RDB_ERR_OPEN:
        return result_error("(error) could not open file");
        break;
      case RDB_ERR_MAGIC:
        return result_error("(error) invalid RDBX file");
        break;
      default:
        return result_error("(error) load failed");
      }
    } else
      return result_nil();
  }

  else if (strcmp(argv[0], "COUNT") == 0) {
    return result_int(ht->count);
  }

  else if (strcmp(argv[0], "INFO") == 0) {
    Info info = ht_info(ht);

    char buffer[512];
    snprintf(
        buffer, sizeof(buffer),
        "Version : 0.1\nUptime Seconds :%ld\n\nKeys : %d\nKeys with TTL : "
        "%d\nExpired keys : %d\n\nBuckets : %d\nLoad Factor : %3f\nResizes "
        ": %d\nMax chain : %d\n",
        time(NULL) - engine_start_time, info.keys, info.keys_with_ttl,
        info.expired_keys, info.buckets, info.load_factor, info.resizes,
        info.max_chain);

    return result_string(buffer);
  }

  else if (strcmp(argv[0], "TTL") == 0) {
    if (argc != 2) {
      return result_error("(error) wrong number of arguments for 'TTL'");
    }
    long val = ht_ttl(ht, argv[1]);
    return result_int(val);
  }

  else if (strcmp(argv[0], "HELP") == 0) {
    if (argc != 1) {
      return result_error("(error) wrong number of arguments for 'HELP'");
    }

    return result_string(
        "RadishDB Commands:\n"
        "\n"
        "SET key value\n"
        "  Set key to hold the given value.\n"
        "\n"
        "SET key value EX seconds\n"
        "  Set key with a time-to-live in seconds.\n"
        "\n"
        "GET key\n"
        "  Get the value of key.\n"
        "  Returns (nil) if the key does not exist or is expired.\n"
        "\n"
        "DEL key\n"
        "  Delete the specified key.\n"
        "  Returns number of keys removed (0 or 1).\n"
        "\n"
        "TTL key\n"
        "  Get remaining time-to-live of key.\n"
        "  Returns:\n"
        "   -1 if key exists but has no expiration\n"
        "   -2 if key does not exist\n"
        "\n"
        "COUNT\n"
        "  Return the number of keys in the database.\n"
        "\n"
        "SAVE file\n"
        "  Save the current database state to a .rdbx snapshot.\n"
        "\n"
        "LOAD file\n"
        "  Load database state from a .rdbx snapshot.\n"
        "  Replaces the current in-memory state.\n"
        "\n"
        "BENCH n\n"
        "  Insert n test key-value pairs for benchmarking.\n"
        "\n"
        "INFO\n"
        "  Show database statistics and internal metrics.\n"
        "\n"
        "CLEAR\n"
        "  Clear the screen (REPL only).\n"
        "\n"
        "HELP\n"
        "  Show this help message.\n");
  } else if (strcmp(argv[0], "EXIT") == 0 || strcmp(argv[0], "QUIT") == 0) {
    if (argc != 1) {
      return result_error("(error) wrong number of arguments for 'EXIT'");
    }
    return result_exit();
  }

  else if (strcmp(argv[0], "CLEAR") == 0) {
    if (argc != 1) {
      return result_error("(error) wrong number of arguments for 'CLEAR'");
    }
    return result_clean();
  }

  else {
    return result_error("(error) Unknown command.");
  }
  return result_nil();
}

void free_result(Result *r) {
  if ((r->type == RES_STRING || r->type == RES_ERROR) && r->value.string) {
    free(r->value.string);
  }
}
