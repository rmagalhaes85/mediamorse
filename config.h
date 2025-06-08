#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef enum {
  MODE_SYNC,
  MODE_GUESS,
} morse_mode_t;

// Configuration
//
typedef struct config {
  // input/output
  char *input_filename;
  bool read_stdin;
  char *output_filename;
  bool keep_tmp_files;
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
  int video_width;
  int video_height;
  int framerate;
  morse_mode_t morse_mode;
} config_t;

config_t *parseConfig(int argc, char *argv[]);
void printConfig(config_t *config);

#endif
