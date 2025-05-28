#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "input.h"
#include "parser.h"
#include "util.h"

static void createTokenBag(token_bag_t **token_bag);

token_bag_t *parseInput(const config_t *config) {
  int letter_count = 0;
  char *lineptr = NULL;
  size_t line_len = 0;
  FILE *in = openInput(config);
  char buffer[256];
  char *buffer_ix = NULL;
  char *index = NULL;
  token_bag_t *token_bag;

  createTokenBag(&token_bag);

getLine:
  while ((getline(&lineptr, &line_len, in) > -1)) {
    index = lineptr;
    while(1) {
      memset(buffer, 0, sizeof buffer);
      buffer_ix = buffer;
      letter_count = 0;

      while(isspace(*index)) {
        if (*index == '\n' || *index == '\r') goto getLine;
        index++;
      }
      if (*index == '\0') goto getLine;

      while(!isspace(*index) && *index != '\0') {
        if (letter_count > sizeof buffer) {
          fprintf(stderr, "A word exceeded the buffer size of %d bytes. Aborting.\n",
              sizeof buffer);
          abort();
        }
        *buffer_ix++ = *index;
        index++;
        letter_count++;
      }

      *buffer_ix = '\0';

      appendToken(token_bag, buffer);
    }
  }

  closeInput(config, in);
}

static void createTokenBag(token_bag_t **token_bag) {
  *token_bag = (token_bag_t *) fmalloc(sizeof (token_bag_t));
  (*token_bag)->tokens = (token_t **) fmalloc(sizeof (token_t));
  (*token_bag)->token_count = 0;
  (*token_bag)->tokens[0] = NULL;
}

void freeTokenBag(token_bag_t *token_bag) {

}

void appendToken(token_bag_t *token_bag, const char *token_text) {
  token_t *token;
  int i;

  if (!strlen(token_text)) return;

  i = token_bag->token_count;

  token_bag->token_count++;
  token_bag->tokens = frealloc(token_bag->tokens,
      sizeof(token_t *) * token_bag->token_count);

  token = (token_t *) fmalloc(sizeof(token_t));
  token->text = strdup(token_text);
  // TODO criar glyphs
  //
  // se o primeiro caracter for '/' e len > 1, trata-se de um prosign
  //
  //   para cada caracter a partir do segundo
  //     obter a representacao morse e concatenar
  //   gerar glyph contendo a representacao morse concatenada
  //     obter a duracao dos sons
  //     obter d duracao do traço farnsworth
  //   gerar glyph e defini-lo como o único glyph do token em questao
  //
  // do contrario (primeiro caracter nao é '/' ou len == 1)
  //   para cada caracter a partir do segundo
  //     obter a representacao morse
  //     obter a duracao dos sons
  //     obter a duracao do traço farnsworth
  //     gerar glyph e adiciona-lo a lista de glyphs do token

  token_bag->tokens[i] = token;
}

void printTokens(token_bag_t *token_bag) {

}
