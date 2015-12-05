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

#ifndef HAVE_COMMON_H
#define HAVE_COMMON_H

#include <sndfile.h>
#include "config.h"

#define FFT_MAX    2048           /* maximum size of FFT transform */
#define OPTIMAL_FFT_SIZE(x)       (2 * pow(2, ceil(log2(x)))) 

#define WINDOW_SIZE(x,y)          (floor(((x) * (y) / 1000)))

#define ARRAY_LEN(x)              ((int) (sizeof (x) / sizeof (x [0])))
#define MAX(x,y)                  ((x) > (y) ? (x) : (y))
#define MIN(x,y)                  ((x) < (y) ? (x) : (y))

/* Boolean support */
#if HAVE__BOOL
  #include <stdbool.h>
#else
  #ifndef bool
  #define bool int
  #endif
  #ifndef false
  #define false 0
  #endif
  #ifndef true
  #define true (!false)
  #endif
#endif

#ifndef istrue_bool
#define istrue_bool(x)           ((((bool) (x)) == true) ? (_("enabled")) : (_("disabled")))
#endif

/* sfx_mix_mono_read_double */
extern sf_count_t sfx_mix_mono_read_double (SNDFILE * file, double * data, sf_count_t datalen);

/* enhance audio file */
extern int enhance_audio (SNDFILE * input_file, SNDFILE * output_file, const char * window_type, int window_size, int fft_size, int overlap, bool downmix);

/* separate_channels_double */
extern int separate_channels_double (double * multi_data, double * single_data, int frames, int channels, int channel_number);

/* combine_channels_double */
extern int combine_channels_double (double * multi_data, double * single_data, int frames, int channels, int channel_number);

/* create dynamic double array */
extern double * init_buffer_dbl(size_t size);

/* multiply two arrays */
extern void multiply_arrays_dbl(double * array1, double * array2, double * output_array, int len);

#endif
