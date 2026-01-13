#include "aof.h"
#include "engine.h"
#include "expires.h"
#include "hashtable.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_INPUT 256
#define MAX_TOKEN 8 // at max 3 is getting used for now

int main() {

  engine_start_time = time(NULL);
  HashTable *ht = ht_create(8);

  expire_init(ht);
  char input[MAX_INPUT];

  // to clear screen before starting the REPL
  system("clear");

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
    execute_command(ht, input, stdout);
  }

  return 0;
}
