#include "hashtable.h"
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
  HashTable *ht = ht_create(8);

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

    else {
      printf("Unknown command. \n");
    }
  }

  ht_free(ht);
  return 0;
}
