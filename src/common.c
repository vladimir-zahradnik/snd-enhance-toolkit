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

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "common.h"
#include "i18n.h"

/* sfx_mix_mono_read_double */
sf_count_t sfx_mix_mono_read_double(SNDFILE *file, double *data, sf_count_t datalen) {
    SF_INFO info;

#if HAVE_SF_GET_INFO
    /*
    ** The function sf_get_info was in a number of 1.0.18 pre-releases but was removed
    ** before 1.0.18 final and replaced with the SFC_GET_CURRENT_SF_INFO command.
    */
          sf_get_info (file, &info);
#else
    sf_command(file, SFC_GET_CURRENT_SF_INFO, &info, sizeof(info));
#endif

    if (info.channels == 1)
        return sf_read_double(file, data, datalen);

    static double multi_data[2048];
    int frames_read;
    sf_count_t dataout = 0;

    while (dataout < datalen) {
        int this_read;

        this_read = MIN (ARRAY_LEN(multi_data) / info.channels, datalen);

        frames_read = sf_readf_double(file, multi_data, this_read);
        if (frames_read == 0)
            break;

        for (int k = 0; k < frames_read; ++k) {
            double mix = 0.0;

            for (int ch = 0; ch < info.channels; ++ch)
                mix += multi_data[k * info.channels + ch];
            data[dataout + k] = mix / info.channels;
        };

        dataout += this_read;
    };

    return dataout;
}

/* separate_channels */
int separate_channels_double(double *multi_data, double *single_data, int frames, int channels, int channel_number) {

    if (channel_number > channels) {
        fprintf(stderr, _("This recording has only %u channels."), channels);
        exit(1);
    }

    for (int k = 0; k < frames; ++k)
        single_data[k] = multi_data[k * channels + channel_number];

    return 0;
}

/* combine_channels_double */
int combine_channels_double(double *multi_data, double *single_data, int frames, int channels, int channel_number) {

    if (channel_number > channels) {
        fprintf(stderr, _("This recording has only %u channels."), channels);
        exit(1);
    }

    for (int k = 0; k < frames; ++k)
        multi_data[k * channels + channel_number] = single_data[k];

    return 0;
}

double *init_buffer_dbl(size_t size) {
    double *ptr = (double *) malloc(sizeof(*ptr) * size);

    if (ptr == NULL) {
        printf(_("\nError: malloc() failed: %s\n"), strerror(errno));
        exit(1);
    }
    /* initialize array to zero */
    memset((void *) ptr, 0, sizeof(*ptr) * size);

    return (ptr);
}

/* multiply two arrays */
void multiply_arrays_dbl(double *array1, double *array2, double *output_array, int len) {
    for (int i = 0; i < len; ++i)
        output_array[i] = array1[i] * array2[i];
}

/* calc_magnitude */
void calc_magnitude(const double *freq, size_t fft_size, double *magnitude) {
    for (size_t i = 0; i <= fft_size / 2; ++i) {
        if (i == 0 || (i == fft_size / 2 && (fft_size % 2 == 0))) /* fft_size is even */
            magnitude[i] = sqrt(freq[i] * freq[i]);
        else
            magnitude[i] = sqrt(freq[i] * freq[i] + freq[fft_size - i] * freq[fft_size - i]);
    }

    return;
}

/* calc_phase */
void calc_phase(const double *freq, size_t fft_size, double *phase) {
    for (size_t i = 0; i <= fft_size / 2; ++i) {
        if (i == 0 || (i == fft_size / 2 && (fft_size % 2 == 0))) /* fft_size is even */
            phase[i] = complex_argument(freq[i], 0.0);
        else
            phase[i] = complex_argument(freq[i], freq[fft_size - i]);
    }
    return;
}

/* calc_power_spectrum */
double calc_power_spectrum(const double *magnitude, size_t fft_size, double *power_spectrum) {
    double norm_ps = 0;

    for (size_t i = 0; i <= fft_size / 2; ++i) {
        power_spectrum[i] = magnitude[i] * magnitude[i];
        norm_ps += power_spectrum[i];
    }

    return norm_ps;
}

/* calc_fft_complex_data */
void calc_fft_complex_data(const double *magnitude, const double *phase, size_t fft_size, double *freq) {

    for (size_t i = 0; i <= fft_size / 2; ++i) {
        if (i == 0 || (i == fft_size / 2 && (fft_size % 2 == 0))) /* fft_size is even */
            freq[i] = magnitude[i];
        else {
            freq[i] = magnitude[i] * cos(phase[i]);
            freq[fft_size - i] = magnitude[i] * sin(phase[i]);
        }
    }

    return;
}

/* multiply_fft_spec_with_gain */
void multiply_fft_spec_with_gain(const double *gain, size_t fft_size, double *freq) {
    for (size_t i = 0; i <= fft_size / 2; ++i) {
        if (i == 0 || (i == fft_size / 2 && (fft_size % 2 == 0))) /* fft_size is even */
            freq[i] *= gain[i];
        else {
            freq[i] *= gain[i];
            freq[fft_size - i] *= gain[i];
        }
    }

    return;
}

double complex_argument(const double real, const double imag) {
    if (real > 0)
        return (atan(imag / real));
    if ((real < 0) && (imag >= 0))
        return (atan(imag / real) + M_PI);
    if ((real < 0) && (imag < 0))
        return (atan(imag / real) - M_PI);
    if ((real == 0) && (imag > 0))
        return (M_PI / 2);
    if ((real == 0) && (imag < 0))
        return (-M_PI / 2);

    return 0;
}

double check_nan(double number) {
    // isnan() uses float as input. However for checking of NaN condition we should be OK.
    if (isnan((float) number) || isinf((float) number))
        return 0.0;

    return number;
}

char *show_time(int samplerate, int samples) {
    static char time_buff[15];
    int hours, minutes;
    double seconds;

    seconds = (double) samples / samplerate;

    minutes = (int) seconds / 60;
    seconds -= minutes * 60;

    hours = minutes / 60;
    minutes -= hours * 60;

    sprintf(time_buff, "%02d:%02d:%06.3f", hours, minutes, seconds);
    return time_buff;
}