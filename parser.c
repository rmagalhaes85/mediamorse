#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "input.h"
#include "parser.h"
#include "util.h"

static void createTokenBag(token_bag_t **token_bag);

// token bag
static void tokenBagFree(token_bag_t *token_bag);
static void tokenBagAppendToken(token_bag_t *token_bag, const char *token_text);
static int tokenBagTotalDurationMs(const token_bag_t *token_bag);
static void tokenBagPrint(token_bag_t *token_bag);

// token
static token_t *tokenCreate(const char *text);
static void tokenFree(token_t *token);

// glyph
static void glyphFree(glyph_t *glyph);
static glyph_t *glyphsCreate(const char *text, token_type_t type);


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

      tokenBagAppendToken(token_bag, buffer);
    }
  }

  closeInput(config, in);
}

static void createTokenBag(token_bag_t **token_bag) {
  *token_bag = (token_bag_t *) fmalloc(sizeof (token_bag_t));
  (*token_bag)->token_head = NULL;
}

static void tokenBagFree(token_bag_t *token_bag) {
  // for each token
  //   free each glyph
  //   free token
  // free token bag
}

static void tokenBagAppendToken(token_bag_t *token_bag, const char *token_text) {
  token_t *token = tokenCreate(token_text);
  if (token_bag->token_head == NULL) {
    token_bag->token_head = token;
    return;
  }
  token_t *temp = token_bag->token_head;
  while (temp->next != NULL) {
    temp = temp->next;
  }
  temp->next = token;
}

static int tokenBagTotalDurationMs(const token_bag_t *token_bag) {
  fprintf(stderr, "tokenBagTotalDurantionMs: not implemented\n");
  exit(1);
}

static void tokenBagPrint(token_bag_t *token_bag) {
  printf("tokenBagPrint: to be implemented\n");
}

static token_t *tokenCreate(const char *text) {
  int len = strlen(text);
  token_t *token = (token_t *) fmalloc(sizeof (token_t));
  token->text = strdup(text);
  token->type = (len > 1 && text[0] == '/') ? prosign : word;
  token->glyph_head = glyphsCreate(text, token->type);
  token->next = NULL;
  return token;
}

static void glyphFillChar(glyph_t *glyph, const char c) {
  // here we expect that the buffer will be allocated by us
  assert(glyph->text == NULL);
  size_t bufsz = sizeof(char) * 2;
  glyph->text = (char *) fmalloc(bufsz);
  memset(glyph->text, '\0', bufsz);
  int written = snprintf(glyph->text, bufsz, "%c", c);
  assert(written == 1);
}

static glyph_t *glyphsCreate(const char *text, token_type_t type) {
  int len = strlen(text);
  const char *c;
  glyph_t *head = NULL;

  if (len == 0) {
    return head;
  }

  c = text;
  head = (glyph_t *) fmalloc(sizeof (glyph_t));
  head->next = NULL;

  if (type == prosign) {
    assert(len > 1 && text[0] == '/');
    // the initial '/' in prosign tokens are ignored when building the glyph:
    c++;
    head->text = strdup(c);
    // TODO: compute glyph durations
    return head;
  } else {
    glyphFillChar(head, *c);
    c++;
    glyph_t *temp = head;

    while (*c != '\0') {
      temp->next = (glyph_t *) fmalloc(sizeof (glyph_t));
      temp->next->next = NULL;
      glyphFillChar(temp->next, *c);
      // TODO compute glyph durations
      temp = temp->next;
      c++;
    }
  }
  return head;
}

