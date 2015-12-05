/********************************************************************
 function: Noise Estimation
 contains: hirsch, vad
 last mod: $Id: lpc.h 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#ifndef HAVE_NOISE_EST_H
#define HAVE_NOISE_EST_H

#include <fftw3.h>
#include <complex.h>

/* Spectral substraction */
extern void snd_enhance_specsub(double * buffer, int datalen, double complex * fft_in, double complex * fft_out,
                                fftw_plan fft_forw, fftw_plan fft_back, int fft_size);

#endif
