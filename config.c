#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#define DEFAULT_WPM (15)

#include "config.h"
#include "util.h"

config_t *parseConfig(int argc, char *argv[]) {
  config_t *config = (config_t *) fmalloc(sizeof (config_t));
  memset(config, 0, sizeof (config_t));
  bool last_arg = false;
  bool farnsworth_informed = false;
  bool inputfile_was_informed = false;
  int bitrate = 44100;
  int freq = 500;

  for (int i = 1; i < argc; ++i) {
    last_arg = i == argc - 1;
    if (!last_arg && (!strcmp("-i", argv[i]) || !strcmp("--input", argv[i]))) {
      config->input_filename = argv[++i];
      inputfile_was_informed = true;
    } else if (!last_arg && (!strcmp("-s", argv[i]) || !strcmp("--speed", argv[i]))) {
      errno = 0;
      char *end;
      config->normal_wpm = strtol(argv[++i], &end, 10);
      if (errno != 0 || argv[i] == end) {
        fprintf(stderr, "Invalid wpm speed\n");
        exit(1);
      }
    } else if (!last_arg && (!strcmp("-fw", argv[i]) || !strcmp("--farnsworth", argv[i]))) {
      farnsworth_informed = true;
      errno = 0;
      char *end;
      config->farnsworth_wpm = strtol(argv[++i], &end, 10);
      if (errno != 0 || argv[i] == end) {
        fprintf(stderr, "Invalid farnsworth wpm speed\n");
        exit(1);
      }
    } else if (!last_arg && (!strcmp("-f", argv[i]) || !strcmp("--frequency", argv[i]))) {
      errno = 0;
      char *end;
      freq = strtol(argv[++i], &end, 10);
      if (errno != 0 || argv[i] == end) {
        fprintf(stderr, "Invalid frequency\n");
        exit(1);
      }
    } else if (!strcmp("-n", argv[i]) || !strcmp("--noise", argv[i])) {
      config->noise = true;
    } else if (!last_arg && (!strcmp("-ar", argv[i]) || !strcmp("--bitrate", argv[i]))) {
      errno = 0;
      char *end;
      bitrate = strtol(argv[++i], &end, 10);
      if (errno != 0 || argv[i] == end) {
        fprintf(stderr, "Invalid bitrate\n");
        exit(1);
      }
    } else if (!last_arg && (!strcmp("-nb", argv[i]) || !strcmp("--noise-bandwidth", argv[i]))) {
      errno = 0;
      char *end;
      config->bandwidth = strtol(argv[++i], &end, 10);
      if (errno != 0 || argv[i] == end) {
        fprintf(stderr, "Invalid bandwidth\n");
        exit(1);
      }
    } else if (!last_arg && (!strcmp("-bg", argv[i]) || !strcmp("--bgcolor", argv[i]))) {
      errno = 0;
      char *end;
      config->bgcolor = argv[++i];
    } else if (!last_arg && (!strcmp("-fg", argv[i]) || !strcmp("--fgcolor", argv[i]))) {
      errno = 0;
      char *end;
      config->fgcolor = argv[++i];
    } else if (!last_arg && (!strcmp("-hl", argv[i]) || !strcmp("--hlcolor", argv[i]))) {
      errno = 0;
      char *end;
      config->hlcolor = argv[++i];
    } else if (!strcmp("-t", argv[i]) || !strcmp("--showtitle", argv[i])) {
      config->showtitle = true;
    } else if (!last_arg && (!strcmp("-td", argv[i]) || !strcmp("--titleduration", argv[i]))) {
      errno = 0;
      char *end;
      config->title_duration_ms = strtol(argv[++i], &end, 10);
      if (errno != 0 || argv[i] == end) {
        fprintf(stderr, "Invalid title duration\n");
        exit(1);
      }
    }
  }

  if (config->normal_wpm <= 0) {
    config->normal_wpm = DEFAULT_WPM;
    fprintf(stderr, "WPM speed not informed. Using default speed of %d wpm\n",
        config->normal_wpm);
  }

  if (!farnsworth_informed) {
    config->farnsworth_wpm = config->normal_wpm;
  } else {
    if (config->farnsworth_wpm > config->normal_wpm) {
      fprintf(stderr, "Farnsworth speed can't be greater than normal speed. "
          "Making farnsworth speed equal to normal one (%d wpm)\n",
          config->normal_wpm);
      config->farnsworth_wpm = config->normal_wpm;
    }
  }

  // the formula below is derived by the characteristics of the standard "PARIS" word in
  // morse code. Further details: https://morsecode.world/international/timing.html
  config->normal_unit_ms = 60000. / (50 * config->normal_wpm);
  config->farnsworth_unit_ms = 60000. / (50 * config->farnsworth_wpm);

  config->read_stdin = !inputfile_was_informed;

  config->bitrate = bitrate;
  config->freq = freq;

  printConfig(config);

  return config;
}

void printConfig(config_t *config) {
  printf("speed: %d\n", config->normal_wpm);
  printf("farnsworth: %d\n", config->farnsworth_wpm);
  printf("normal unit duration in ms: %7.5f\n", config->normal_unit_ms);
  printf("farns unit duration in ms: %7.5f\n", config->farnsworth_unit_ms);
}
