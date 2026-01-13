#include "hashtable.h"
#include <stdio.h>

void execute_command(HashTable *ht, const char *line, FILE *out);

#pragma once
#include <time.h>

extern time_t engine_start_time;
