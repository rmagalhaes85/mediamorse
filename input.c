#include <stdlib.h>

#include "input.h"

FILE *openInput(const config_t *config) {
  if (config->read_stdin) {
    return stdin;
  } else {
    FILE *input_file;
    input_file = fopen(config->input_filename, "r");
    if (!input_file) {
        fprintf(stderr, "Couldn't open input file %s\n", config->input_filename);
        exit(1); 
    }
    return input_file;
  }
}

void closeInput(const config_t *config, FILE *file) {
  if (!config->read_stdin) return;
  fclose(file);
}
