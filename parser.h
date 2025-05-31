#ifndef PARSER_H
#define PARSER_H

#include "config.h"

typedef enum {
  prosign,
  word
} token_type_t;

typedef struct glyph_s {
  int sound_duration_ms;
  int total_duration_ms;
  char *text;
  struct glyph_s *next;
} glyph_t;

typedef struct token_s {
  char *text;
  token_type_t type;
  glyph_t *glyph_head;
  struct token_s *next;
} token_t;

typedef struct {
  token_t *token_head;
} token_bag_t;


token_bag_t *parseInput(const config_t *config);

#endif
