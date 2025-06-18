#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "morse.h"

static void getMorseChar(const int c, char *out_buffer,
    size_t out_buffer_size) {

  assert(c);
  int upper_c = isalpha(c) ? toupper(c) : c;

#define MORSE_MAP(s, r) case s: \
  snprintf(out_buffer, out_buffer_size, "%s", #r); return
  switch (upper_c) {
    MORSE_MAP('A', .-);
    MORSE_MAP('B', -...);
    MORSE_MAP('C', -.-.);
    MORSE_MAP('D', -..);
    MORSE_MAP('E', .);
    MORSE_MAP('F', ..-.);
    MORSE_MAP('G', --.);
    MORSE_MAP('H', ....);
    MORSE_MAP('I', ..);
    MORSE_MAP('J', .---);
    MORSE_MAP('K', -.-);
    MORSE_MAP('L', .-..);
    MORSE_MAP('M', --);
    MORSE_MAP('N', -.);
    MORSE_MAP('O', ---);
    MORSE_MAP('P', .--.);
    MORSE_MAP('Q', --.-);
    MORSE_MAP('R', .-.);
    MORSE_MAP('S', ...);
    MORSE_MAP('T', -);
    MORSE_MAP('U', ..-);
    MORSE_MAP('V', ...-);
    MORSE_MAP('W', .--);
    MORSE_MAP('X', -..-);
    MORSE_MAP('Y', -.--);
    MORSE_MAP('Z', --..);
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
