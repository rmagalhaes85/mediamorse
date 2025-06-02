#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>


// Configuration
//
typedef struct config {
  // input
  char *input_filename;
  bool read_stdin;
  // morse
  int normal_wpm;
  int farnsworth_wpm;
  float normal_unit_ms;
  float farnsworth_unit_ms;
  // audio
  int freq;
  bool noise;
  int bandwidth;
  int bitrate;
  // video
  char *bgcolor;
  char *fgcolor;
  char *hlcolor;
  bool showtitle;
  int title_duration_ms;
} config_t;

config_t *parseConfig(int argc, char *argv[]);
void printConfig(config_t *config);

#endif
