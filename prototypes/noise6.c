// compile with: gcc -g minfft.c noise6.c -onoise6 -lm
/* Factored discrete Fourier transform, or FFT, and its inverse iFFT */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <complex.h>

#include "minfft.h"

//#define q	3		/* for 2^3 points */
//#define N	(1<<q)		/* N-point FFT, iFFT */

#define N (32768)

#ifndef PI
# define PI	3.14159265358979323846264338327950288
#endif


void writeSound(float sample, FILE *pipeout) {
  size_t written = fwrite(&sample, sizeof(sample), 1, pipeout);
  if (written < 1) {
    fprintf(stderr, "Error send audio bytes to ffmpeg\n");
    exit(1);
  }
}


void writeAudio(complex double *samples, int nb_samples, int bitrate) {
  FILE *pipeout = NULL;
  const char cmd_fmt[] = "ffmpeg -y -f f32le -ar %d -ac 1 -i - -f mp3 /tmp/noise.mp3";
  int cmd_bufsz;
  char *ffmpeg_cmd;

  cmd_bufsz = snprintf(NULL, 0, cmd_fmt, bitrate);
  cmd_bufsz++;
  ffmpeg_cmd = (char *) malloc(cmd_bufsz + 1);
  snprintf(ffmpeg_cmd, cmd_bufsz, cmd_fmt, bitrate);

  // TODO consider changing to `run_and_capture`, in `util.h`
  pipeout = popen(ffmpeg_cmd, "w");
  if (pipeout == NULL) {
    fprintf(stderr, "Could not open audio file\n");
    exit(1);
  }

  for (int i = 0; i < nb_samples; i++) {
    writeSound(creal(samples[i]) * 300., pipeout);
  }

  fclose(pipeout);
}

void writeFile(const complex double *samples, int num_samples, const char *filename) {
  FILE *f = fopen(filename, "w");
  for (int i = 0; i < num_samples; ++i) {
    fprintf(f, "%10.5f,%10.5f\n", creal(samples[i]), cimag(samples[i]));
  }
  fclose(f);
}

void fill_conjugates(complex double *samples, int num_samples) {
  int i = 1, j = num_samples - 1;
  int np = (num_samples - 1) / 2;
  while (i < np) {
    samples[j--] = creal(samples[i]) - cimag(samples[i]) * I;
    i++;
  }
}

int
main(void)
{
  //complex v[N], v1[N], v2[N], scratch[N];

  int num_samples = 22050;
  int sample_rate = 22050;
  int min_freq = 400;
  int max_freq = 600;
  //complex *samples, *temp;
  minfft_cmpl samples[N], output[N];
  minfft_aux *temp;

  temp = minfft_mkaux_dft_1d(N);

/*
f = [0, 1, ...,   n/2-1,     -n/2, ..., -1] / (d*n)   if n is even
f = [0, 1, ..., (n-1)/2, -(n-1)/2, ..., -1] / (d*n)   if n is odd
*/
  int i = 0, factor;
  float real_part = 0.;

  if (num_samples % 2 == 0) {
    // num_samples is even
    factor = -num_samples / 2;
    while (i <= num_samples / 2 - 1) {
      real_part = num_samples / 2 + factor;
      real_part /= ((float)num_samples / sample_rate);
      //samples[i].Re = real_part;
      samples[i] = ((real_part >= min_freq && real_part <= max_freq) ? 1. : 0.) + 0 * I;
      factor++;
      i++;
    }
    while (i < num_samples) {
      real_part = -num_samples / 2 + factor;
      real_part /= ((float)num_samples / sample_rate);
      //samples[i].Re = real_part;
      samples[i] = ((real_part >= min_freq && real_part <= max_freq) ? 1. : 0.) + 0 * I;
      factor++;
      i++;
    }
  } else {
    // num_samples is odd
    factor = -(num_samples - 1) / 2;
    while (i <= (num_samples - 1) / 2) {
      real_part = (num_samples - 1) / 2 + factor;
      real_part /= ((float)num_samples / sample_rate);
      //samples[i].Re = real_part;
      samples[i] = ((real_part >= min_freq && real_part <= max_freq) ? 1. : 0.) + 0 * I;
      factor++;
      i++;
    }
    while (i < num_samples) {
      real_part = -(num_samples - 1) / 2 + factor - 1;
      real_part /= ((float)num_samples / sample_rate);
      //samples[i].Re = real_part;
      samples[i] = ((real_part >= min_freq && real_part <= max_freq) ? 1. : 0.) + 0 * I;
      factor++;
      i++;
    }
  }

  for (int i = 0; i < num_samples; i++) {
    if (creal(samples[i]) < 1.) continue;
    /*float r = (float) rand() / (float) RAND_MAX;
    r *= 2 * PI;*/
    float r = PI * .25;
    samples[i] = cos(r) + sin(r) * I;
  }

  fill_conjugates(samples, num_samples);

  writeFile(samples, num_samples, "/tmp/noise6-input.csv");

  //ifft(samples, num_samples, temp);
  minfft_invdft(samples, output, temp);


  /*for (int i = 0; i < num_samples; ++i) {
    samples[i].Re /= num_samples;
    samples[i].Im /= num_samples;
  }*/

  writeFile(output, num_samples, "/tmp/noise6-output.csv");

  writeAudio(output, num_samples, sample_rate);

  minfft_dft(output, samples, temp);

  writeFile(samples, num_samples, "/tmp/noise6-reinput.csv");

  exit(EXIT_SUCCESS);
}

