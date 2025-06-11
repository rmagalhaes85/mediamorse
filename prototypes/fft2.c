// compile with: gcc -g minfft.c noise5.c -onoise5 -lm

#include <complex.h>

#include "minfft.h"


#define N (8)

int main(void) {
  minfft_cmpl x[N],y[N]; // input and output buffers
  minfft_aux *a; // aux data
  // prepare aux data
  a=minfft_mkaux_dft_1d(N);

//  v2[0].Re = 0.;
//  v2[0].Im = 0.;
x[0] = 0;
//  v2[1].Re = 0.125;
//  v2[1].Im = 0.;
x[1] = .125;
//  v2[2].Re = 0.25;
//  v2[2].Im = 0.;
x[2] = .25;
//  v2[3].Re = 0.375;
//  v2[3].Im = 0.;
x[3] = .375;
//  v2[4].Re = -.5;
//  v2[4].Im = 0.;
x[4] = -.5;
//  v2[5].Re = -.375;
//  v2[5].Im = 0.;
x[5] = -.375;
//  v2[6].Re = -.25;
//  v2[6].Im = 0.;
x[6] = -.25;
//  v2[7].Re = -.125;
//  v2[7].Im = 0.;
x[7] = -.125;

  // do transforms
  minfft_dft(x,y,a);
  minfft_invdft(y,x,a);
  // free aux data
  minfft_free_aux(a);
  return 0;
}
