/********************************************************************
 function: Noise Estimation
 contains: hirsch, vad
 last mod: $Id: lpc.h 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#ifndef HAVE_SND_ENHANCE_H
#define HAVE_SND_ENHANCE_H

#include <fftw3.h>
#include "common.h"
#include "noise_est.h"

typedef void (*snd_enh_func_t)(double *fft_data, int fft_size, fftw_plan fft_forw, fftw_plan fft_back,
                               noise_est_func_t noise_estimation, int datalen, int samplerate);

extern snd_enh_func_t parse_snd_enhance_type(const char *name, bool verbose);

extern char *get_snd_enhance_name(const char *name);

/* Sound Enhancement Algorithms */
extern void snd_enhance_specsub(double *fft_data, int fft_size, fftw_plan fft_forw, fftw_plan fft_back,
                                noise_est_func_t noise_estimation, int datalen, int samplerate);

extern void snd_enhance_mmse(double *fft_data, int fft_size, fftw_plan fft_forw, fftw_plan fft_back,
                             noise_est_func_t noise_estimation, int datalen, int samplerate);

extern void snd_enhance_wiener_as(double *fft_data, int fft_size, fftw_plan fft_forw, fftw_plan fft_back,
                                  noise_est_func_t noise_estimation, int datalen, int samplerate);

extern void snd_enhance_wiener_iter(double *fft_data, int fft_size, fftw_plan fft_forw, fftw_plan fft_back,
                                    noise_est_func_t noise_estimation, int datalen, int samplerate);

extern void snd_enhance_residual(double *fft_data, int fft_size, fftw_plan fft_forw, fftw_plan fft_back,
                                 noise_est_func_t noise_estimation, int datalen, int samplerate);

#endif
