#include "WProgram.h"

void initRectangular()
// precompute w as a rectangular window
{
  filter_correction=1.0;
  for (int n = 0; n < L; n++){
    w[n]=1;
  }
}

void initHamming()
// precompute w as a Hamming Window
{
  filter_correction=2.0; // not exactly sure if this is true..... should be something like the integral underneath the window divided by the interval length?
  for (int n = 0; n < L; n++){
    w[n]=0.54 - 0.46*cos(2*PI*n/(L-1));
  }
}

void initBlackman()
// precompute w as a Blackman Window
{
  filter_correction=3.0; 
  float a = 0.16;
  float a0 = (1 - a) / 2;
  float a1 = 0.5;
  float a2 = a / 2;
  for (int n = 0; n < L; n++){
    w[n] = a0 - a1*cos(2*PI*n/(L-1)) + a2*cos(4*PI*n/(L-1));
  }
}
