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

#include "common.h"
#include <string.h>
#include <math.h>
#include "noise_est.h"

/* static arrays used by noise estimation algorithms */
static double P [FFT_MAX];
static double noise_ps [FFT_MAX];

/* hirsch noise estimation */
void hirsch_estimation(double * ns_ps, int fft_size)
{
  /* initialize static variables */
  static int n = 1; /* check number of calls */
  static double as = 0.85;
  static double beta = 1.5;
  static double omin = 1.5;
  
  int i;
  
  if (n == 1) {
        memcpy((void *) P, (void *) ns_ps, sizeof(* P) * fft_size);
        memcpy((void *) noise_ps, (void *) ns_ps, sizeof(* noise_ps) * fft_size);
  }
  else {
        for (i = 0; i < fft_size; i++) {
             P [i] = as * P [i] + (1 - as) * ns_ps [i];
              if (P [i] < beta * noise_ps [i])
                 noise_ps [i] = as * noise_ps [i] + (1 - as) * P [i];
        }
  }
  
  n++;
  memcpy((void *) ns_ps, (void *) noise_ps, sizeof(* ns_ps) * fft_size);
}

/* simple VAD noise estimation */
void vad_estimation(double * ns_ps, int fft_size, double SNRseg)
{
   const int nf_sabsent = 6; /* speech absent frames */
   double thres = 3.0;
   double G = 0.9;
   static int frame = 0;
   
   int i;
   
   if (frame < nf_sabsent) {
     for (i = 0; i < fft_size; i++) {
        noise_ps [i] = noise_ps [i] + ns_ps [i] / nf_sabsent; 
     }
   }
   else {
     /* --- implement a simple VAD detector -------------- */
     if (SNRseg < thres) {
       for (i = 0; i < fft_size; i++) {
         noise_ps [i] = G * noise_ps [i] + (1 - G) * ns_ps [i];
       }
     }
     
     memcpy((void *) ns_ps, (void *) noise_ps, sizeof(* ns_ps) * fft_size);
   }
   
   frame++;
}
