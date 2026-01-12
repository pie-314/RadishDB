#include <stdio.h>

void trim_newline(char *s);
int split_tokens(char *line, char *argv[], int max);

size_t aof_filesize(const char *path);
size_t aof_header_filesize(const char *path);
