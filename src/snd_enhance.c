/*
** Copyright (C) 2011 Vladimir Zahradnik <vladimir.zahradnik@gmail.com>
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 or version 3 of the
** License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "common.h"
#include "snd_enhance.h"
#include "noise_est.h"

/* Required in spectral substraction algorithm */
static double berouti(double SNR); /* if alpha == 2 */

void snd_enhance_specsub(double * buffer, int datalen, double complex * fft_in, double complex * fft_out, fftw_plan fft_forw, fftw_plan fft_back, int fft_size)
{
  /* initialize variables */
  double * y_ps = alloca (sizeof(* y_ps) * fft_size); /* signal power spectrum */
  double * y_specang = alloca (sizeof(* y_specang) * fft_size); /* signal phase */
  double * noise_ps = alloca (sizeof(* noise_ps) * fft_size); /* noise power spectrum */
  double * sub_speech = alloca(sizeof(* sub_speech) * fft_size);
  double magn_sum, noise_sum, beta;
  static double SNRseg = 0;
  double floor = 0.002;
  int i;
  
  /* set input buffer for FFT to zero, otherwise components
        with index above nwind will be not zero valued */
  memset((void *) fft_in, 0, sizeof(*fft_in) * fft_size);
   
  for (i = 0; i < datalen; i++) {
       fft_in [i] =  buffer [i];
  }
   
   fftw_execute(fft_forw); /* FFT */
   
   for (i = 0; i < fft_size; i++) {
         y_ps [i] = pow(cabs(fft_out[i]), 2); /* power spectrum */
         y_specang [i] = carg(fft_out[i]); /* spectrum phase */
      }
     
     /* copy signal power spectrum to noise_ps - it is used for estimation */
        memcpy((void *) noise_ps, (void *) y_ps, sizeof(* y_ps) * fft_size);
      
    /* estimate noise power spectrum */
       hirsch_estimation(noise_ps, fft_size);
    
       for (i = 0; i < fft_size; i++) {
         if (i == 0) magn_sum = noise_sum = 0;
            magn_sum += y_ps [i];
            noise_sum += noise_ps [i];
        } /* temporal variables for computation norm of a vector */
   
       SNRseg = 10 * log10(magn_sum / noise_sum);
      
       beta = berouti(SNRseg);
    
      for (i = 0; i < fft_size; i++) {
        sub_speech[i] = y_ps[i] - beta * noise_ps[i];
        if( (sub_speech[i] - floor * noise_ps[i]) < 0 ) {
             sub_speech[i] = floor * noise_ps[i];
         }
      }
   
    /*  to ensure conjugate symmetry for real reconstruction */
    for (i = 0; i < fft_size/2; i++) {
         sub_speech[fft_size - i] = sub_speech[i];
    }
   
   /* enhanced signal */
     for (i = 0; i < fft_size; i++) {
        fft_out[i] = sqrt(sub_speech[i]) * (ccos(y_specang[i]) + I*csin(y_specang[i]));
     }
   
     fftw_execute(fft_back); /* IFFT */
}

/* Required in spectral substraction algorithm */
static double berouti(double SNR)
{
  if (SNR >= -5.0 && SNR <= 20) {
    return (4 - SNR * 3/20);
  }
  else {
    if (SNR < - 5.0) return 5.0;
    if (SNR > 20) return 1.0;
  }
  return 0;
}