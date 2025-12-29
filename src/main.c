#include "hashtable.h"
#include "persistence.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_INPUT 256
#define MAX_TOKEN 8

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
  HashTable *ht = ht_create(MAX_TOKEN);

  char input[MAX_INPUT];

  while (1) {
    printf(">");
    fflush(stdout);

    if (!fgets(input, MAX_INPUT, stdin)) {
      break;
    }

    trim_newline(input);

    char *argv[MAX_TOKEN];
    int argc = split_tokens(input, argv, MAX_TOKEN);

    if (argc == 0) {
      continue;
    }

    if (strcmp(argv[0], "SET") == 0) {
      ht_set(ht, argv[1], argv[2]);
    }

    else if (strcmp(argv[0], "BENCH") == 0 && argc == 2) {

      int n = atoi(argv[1]);
      if (n <= 0) {
        printf("Invalid number.\n");
        continue;
      }

      for (int i = 0; i < n; i++) {
        char key[64];
        char value[64];
        sprintf(key, "key%d", i);
        sprintf(value, "val%d", i);
        ht_set(ht, key, value);
      }

      printf("Inserted %d entries.\n", n);
    }

    else if (strcmp(argv[0], "DEL") == 0) {
      int value = ht_delete(ht, argv[1]);
      if (value)
        printf("%d\n", value);
      else
        printf("(nil)\n");
    }

    else if (strcmp(argv[0], "GET") == 0) {
      char *value = ht_get(ht, argv[1]);
      if (value)
        printf("%s\n", value);
      else
        printf("(nil)\n");
    }

    else if (strcmp(argv[0], "EXIT") == 0) {
      break;
    }

    else if (strcmp(argv[0], "SAVE") == 0) {
      char *value = strcat(argv[1], ".rdbx");
      if (value) {
        ht_save(ht, value);
        printf("FILE SAVED : %s\n", value);
      } else
        printf("(nil)\n");
    }

    else if (strcmp(argv[0], "LOAD") == 0) {
      char *value = argv[1];
      if (value) {
        ht_load(ht, value);
        printf("FILE LOADED : %s\n", value);
      } else
        printf("(nil)\n");
    }

    else if (strcmp(argv[0], "COUNT") == 0) {
      printf("%d\n", ht->count);
    } else {
      printf("Unknown command. \n");
    }
  }

  ht_free(ht);
  return 0;
}
