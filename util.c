#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "util.h"

void *fmalloc(size_t size) {
  void *pt = malloc(size);
  if (pt == NULL) {
    fprintf(stderr, "Error allocating %d bytes. Aborting.\n", size);
    exit(1);
  }
  return pt;
}

void *frealloc(void *ptr, size_t new_size) {
  void *pt = realloc(ptr, new_size);
  if (pt == NULL) {
    fprintf(stderr, "Error allocating %d bytes. Aborting.\n", new_size);
    exit(1);
  }
  return pt;
}

