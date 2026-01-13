#include "utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define AOF_MAGIC "AOFX1"
#define AOF_MAGIC_LEN 5

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

size_t aof_filesize(const char *path) {
  struct stat st;
  if (stat(path, &st) != 0)
    return 0;
  return (size_t)st.st_size;
}

size_t aof_header_filesize(const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f)
    return 0;

  char magic[6] = {0};
  if (fread(magic, 1, AOF_MAGIC_LEN, f) != AOF_MAGIC_LEN ||
      memcmp(magic, AOF_MAGIC, AOF_MAGIC_LEN) != 0) {
    fclose(f);
    return 0; // no header → old AOF
  }

  uint64_t base_size;
  if (fread(&base_size, sizeof(uint64_t), 1, f) != 1) {
    fclose(f);
    return 0; // corrupt header
  }

  fclose(f);
  return (size_t)base_size;
}
