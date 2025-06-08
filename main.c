#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "config.h"
#include "util.h"
#include "parser.h"
#include "token.h"
#include "audio.h"
#include "video.h"

char *createTempFile(const char *ext) {
  char *basename = tmpnam(NULL);
  if (basename == NULL) {
    fprintf(stderr, "Could not generate a temporary file\n");
    exit(1);
  }
  if (ext == NULL || strlen(ext) == 0) {
    // condition above don't cover `ext`s consisting of spaces only
    return strdup(basename);
  }
  // allocates a buffer with enough space to store the file extension
  char *tempname = (char *) malloc(strlen(basename) + strlen(ext) + 1);
  sprintf(tempname, "%s.%s", basename, ext);
  return tempname;
}

int main(int argc, char *argv[]) {
  const char *audio_filename, *video_filename;

  config_t *config = parseConfig(argc, argv);
  token_bag_t *token_bag = parseInput(config);

  audio_filename = createTempFile("mp3");
  video_filename = createTempFile("mp4");

  writeAudio(config, token_bag, audio_filename);
  writeVideo(config, token_bag, video_filename);

  free(config);

  // remove temporary files unless the current config says not to do so

  // validate/sanitize output file name, for security reasons
  // run the final ffmpeg command using system()
  //
  // #ffmpeg -y -i /tmp/morse.mp4 -i /tmp/morse.mp3 -c:a copy -c:v copy /tmp/tnc.mp4
  char *const ffmpeg_args[] = { "ffmpeg", "-v", "0", "-y",
    "-i", (char * const) audio_filename,
    "-i", (char * const) video_filename,
    "-c:a", "copy", "-c:v", "copy", config->output_filename,
    NULL};
  run_command("ffmpeg", ffmpeg_args);

  if (config->keep_tmp_files) {
    fprintf(stderr, "Keeping temporary audio file: %s\n", audio_filename);
    fprintf(stderr, "Keeping temporary video file: %s\n", video_filename);
  } else {
    remove(audio_filename);
    remove(video_filename);
  }

  return 0;
}
