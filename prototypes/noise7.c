// compile with: gcc -g noise7.c -o noise7 -lm
#include <stdlib.h>
#include <stdio.h>
#include <complex.h>
#include <math.h>

int main() {
  int duration_s = 5;
  int bitrate = 22050;
  int num_samples = duration_s * bitrate;
  float center_freq = 500;
  float bandwidth = 100;
  float min_freq = center_freq - bandwidth / 2;
  float max_freq = center_freq + bandwidth / 2;
  int num_oscillators = 240;
  complex double *oscs;
  float *freqs;
  // ffmpeg pipe
  const char cmd_fmt[] = "ffmpeg -v 0 -y -f f64le -ar %d -ac 1 -i - -f mp3 %s";
  const char audio_filename[] = "/tmp/noise.mp3";
  int cmd_bufsz;
  char *ffmpeg_cmd;
  FILE *pipeout = NULL;

  oscs = (complex double *) calloc(sizeof(complex double), num_oscillators);
  freqs = (float *) calloc(sizeof(float), num_oscillators);

  // compute output frequencies
  if (num_oscillators == 1) {
    freqs[0] = center_freq;
  } else {
    float cur_freq = min_freq;
    float freq_iter = bandwidth / (num_oscillators - 1);
    int i = 0;
    while (cur_freq <= max_freq) {
      freqs[i++] = cur_freq;
      cur_freq += freq_iter;
    }
  }

  // initialize oscillators with random numbers
  for (int i = 0; i < num_oscillators; ++i) {
    float r = ((float) rand() / (float) RAND_MAX) * 2 * M_PI; // radians
    oscs[i] = cos(r) + sin(r) * I;
  }

  // initialize audio output pipe
  cmd_bufsz = snprintf(NULL, 0, cmd_fmt, bitrate, audio_filename);
  cmd_bufsz++;
  ffmpeg_cmd = (char *) malloc(cmd_bufsz + 1);
  snprintf(ffmpeg_cmd, cmd_bufsz, cmd_fmt, bitrate, audio_filename);
  pipeout = popen(ffmpeg_cmd, "w");

  // rotate oscillators and write their real parts to the audio file
  for (int i = 0; i < num_samples; ++i) {
    double real_output = 0.;
    for (int j = 0; j < num_oscillators; ++j) {
      //*osc *= cexpf(config->freq * (2.0 * M_PI / config->bitrate) * I);
      oscs[j] *= cexpf(freqs[j] * (2. * M_PI / bitrate) * I);
      real_output += creal(oscs[j]) / (num_oscillators * 0.5);
    }
    size_t written = fwrite(&real_output, sizeof(real_output), 1, pipeout);
    if (written < 1) {
      fprintf(stderr, "Error send audio bytes to ffmpeg\n");
      exit(1);
    }
  }

  // close audio pipe
  fclose(pipeout);
}
