#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>
#include <math.h>

#include "audio.h"
#include "util.h"

void writeSound(float complex *osc, FILE *pipeout, const config_t *config,
    int duration_ms) {
  float output = 0.;
  int nb_samples = (duration_ms / 1000.) * config->bitrate;

  for (int i = 0; i < nb_samples; ++i) {
    *osc *= cexpf(config->freq * (2.0 * M_PI / config->bitrate) * I);
    // TODO make audio volume configurable
    float output = crealf(*osc) * 0.5;
    size_t written = fwrite(&output, sizeof(output), 1, pipeout);
    if (written < 1) {
      fprintf(stderr, "Error send audio bytes to ffmpeg\n");
      exit(1);
    }
  }
}

void writeSilence(float complex *osc, FILE *pipeout, const config_t *config,
    int duration_ms) {
  float output = 0.;
  int nb_samples = (duration_ms / 1000.) * config->bitrate;

  *osc = I;
  for (int i = 0; i < nb_samples; ++i) {
    size_t written = fwrite(&output, sizeof(output), 1, pipeout);
    if (written < 1) {
      fprintf(stderr, "Error send audio bytes to ffmpeg\n");
      exit(1);
    }
  }

}

void writeAudio(const config_t *config, const token_bag_t *token_bag,
    const char *audio_filename) {
  FILE *pipeout = NULL;
  const char cmd_fmt[] = "ffmpeg -v 0 -y -f f32le -ar %d -ac 1 -i - -f mp3 %s";
  int cmd_bufsz;
  char *ffmpeg_cmd;
  // audio
  float complex osc;
  // tokens/glyphs
  token_t *token = NULL;
  glyph_t *glyph = NULL;

  cmd_bufsz = snprintf(NULL, 0, cmd_fmt, config->bitrate, audio_filename);
  cmd_bufsz++;
  ffmpeg_cmd = (char *) fmalloc(cmd_bufsz + 1);
  snprintf(ffmpeg_cmd, cmd_bufsz, cmd_fmt, config->bitrate, audio_filename);

  if (!is_valid_filename(audio_filename)) {
    fprintf(stderr, "Invalid audio file name\n");
    exit(1);
  }

  // TODO consider changing to `run_and_capture`, in `util.h`
  pipeout = popen(ffmpeg_cmd, "w");
  if (pipeout == NULL) {
    fprintf(stderr, "Could not open audio file\n");
    exit(1);
  }

  osc = I;

  token = token_bag->token_head;

  while (token != NULL) {
    glyph = token->glyph_head;

    while (glyph != NULL) {
      const char *morse = glyph->morse;
      int morse_len = strlen(morse);
      for (int i = 0; i < morse_len; ++i) {
        int duration_ms = config->normal_unit_ms * (morse[i] == '.' ? 1 : 3);
        int is_last = i == morse_len - 1;

        writeSound(&osc, pipeout, config, duration_ms);
        if (!is_last) {
          // intra-character space of 1 unit
          writeSilence(&osc, pipeout, config, config->normal_unit_ms);
        }
      }

      glyph = glyph->next;

      if (glyph == NULL) {
        // the word is finished. Output inter-word spacer. (Farnsworth will equal normal
        // durations when no farnsworth duration is specified
        writeSilence(&osc, pipeout, config, config->farnsworth_unit_ms * 7);
      } else {
        // there are still characters in the current word. Output the inter-character
        // spacer
        writeSilence(&osc, pipeout, config, config->farnsworth_unit_ms * 3);
      }
    }

    token = token->next;
  }

  fclose(pipeout);
}
