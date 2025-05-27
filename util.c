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
