#ifndef PARSER_H
#define PARSER_H

#include "config.h"

typedef struct {
  int sound_duration_ms;
  int total_duration_ms;
  char *text;
} glyph_t;

typedef struct {
  int glyph_count;
  glyph_t **glyphs;
  char *text;
} token_t;

typedef struct {
  int token_count;
  token_t **tokens;
} token_bag_t;


token_bag_t *parseInput(const config_t *config);
void freeTokenBag(token_bag_t *token_bag);
void appendToken(token_bag_t *token_bag, const char *token_text);
void printTokens(token_bag_t *token_bag);

#endif
