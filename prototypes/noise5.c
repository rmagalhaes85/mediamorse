// compile with: gcc -g noise5.c -onoise5 -lm
/* Factored discrete Fourier transform, or FFT, and its inverse iFFT */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


//#define q	3		/* for 2^3 points */
//#define N	(1<<q)		/* N-point FFT, iFFT */

typedef float real;
typedef struct{real Re; real Im;} complex;

#ifndef PI
# define PI	3.14159265358979323846264338327950288
#endif


/* Print a vector of complexes as ordered pairs. */
static void
print_vector(
	     const char *title,
	     complex *x,
	     int n)
{
  int i;
  printf("%s (dim=%d):", title, n);
  for(i=0; i<n; i++ ) printf(" %6.3f,%6.3f ", x[i].Re,x[i].Im);
  putchar('\n');
  return;
}


/*
   fft(v,N):
   [0] If N==1 then return.
   [1] For k = 0 to N/2-1, let ve[k] = v[2*k]
   [2] Compute fft(ve, N/2);
   [3] For k = 0 to N/2-1, let vo[k] = v[2*k+1]
   [4] Compute fft(vo, N/2);
   [5] For m = 0 to N/2-1, do [6] through [9]
   [6]   Let w.re = cos(2*PI*m/N)
   [7]   Let w.im = -sin(2*PI*m/N)
   [8]   Let v[m] = ve[m] + w*vo[m]
   [9]   Let v[m+N/2] = ve[m] - w*vo[m]
 */
void
fft( complex *v, int n, complex *tmp )
{
  if(n>1) {			/* otherwise, do nothing and return */
    int k,m;    complex z, w, *vo, *ve;
    ve = tmp; vo = tmp+n/2;
    for(k=0; k<n/2; k++) {
      ve[k] = v[2*k];
      vo[k] = v[2*k+1];
    }
    fft( ve, n/2, v );		/* FFT on even-indexed elements of v[] */
    fft( vo, n/2, v );		/* FFT on odd-indexed elements of v[] */
    for(m=0; m<n/2; m++) {
      w.Re = cos(2*PI*m/(double)n);
      w.Im = -sin(2*PI*m/(double)n);
      z.Re = w.Re*vo[m].Re - w.Im*vo[m].Im;	/* Re(w*vo[m]) */
      z.Im = w.Re*vo[m].Im + w.Im*vo[m].Re;	/* Im(w*vo[m]) */
      v[  m  ].Re = ve[m].Re + z.Re;
      v[  m  ].Im = ve[m].Im + z.Im;
      v[m+n/2].Re = ve[m].Re - z.Re;
      v[m+n/2].Im = ve[m].Im - z.Im;
    }
  }
  return;
}

/*
   ifft(v,N):
   [0] If N==1 then return.
   [1] For k = 0 to N/2-1, let ve[k] = v[2*k]
   [2] Compute ifft(ve, N/2);
   [3] For k = 0 to N/2-1, let vo[k] = v[2*k+1]
   [4] Compute ifft(vo, N/2);
   [5] For m = 0 to N/2-1, do [6] through [9]
   [6]   Let w.re = cos(2*PI*m/N)
   [7]   Let w.im = sin(2*PI*m/N)
   [8]   Let v[m] = ve[m] + w*vo[m]
   [9]   Let v[m+N/2] = ve[m] - w*vo[m]
 */
void
ifft( complex *v, int n, complex *tmp )
{
  if(n>1) {			/* otherwise, do nothing and return */
    int k,m;    complex z, w, *vo, *ve;
    ve = tmp; vo = tmp+n/2;
    for(k=0; k<n/2; k++) {
      ve[k] = v[2*k];
      vo[k] = v[2*k+1];
    }
    ifft( ve, n/2, v );		/* FFT on even-indexed elements of v[] */
    ifft( vo, n/2, v );		/* FFT on odd-indexed elements of v[] */
    for(m=0; m<n/2; m++) {
      w.Re = cos(2*PI*m/(double)n);
      w.Im = sin(2*PI*m/(double)n);
      z.Re = w.Re*vo[m].Re - w.Im*vo[m].Im;	/* Re(w*vo[m]) */
      z.Im = w.Re*vo[m].Im + w.Im*vo[m].Re;	/* Im(w*vo[m]) */
      v[  m  ].Re = ve[m].Re + z.Re;
      v[  m  ].Im = ve[m].Im + z.Im;
      v[m+n/2].Re = ve[m].Re - z.Re;
      v[m+n/2].Im = ve[m].Im - z.Im;
    }
  }
  return;
}

void writeSound(float sample, FILE *pipeout) {
  size_t written = fwrite(&sample, sizeof(sample), 1, pipeout);
  if (written < 1) {
    fprintf(stderr, "Error send audio bytes to ffmpeg\n");
    exit(1);
  }
}


void writeAudio(complex *samples, int nb_samples, int bitrate) {
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
    writeSound(samples[i].Re * 500., pipeout);
  }

  fclose(pipeout);
}

