#include "expires.h"
#include "hashtable.h"
#include <stdlib.h>
#include <time.h>

static size_t expire_cursor = 0;
static long expired_deleted = 0;

void expire_init(HashTable *ht) {
  (void)ht;
  expire_cursor = 0;
  expired_deleted = 0;
}

void expire_sweep(HashTable *ht, size_t max_checks) {
  if (!ht || ht->size == 0 || max_checks == 0)
    return;

  time_t now = time(NULL);
  size_t buckets_scanned = 0;

  while (max_checks > 0 && buckets_scanned < ht->size) {
    size_t index = expire_cursor;
    Entry *entry = ht->buckets[index];
    Entry *prev = NULL;

    while (entry && max_checks > 0) {
      Entry *next = entry->next;

      if (entry->expires_at != 0 && entry->expires_at <= now) {
        // unlink
        if (prev == NULL) {
          ht->buckets[index] = next;
        } else {
          prev->next = next;
        }

        free(entry->key);
        free(entry->value);
        free(entry);

        ht->count--;
        expired_deleted++;
        // prev does NOT move on delete
      } else {
        prev = entry;
      }

      entry = next;
      max_checks--;
    }

    // move cursor to next bucket
    expire_cursor = (expire_cursor + 1) % ht->size;
    buckets_scanned++;
  }
}
