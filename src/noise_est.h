/********************************************************************
 function: Noise Estimation
 contains: hirsch, vad
 last mod: $Id: lpc.h 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#ifndef HAVE_NOISE_EST_H
#define HAVE_NOISE_EST_H

#include "common.h"

typedef double (* noise_est_func_t) (const double * ns_ps, int fft_size, double * noise_ps, double SNRseg, int samplerate);

extern noise_est_func_t parse_noise_est_type (const char * name, bool verbose);

extern char * get_noise_est_name (const char * name);

/* Noise estimation algorithms */

extern double hirsch_estimation(const double * ns_ps, int fft_size, double * noise_ps, double SNRseg, int samplerate);

extern double vad_estimation(const double * ns_ps, int fft_size, double * noise_ps, double SNRseg, int samplerate);

extern double doblinger_estimation(const double * ns_ps, int fft_size, double * noise_ps, double SNRseg, int samplerate);

extern double mcra_estimation(const double * ns_ps, int fft_size, double * noise_ps, double SNRseg, int samplerate);

extern double mcra2_estimation(const double * ns_ps, int fft_size, double * noise_ps, double SNRseg, int samplerate);

#endif