void writeFile(const complex *samples, int num_samples, const char *filename) {
  FILE *f = fopen(filename, "w");
  for (int i = 0; i < num_samples; ++i) {
    fprintf(f, "%10.5f,%10.5f\n", samples[i].Re, samples[i].Im);
  }
  fclose(f);
}

void fill_conjugates(complex *samples, int num_samples) {
  int i = 1, j = num_samples - 1;
  int np = (num_samples - 1) / 2;
  while (i < np) {
    samples[j--] = (complex) { .Re = samples[i].Re, .Im = -samples[i].Im };
    i++;
  }
}

int getRandomNumber(int min, int max) {
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd == -1) {
    perror("Failed to open /dev/urandom");
    exit(EXIT_FAILURE);
  }

  int random_number;
  ssize_t bytes_read = read(fd, &random_number, sizeof(random_number));
  if (bytes_read != sizeof(random_number)) {
    perror("Failed to read from /dev/urandom");
    close(fd);
    exit(EXIT_FAILURE);
  }
  close(fd);

  // Scale to the desired range
  return (random_number % (max - min + 1)) + min;
}

double* read_doubles_from_file(const char* filename, size_t* out_count) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    size_t capacity = 10;
    size_t count = 0;
    double* numbers = malloc(capacity * sizeof(double));
    if (!numbers) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    double value;
    while (fscanf(file, "%lf", &value) == 1) {
        if (count >= capacity) {
            capacity *= 2;
            double* temp = realloc(numbers, capacity * sizeof(double));
            if (!temp) {
                perror("Memory reallocation failed");
                free(numbers);
                fclose(file);
                return NULL;
            }
            numbers = temp;
        }
        numbers[count++] = value;
    }

    fclose(file);
    *out_count = count;
    return numbers;
}



int
main(void)
{
  //complex v[N], v1[N], v2[N], scratch[N];

  int num_samples = 22050;
  int sample_rate = 22050;
  int min_freq = 400;
  int max_freq = 600;
  complex *samples, *temp;

  samples = (complex *) malloc(sizeof(complex) * num_samples);
  temp = (complex *) malloc(sizeof(complex) * num_samples);

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
      real_part = num_samples / 2. + factor;
      real_part /= ((float)num_samples / sample_rate);
      //samples[i].Re = real_part;
      samples[i].Re = (real_part >= min_freq && real_part <= max_freq) ? 1. : 0.;
      factor++;
      i++;
    }
    while (i < num_samples) {
      real_part = -num_samples / 2. + factor;
      real_part /= ((float)num_samples / sample_rate);
      //samples[i].Re = real_part;
      samples[i].Re = (real_part >= min_freq && real_part <= max_freq) ? 1. : 0.;
      factor++;
      i++;
    }
  } else {
    // num_samples is odd
    factor = -(num_samples - 1) / 2;
    while (i <= (num_samples - 1) / 2) {
      real_part = (num_samples - 1) / 2. + factor;
      real_part /= ((float)num_samples / sample_rate);
      //samples[i].Re = real_part;
      samples[i].Re = (real_part >= min_freq && real_part <= max_freq) ? 1. : 0.;
      factor++;
      i++;
    }
    while (i < num_samples) {
      real_part = -(num_samples - 1) / 2. + factor - 1;
      real_part /= ((float)num_samples / sample_rate);
      //samples[i].Re = real_part;
      samples[i].Re = (real_part >= min_freq && real_part <= max_freq) ? 1. : 0.;
      factor++;
      i++;
    }
  }

  size_t randoms_count = 0;
  double *randoms = read_doubles_from_file("random_numbers.txt", &randoms_count);
  if ((randoms_count * 2) < (num_samples - 2)) {
    fprintf(stderr,
        "There are %d nums in the random numbers file,"
        " not enough random numbers in the file\n", randoms_count);
    exit(1);
  }

  for (int i = 1; i <= ((num_samples - 1) / 2) + 1; i++) {
    if (samples[i].Re < 1.) continue;
    //float r = abs(getRandomNumber(0, 10000)) / 10000.;
    //float r = (float) rand() / (float) RAND_MAX;
    double r = randoms[i - 1];
    r *= 2 * PI;
    //float r = PI * .25;
    samples[i].Re = cos(r);
    samples[i].Im = sin(r);
  }

  fill_conjugates(samples, num_samples);

  writeFile(samples, num_samples, "/tmp/noise5-input.csv");

  ifft(samples, num_samples, temp);

  for (int i = 0; i < num_samples; ++i) {
    samples[i].Re /= num_samples;
    samples[i].Im /= num_samples;
  }

  writeFile(samples, num_samples, "/tmp/noise5-output.csv");

  writeAudio(samples, num_samples, sample_rate);

  fft(samples, num_samples, temp);

  writeFile(samples, num_samples, "/tmp/noise5-reinput.csv");

  exit(EXIT_SUCCESS);
}

