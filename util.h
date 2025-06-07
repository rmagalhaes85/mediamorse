#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

void *fmalloc(size_t size);
void *frealloc(void *ptr, size_t new_size);
int is_valid_filename(const char *filename);
int run_and_capture(const char *cmd, char *const argv[]);
int run_command(const char *cmd, char *const argv[]);

#endif

