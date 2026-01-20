#include "repl.h"
#include "aof.h"
#include "engine.h"
#include "expires.h"
#include "hashtable.h"
#include "result.h"
#include "utils.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TOKEN 8 // at max 3 is getting used for now
#define MAX_INPUT 256

void repl_loop(HashTable *ht, size_t aof_size) {
  char input[MAX_INPUT];
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
    Result engine_return = execute_command(ht, input);
    if (engine_return.type == RES_CLEAN) {
      system("clear");
    } else {
      print_result(stdout, &engine_return);
    }
    free_result(&engine_return);
  }
}
