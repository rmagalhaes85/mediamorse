#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "morse.h"

static void getMorseChar(const int c, char *out_buffer,
    size_t out_buffer_size) {

  assert(c);

#define MORSE_MAP(s, r) case s: case s - 0x20: \
  snprintf(out_buffer, out_buffer_size, "%s", #r); return
  switch (c) {
    MORSE_MAP('a', .-);
    MORSE_MAP('b', -...);
    MORSE_MAP('c', -.-.);
    MORSE_MAP('d', -..);
    MORSE_MAP('e', .);
    MORSE_MAP('f', ..-.);
    MORSE_MAP('g', --.);
    MORSE_MAP('h', ....);
    MORSE_MAP('i', ..);
    MORSE_MAP('j', .---);
    MORSE_MAP('k', -.-);
    MORSE_MAP('l', .-..);
    MORSE_MAP('m', --);
    MORSE_MAP('n', -.);
    MORSE_MAP('o', ---);
    MORSE_MAP('p', .--.);
    MORSE_MAP('q', --.-);
    MORSE_MAP('r', .-.);
    MORSE_MAP('s', ...);
    MORSE_MAP('t', -);
    MORSE_MAP('u', ..-);
    MORSE_MAP('v', ...-);
    MORSE_MAP('w', .--);
    MORSE_MAP('x', -..-);
    MORSE_MAP('y', -.--);
    MORSE_MAP('z', --..);
    MORSE_MAP('0', -----);
    MORSE_MAP('1', .----);
    MORSE_MAP('2', ..---);
    MORSE_MAP('3', ...--);
    MORSE_MAP('4', ....-);
    MORSE_MAP('5', .....);
    MORSE_MAP('6', -....);
    MORSE_MAP('7', --...);
    MORSE_MAP('8', ---..);
    MORSE_MAP('9', ----.);
    MORSE_MAP('/', -..-.);
    MORSE_MAP('?', ..--..);
    MORSE_MAP('!', -.-.--);
    MORSE_MAP('.', .-.-.-);
    MORSE_MAP(',', --..--);
    MORSE_MAP(':', ---...);
    MORSE_MAP(';', -.-.-.);
    MORSE_MAP('=', -...-);
    MORSE_MAP('+', .-.-.);
    MORSE_MAP('-', -....-);
    MORSE_MAP('_', ..--.-);
    MORSE_MAP('"', .-..-.);
    MORSE_MAP('@', .--.-.);
    MORSE_MAP('&', .-...);
    MORSE_MAP('(', -.--.);
    MORSE_MAP(')', -.--.-);
    MORSE_MAP('$', ...-..-);
    MORSE_MAP(' ', S);
    default:
      return getMorseChar(' ', out_buffer, out_buffer_size);
  }

}

char *getMorse(const char *glyph_contents) {
  char buffer[64];
  char morse_char[20];
  int buffer_remaining = sizeof(buffer) - 1;

  memset(buffer, '\0', sizeof(buffer));

  for (const char *ci = glyph_contents; *ci != '\0'; ++ci) {
    getMorseChar(*ci, morse_char, sizeof(morse_char));
    int morse_len = strlen(morse_char);
    if (morse_len > buffer_remaining) {
      fprintf(stderr, "No more available space in buffer\n");
      exit(1);
    }
    strncat(buffer, morse_char, morse_len);
    buffer_remaining -= morse_len;
  }

  return strdup(buffer);
}
