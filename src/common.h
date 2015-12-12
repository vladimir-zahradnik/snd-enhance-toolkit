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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sndfile.h>

/* maximum size of FFT transform */
#define FFT_MAX                             2048

/* maximum size of window */
#define WINDOW_MAX                          FFT_MAX/2

/* make sure it is an integer */
#define OPTIMAL_FFT_SIZE(x)                 ((size_t) (2 * pow(2, ceil(log2(x)))))

/* Make sure it is an integer */
#define WINDOW_SIZE(x, y)                   ((size_t) (floor(((x) * (y) / 1000))))

#define ARRAY_LEN(x)                        ((int) (sizeof (x) / sizeof (x [0])))
#define MAX(x, y)                           ((x) > (y) ? (x) : (y))
#define MIN(x, y)                           ((x) < (y) ? (x) : (y))


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
#define istrue_bool(x)                      ((((bool) (x)) == true) ? (_("enabled")) : (_("disabled")))
#endif

/* sfx_mix_mono_read_double */
extern sf_count_t sfx_mix_mono_read_double(SNDFILE *file, double *data, sf_count_t datalen);

/* separate_channels_double */
extern int separate_channels_double(double *multi_data, double *single_data, int frames, int channels,
                                    int channel_number);

/* combine_channels_double */
extern int combine_channels_double(double *multi_data, double *single_data, int frames, int channels,
                                   int channel_number);

/* create dynamic double array */
extern double *init_buffer_dbl(size_t size);

/* multiply two arrays */
extern void multiply_arrays_dbl(double *array1, double *array2, double *output_array, int len);

/* calc_magnitude */
extern void calc_magnitude(const double *freq, size_t fft_size, double *magnitude);

/* calc_phase */
extern void calc_phase(const double *freq, size_t fft_size, double *phase);

/* calc_power_spectrum */
extern double calc_power_spectrum(const double *magnitude, size_t fft_size, double *power_spectrum);

/* recreate complex array from magnitude and phase arrays */
extern void calc_fft_complex_data(const double *magnitude, const double *phase, size_t fft_size, double *freq);

/* multiply_fft_spec_with_gain */
extern void multiply_fft_spec_with_gain(const double *gain, size_t fft_size, double *freq);

/* phase of complex number */
extern double complex_argument(const double real, const double imag);

extern double check_nan(double number);

/* prints time in seconds calculated based on elapsed frames and framerate */
extern char *show_time(int samplerate, int samples);

#endif
