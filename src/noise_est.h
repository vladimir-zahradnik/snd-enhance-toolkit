/********************************************************************
 function: Noise Estimation
 contains: hirsch, vad
 last mod: $Id: lpc.h 7187 2004-07-20 07:24:27Z xiphmont $

 ********************************************************************/

#ifndef HAVE_NOISE_EST_H
#define HAVE_NOISE_EST_H

/* hirsch noise estimation */
extern void hirsch_estimation(double * ns_ps, int fft_size);

/* simple VAD noise estimation */
extern void vad_estimation(double * ns_ps, int fft_size, double SNRseg);

#endif
