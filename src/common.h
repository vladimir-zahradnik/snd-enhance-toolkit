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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <string.h>
#include <errno.h>
#include <sndfile.h>
#include <fftw3.h>
#include "i18n.h"

#define SETK_VERSION   0.16

#define OPTIMAL_FFT_SIZE(x)       (2 * pow(2, ceil(log2(x)))) 

#define WINDOW_SIZE(x,y)          (floor(((x) * (y) / 1000)))

#define ARRAY_LEN(x)              ((int) (sizeof (x) / sizeof (x [0])))
#define MAX(x,y)                  ((x) > (y) ? (x) : (y))
#define MIN(x,y)                  ((x) < (y) ? (x) : (y))

/* Boolean support */
#ifndef bool
#define bool int
#endif
#ifndef false
#define false 0
#endif
#ifndef true
#define true (!false)
#endif
#ifndef istrue_bool
#define istrue_bool(x)           ((((bool) (x)) == true) ? (_("enabled")) : (_("disabled")))
#endif

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define NF_SABSENT 5              /* number of speech absent frames
                                   * to compute noise power spectrum */

#define FFT_MAX    2048           /* maximum size of FFT transform */
#define THRES      3              /* VAD threshold in dB SNRseg */
#define ALPHA      2.0            /* power exponent             */
#define FLOOR      0.002
#define G          0.9

/* parse_line() rules */
enum pl_ruletype
{
    PLRT_STRING,                  /* Offset points to a String (strdup()) */
    PLRT_INTEGER,                 /* Offset points to a Integer (unsigned int) */
    PLRT_BOOL,                    /* Offset points to a Boolean. */
    PLRT_END                      /* End of rules */
};

/* command line rules */
enum setk_arguments
{
    ARG_WINDOW_TYPE,
    ARG_NOISE_EST,
    ARG_DOWNMIX,
    ARG_INPUT_FILE,
    ARG_OUTPUT_FILE,
    ARG_FRAME_DURATION,
    ARG_OVERLAP,
    ARG_FFT_SIZE,
    ARG_VERSION
};

typedef struct pl_rule_t {
    const char *title;
    unsigned int type;
    unsigned int offset;
} pl_rule;

typedef struct globalArgs_t {
    /* these values may be changed by user */
    int frame_duration;           /* --frame-dur option      */
    int fft_size;                 /* --fft-size option       */
    int overlap_percentage;       /* --overlap option        */
    const char *input_filename;   /* --input option          */
    const char *output_filename;  /* --output option         */
    int window_type;              /* --window option         */
    bool downmix;                 /* --downmix option        */
    bool verbosity;               /* -v or --verbose option  */
    
    /* these values are calcullated from values set above    */
    int nwind;                    /* size of window          */
    double winGain;               /* normalization gain      */
} setk_options;

/* parse configuration file */
extern int parse_line(char *line, const char *split, pl_rule *rules, void *data);

/* sfx_mix_mono_read_double */
extern sf_count_t sfx_mix_mono_read_double (SNDFILE * file, double * data, sf_count_t datalen);

/* enhance audio file */
extern int enhance_audio (SNDFILE * input_file, SNDFILE * output_file, int window_size, int overlap, bool downmix);

/* separate_channels_double */
extern int separate_channels_double (double * multi_data, double * single_data, int frames, int channels, int channel_number);

/* combine_channels_double */
extern int combine_channels_double (double * multi_data, double * single_data, int frames, int channels, int channel_number);

/* create dynamic double array */
double * init_buffer_dbl(size_t size);

#endif
