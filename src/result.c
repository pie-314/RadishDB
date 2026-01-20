#include "engine.h"
#include <stdio.h>

void print_result(FILE *out, const Result *r) {

  if (r->type == RES_STRING) {
    fprintf(out, "%s\n", r->value.string);
  }

  else if (r->type == RES_ERROR) {
    fprintf(out, "%s\n", r->value.string);
  }

  else if (r->type == RES_NIL) {
    fprintf(out, "(nil)\n");
  }

  else if (r->type == RES_OK) {
    fprintf(out, "OK\n");
  }

  else if (r->type == RES_INTEGER) {
    fprintf(out, "%ld\n", r->value.integer);
  }
}
