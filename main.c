#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "config.h"
#include "util.h"

// definitions
int main(int argc, char *argv[]) {
  config_t *config = parseConfig(argc, argv);
  free(config);
  return 0;
}

