
#pragma once
#include "hashtable.h"
#include <stdio.h>
#include <time.h>

typedef enum {
  RES_STRING,
  RES_OK,
  RES_ERROR,
  RES_INTEGER,
  RES_NIL,
  RES_CLEAN,
  RES_EXIT
} ResultType;

typedef struct Result {
  ResultType type;
  union {
    char *string; // for STRING and ERROR
    long integer; // for INTEGER
  } value;
} Result;

Result execute_command(HashTable *ht, char *line);
void free_result(Result *r);
// typedef enum { ERR_NO_ARG, RES_OK, RES_ERROR, RES_INTEGER, RES_NIL } Errors;
extern time_t engine_start_time;
