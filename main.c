#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "config.h"
#include "util.h"
#include "parser.h"
#include "token.h"
#include "audio.h"
#include "video.h"

const char *createTempAudioFile() {
  static char template[] = "audioXXXXXX";
  int f = mkstemp(template);
  if (f == -1) {
    fprintf(stderr, "Could not create the temporary audio file\n");
    exit(1);
  }
  close(f);
  return template;
}

const char *createTempVideoFile() {
  static char template[] = "videoXXXXXX";
  int f = mkstemp(template);
  if (f == -1) {
    fprintf(stderr, "Could not create the temporary video file\n");
    exit(1);
  }
  close(f);
  return template;
}

int main(int argc, char *argv[]) {
  const char *audio_filename, *video_filename;

  config_t *config = parseConfig(argc, argv);
  token_bag_t *token_bag = parseInput(config);

  audio_filename = createTempAudioFile();
  video_filename = createTempVideoFile();

  writeAudio(config, token_bag, audio_filename);
  writeVideo(config, token_bag, video_filename);

  free(config);

  // remote temporary files unless the current config says not to do so

  // validate/sanitize output file name, for security reasons
  // run the final ffmpeg command using system()
  //
  // #ffmpeg -y -i /tmp/morse.mp4 -i /tmp/morse.mp3 -c:a copy -c:v copy /tmp/tnc.mp4
  return 0;
}
