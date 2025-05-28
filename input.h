#ifndef INPUT_H
#define INPUT_H

#include <stdio.h>

#include "config.h"

FILE *openInput(const config_t *config);
void closeInput(const config_t *config, FILE *file);

#endif
