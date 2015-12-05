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

//#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <complex.h>
#include <fftw3.h>
#include "common.h"
#include "i18n.h"

/* sfx_mix_mono_read_double */
sf_count_t sfx_mix_mono_read_double (SNDFILE * file, double * data, sf_count_t datalen)
{
  SF_INFO info;

#if HAVE_SF_GET_INFO
  /*
  ** The function sf_get_info was in a number of 1.0.18 pre-releases but was removed
  ** before 1.0.18 final and replaced with the SFC_GET_CURRENT_SF_INFO command.
  */
        sf_get_info (file, &info);
#else
        sf_command (file, SFC_GET_CURRENT_SF_INFO, &info, sizeof (info));
#endif

        if (info.channels == 1)
             return sf_read_double (file, data, datalen);

        static double multi_data [2048];
        int k, ch, frames_read;
        sf_count_t dataout = 0;

        while (dataout < datalen) {
                int this_read;

                this_read = MIN (ARRAY_LEN (multi_data) / info.channels, datalen);

                frames_read = sf_readf_double (file, multi_data, this_read);
                if (frames_read == 0)
                      break;

                for (k = 0; k < frames_read; k++) {
                        double mix = 0.0;

                        for (ch = 0; ch < info.channels; ch++)
                               mix += multi_data [k * info.channels + ch];
                        data [dataout + k] = mix / info.channels;
                 };

                 dataout += this_read;
          };

         return dataout;
}

/* separate_channels */
int separate_channels_double (double * multi_data, double * single_data, int frames, int channels, int channel_number)
{
   int k;
   
   if (channel_number > channels) {
        fprintf(stderr, _("This recording has only %u channels."), channels);
        exit (1);
   }
   
   for (k = 0; k < frames; k++)
         single_data [k] = multi_data [k * channels + channel_number];

   return 0;
}

/* combine_channels_double */
int combine_channels_double (double * multi_data, double * single_data, int frames, int channels, int channel_number)
{
   int k;
   
   if (channel_number > channels) {
        fprintf(stderr, _("This recording has only %u channels."), channels);
        exit (1);
    }
   
    for (k = 0; k < frames; k++)
          multi_data [k * channels + channel_number] = single_data [k];

   return 0;
}

/* enhance audio file */
int enhance_audio (SNDFILE * input_file, SNDFILE * output_file, const char * window_type, int fft_size, int window_size, int overlap, bool downmix)
{
  SF_INFO info;
  sf_count_t (* ptr_read_dbl)() = sf_readf_double;
  int noverlap, nslide, channels, ch, i;
  sf_count_t count, frames_read = 0;
  double * multi_data, * prev_multi_data, * buffer, * window, * enhanced_multi_data;
  double * enhanced_prev;
  double winGain; /* normalization gain */
  
  /* initialize FFT */
  //double complex * fft_in = (double complex *) fftw_malloc(sizeof(* fft_in) * fft_size);
  //double complex * fft_out = (double complex *) fftw_malloc(sizeof(* fft_out) * fft_size);
  double complex fft_in[FFT_MAX];
  double complex fft_out[FFT_MAX];
  
  /* create plan for FFT and IFFT transform */
  fftw_plan fft_forw = fftw_plan_dft_1d(fft_size, fft_in, fft_out, FFTW_FORWARD, FFTW_MEASURE);
  fftw_plan fft_back = fftw_plan_dft_1d(fft_size, fft_out, fft_in, FFTW_BACKWARD, FFTW_MEASURE);
  
  #if HAVE_SF_GET_INFO
       /*
       **  The function sf_get_info was in a number of 1.0.18 pre-releases but was removed
       **  before 1.0.18 final and replaced with the SFC_GET_CURRENT_SF_INFO command.
       */
       sf_get_info (input_file, &info);
  #else
       sf_command (input_file, SFC_GET_CURRENT_SF_INFO, &info, sizeof (info));
  #endif
   
   if (downmix == true) {
     /* if downmix is enabled, use sfx_mix_mono_read_double */
     channels = 1;
     ptr_read_dbl = sfx_mix_mono_read_double;
   }
   else {
     channels = info.channels;
   }
       
   noverlap = floor(window_size * overlap / 100);
   nslide = window_size - noverlap;
   
   /* input buffers */
   multi_data = init_buffer_dbl(window_size * channels);
   prev_multi_data = init_buffer_dbl(noverlap * channels);
   
   /* output buffers */
   buffer = init_buffer_dbl(window_size);
   window = init_buffer_dbl(window_size);
   enhanced_prev = init_buffer_dbl(nslide); /* a priori enhanced data */
   enhanced_multi_data = init_buffer_dbl(nslide * channels);
   
   /* calculate window */
   winGain = calc_window(window, window_size, window_type);
   winGain = nslide / winGain;
   
   do {
      if(frames_read == 0) {
        if ((count = ptr_read_dbl (input_file, multi_data, window_size)) <= 0)
             exit (1);
        memcpy((void *) prev_multi_data, (void *) (multi_data + nslide * channels), sizeof(* multi_data) * noverlap * channels);
      }
      else {
        count = ptr_read_dbl (input_file, (multi_data + noverlap * channels), nslide);
        memcpy((void *) multi_data, (void *) prev_multi_data, sizeof(* prev_multi_data) * noverlap * channels);
        memcpy((void *) prev_multi_data, (void *) (multi_data + nslide * channels), sizeof(* multi_data) * noverlap * channels);
      }
        
        frames_read += count;

        for (ch = 0; ch < channels; ch++) {
              separate_channels_double (multi_data, buffer, window_size, channels, ch);
      
             /* apply window */
             multiply_arrays_dbl(buffer, window, buffer, window_size);
     
             snd_enhance_specsub(buffer, window_size, fft_in, fft_out, fft_forw, fft_back, fft_size);
     
            /* Add-and-Overlap */
            for (i = 0; i < nslide; i++) {
               buffer[i] = winGain * (creal(fft_in[i])/fft_size + enhanced_prev[i]);
            }
    
           for (i = 0; i < nslide; i++) {
               enhanced_prev[i] = creal(fft_in[i+noverlap])/fft_size;
           }
     
           combine_channels_double (enhanced_multi_data, buffer, nslide, channels, ch);
       }
        
        sf_writef_double (output_file, enhanced_multi_data, nslide);
   } while (count > 0);
   
   free (multi_data);
   free (prev_multi_data);
   free (enhanced_prev);
   free (enhanced_multi_data);
   free (buffer);
   free (window);
   
   /* destroy FFT plan and free memory */
   fftw_destroy_plan (fft_forw);
   fftw_destroy_plan (fft_back);
   //fftw_free (fft_in);
   //fftw_free (fft_out);
       
  return 0;
}

double * init_buffer_dbl(size_t size)
{
  double * ptr = (double *) malloc (sizeof(* ptr) * size);
  
  if (ptr == NULL) {
     printf(_("\nError: malloc failed: %s\n"), strerror(errno));
     exit (1);
  }
  /* initialize array to zero */
  memset ((void *) ptr, 0, sizeof(* ptr) * size);
  
  return (ptr);
}

/* multiply two arrays */
void multiply_arrays_dbl(double * array1, double * array2, double * output_array, int len)
{
  int i;
  for (i = 0; i < len; i++)
    output_array [i] = array1 [i] * array2 [i];
}
