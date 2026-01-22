#include "aof.h"
#include "engine.h"
#include "expires.h"
#include "hashtable.h"
#include "repl.h"
#include "server.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_INPUT 256
#define MAX_TOKEN 8 // at max 3 is getting used for now

int main(int argc, char *argv[]) {
  engine_start_time = time(NULL);
  HashTable *ht = ht_create(8);

  expire_init(ht);

  // to clear screen before starting the REPL

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

  int server_mode = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--server") == 0 || strcmp(argv[i], "-s") == 0) {
      server_mode = 1;
      break;
    }

    if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
      printf("version 0.1\n");
      return 0;
    }

    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {

      printf("RadishDB Commands:\n"
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
      return 0;
    }
  }

  if (server_mode) {
    // server(ht, aof_size);
    start_server(ht);
  } else {

    system("clear");
    // REPL loop call
    repl_loop(ht, aof_size);
  }
  return 0;
}
