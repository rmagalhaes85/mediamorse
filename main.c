#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "config.h"
#include "util.h"
#include "parser.h"
#include "audio.h"

const char *createTempAudioFile() {
  return "/tmp/morse.mp3";
}

const char *createTempVideoFile() {

}

int main(int argc, char *argv[]) {
  const char *audio_filename, *videoFileName;

  config_t *config = parseConfig(argc, argv);
  token_bag_t *token_bag = parseInput(config);

  audio_filename = createTempAudioFile();
  //videoFileName = createTempVideoFile();

  writeAudio(config, token_bag, audio_filename);
  //printTokens(token_bag);

  free(config);

  return 0;
}
