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
#include "snd_enhance.h"
#include "noise_est.h"
#include "tbessi.h"
#include "lpc.h"
#include "i18n.h"

/* Required in spectral substraction algorithm */
static double berouti(double SNR); /* if alpha == 2 */

/* calculate segmentary SNR */
static double calc_snr_seg(double norm_signal, double norm_noise);

snd_enh_func_t parse_snd_enhance_type(const char *name, bool verbose) {
    if (name == NULL) {
        if (verbose == true)
            puts(_("No sound enhancement algorithm was specified. Using default."));
        return snd_enhance_specsub;
    }
    if (strcmp(name, "specsub") == 0)
        return snd_enhance_specsub;
    if (strcmp(name, "wiener-as") == 0)
        return snd_enhance_wiener_as;
    if (strcmp(name, "wiener-iter") == 0)
        return snd_enhance_wiener_iter;
    if (strcmp(name, "mmse") == 0)
        return snd_enhance_mmse;
    if (strcmp(name, "residual") == 0)
        return snd_enhance_residual;

    if (verbose == true)
        puts(_("Error: Unknown sound enhancement algorithm was specified. Using default."));
    return snd_enhance_specsub;
}

char *get_snd_enhance_name(const char *name) {
    if (name == NULL) {
        return (_("Spectral substraction algorithm (default)"));
    }
    if (strcmp(name, "specsub") == 0)
        return (_("Spectral substraction algorithm"));
    if (strcmp(name, "wiener-as") == 0)
        return (_("Wiener filter with a priori SNR estimation"));
    if (strcmp(name, "wiener-iter") == 0)
        return (_("Iterative Wiener filter"));
    if (strcmp(name, "mmse") == 0)
        return (_("Minimum Mean Square Error [MMSE]"));
    if (strcmp(name, "residual") == 0)
        return (_("Residual noise output"));

    return (_("Spectral substraction algorithm (default)"));
}

void snd_enhance_specsub(double *fft_data, int fft_size, fftw_plan fft_forw, fftw_plan fft_back,
                         noise_est_func_t noise_estimation, int datalen, int samplerate) {
    /* initialize variables */
    double *y_ps = init_buffer_dbl(fft_size / 2 + 1); /* power spectrum */
    double *y_phase = init_buffer_dbl(fft_size / 2 + 1); /* phase */
    double *noise_ps = init_buffer_dbl(fft_size / 2 + 1); /* noise power spectrum */
    double norm_ps, norm_ns_ps, beta;
    static double SNRseg = 0.0;
    const double floor = 0.002;
    int i;

    /* FFT */
    fftw_execute(fft_forw);

    calc_magnitude(fft_data, fft_size, y_ps);

    calc_phase(fft_data, fft_size, y_phase);

    norm_ps = calc_power_spectrum(y_ps, fft_size, y_ps);

    /* noise estimation */
    norm_ns_ps = noise_estimation(y_ps, fft_size, noise_ps, SNRseg, samplerate);

    SNRseg = calc_snr_seg(norm_ps, norm_ns_ps);

    beta = berouti(SNRseg);

    /* spectral substraction */
    for (i = 0; i <= fft_size / 2; i++) {
        y_ps[i] = y_ps[i] - beta * noise_ps[i];
        if ((y_ps[i] - floor * noise_ps[i]) < 0) {
            /* floor negative components */
            y_ps[i] = floor * noise_ps[i];
        }
        /* create enhanced magnitude spectrum */
        y_ps[i] = sqrt(y_ps[i]);
    }

    /* recreate frequency spectrum from magnitude and phase */
    calc_fft_complex_data(y_ps, y_phase, fft_size, fft_data);

    /* IFFT */
    fftw_execute(fft_back);

    free(y_ps);
    free(y_phase);
    free(noise_ps);
}

void snd_enhance_mmse(double *fft_data, int fft_size, fftw_plan fft_forw, fftw_plan fft_back,
                      noise_est_func_t noise_estimation, int datalen, int samplerate) {
    /* initialize variables */
    double *y_ps = init_buffer_dbl(fft_size / 2 + 1); /* power spectrum */
    double *y_phase = init_buffer_dbl(fft_size / 2 + 1); /* phase */
    double *noise_ps = init_buffer_dbl(fft_size / 2 + 1); /* noise power spectrum */
    double norm_ps, norm_ns_ps;
    static double Xk_prev[FFT_MAX / 2 + 1];
    static double SNRseg = 0.0;
    static int calls = 0;
    int i;

    /* MMSE parameters */
    const double aa = 0.98;
    const double c = sqrt(M_PI) / 2;
    const double qk = 0.3;
    const double qkr = (1 - qk) / qk;
    const double ksi_min = pow(10, -2.5);
    double gammak, ksi, max, vk, j0, j1, A, B, C, hw, evk, Lambda, pSAP;

    /* FFT */
    fftw_execute(fft_forw);

    calc_magnitude(fft_data, fft_size, y_ps);

    calc_phase(fft_data, fft_size, y_phase);

    norm_ps = calc_power_spectrum(y_ps, fft_size, y_ps);

    /* noise estimation */
    norm_ns_ps = noise_estimation(y_ps, fft_size, noise_ps, SNRseg, samplerate);

    SNRseg = calc_snr_seg(norm_ps, norm_ns_ps);

    for (i = 0; i <= fft_size / 2; i++) {
        gammak = check_nan(y_ps[i] / noise_ps[i]);

        if (gammak > 40)
            gammak = 40;

        max = ((gammak - 1) > 0) ? (gammak - 1) : 0;

        if (calls == 0)
            ksi = aa + (1 - aa) * max;
        else {
            ksi = check_nan(aa * Xk_prev[i] / noise_ps[i]) + (1 - aa) * max;
            /* decision-direct estimate of a priori SNR */
            if (ksi < ksi_min)
                ksi = ksi_min; /* limit ksi to -25 dB */
        }

        vk = ksi * gammak / (1 + ksi);
        j0 = BESSI(0, vk / 2);
        j1 = BESSI(1, vk / 2);

        /* --------------- */
        C = exp(-0.5 * vk);
        A = ((c * pow(vk, 0.5)) * C) / gammak;
        B = (1 + vk) * j0 + vk * j1;
        hw = A * B;

        /* Speech Presence Uncertainity */
        evk = exp(vk);
        Lambda = qkr * evk / (1 + ksi);
        pSAP = Lambda / (1 + Lambda);

        y_ps[i] = sqrt(y_ps[i]) * hw * pSAP; /* enhanced magnitude spectrum */

        Xk_prev[i] = pow(y_ps[i], 2);

    }

    /* recreate frequency spectrum from magnitude and phase */
    calc_fft_complex_data(y_ps, y_phase, fft_size, fft_data);

    /* IFFT */
    fftw_execute(fft_back);

    calls++;

    free(y_ps);
    free(y_phase);
    free(noise_ps);
}

void snd_enhance_wiener_as(double *fft_data, int fft_size, fftw_plan fft_forw, fftw_plan fft_back,
                           noise_est_func_t noise_estimation, int datalen, int samplerate) {
    /* initialize variables */
    double *y_ps = init_buffer_dbl(fft_size / 2 + 1); /* power spectrum */
    double *noise_ps = init_buffer_dbl(fft_size / 2 + 1); /* noise power spectrum */
    double *priori = init_buffer_dbl(fft_size / 2 + 1);
    double *posteri = init_buffer_dbl(fft_size / 2 + 1);
    double *posteri_prime = init_buffer_dbl(fft_size / 2 + 1);
    double *G = init_buffer_dbl(fft_size / 2 + 1);
    static double posteri_prev[
            FFT_MAX / 2 + 1]; /* needs to be static array, because it must remember its previous state */
    static double G_prev[FFT_MAX / 2 + 1];
    double norm_ps, norm_ns_ps;
    static double SNRseg = 0.0;
    static int calls = 0;
    const double a_dd = 0.98;
    int i;

    /* FFT */
    fftw_execute(fft_forw);

    calc_magnitude(fft_data, fft_size, y_ps);

    norm_ps = calc_power_spectrum(y_ps, fft_size, y_ps);

    /* noise estimation */
    norm_ns_ps = noise_estimation(y_ps, fft_size, noise_ps, SNRseg, samplerate);

    SNRseg = calc_snr_seg(norm_ps, norm_ns_ps);

    for (i = 0; i <= fft_size / 2; i++) {
        posteri[i] = check_nan(y_ps[i] / noise_ps[i]);

        posteri_prime[i] = posteri[i] - 1;

        if (posteri_prime[i] < 0)
            posteri_prime[i] = 0;

        if (calls == 0)
            priori[i] = a_dd + (1 - a_dd) * posteri_prime[i];
        else
            priori[i] = a_dd * pow(G_prev[i], 2) * posteri_prev[i] + (1 - a_dd) * posteri_prime[i];

        /* Gain function */
        G[i] = sqrt(priori[i] / (1 + priori[i]));
    }

    /* Multiply FFT spectrum with gain function */
    multiply_fft_spec_with_gain(G, fft_size, fft_data);

    /* IFFT */
    fftw_execute(fft_back);

    memcpy((void *) G_prev, (void *) G, sizeof(*G) * (fft_size / 2 + 1));
    memcpy((void *) posteri_prev, (void *) posteri, sizeof(*posteri) * (fft_size / 2 + 1));

    calls++;

    free(y_ps);
    free(noise_ps);
    free(posteri);
    free(posteri_prime);
    free(priori);
    free(G);
}

void snd_enhance_wiener_iter(double *fft_data, int fft_size, fftw_plan fft_forw, fftw_plan fft_back,
                             noise_est_func_t noise_estimation, int datalen, int samplerate) {
    /* initialize variables */
    const int pred_order = 12; /* LPC order */
    const int iter_num = 3;
    const double min_energy = 1e-16;
    double *y_ps = init_buffer_dbl(fft_size / 2 + 1); /* power spectrum */
    double *noise_ps = init_buffer_dbl(fft_size / 2 + 1); /* noise power spectrum */
    double *xx = init_buffer_dbl(fft_size / 2 + 1);
    double xx_tmp[2]; /* tmp variable for xx array, contains real and imag data */
    double *h_spec = init_buffer_dbl(fft_size / 2 + 1);
    double *lpc_coeffs = init_buffer_dbl(sizeof(*lpc_coeffs) * pred_order); /* LPC coefficients */
    double norm_ps, norm_ns_ps;
    static double SNRseg = 0.0;
    double mean_tmp = 0;
    double lpc_energy = 0;
    double g = 0; /* gain */
    int i, j, k;

    lpc_from_data(fft_data, lpc_coeffs, datalen, pred_order);

    /* FFT */
    fftw_execute(fft_forw);

    calc_magnitude(fft_data, fft_size, y_ps);

    norm_ps = calc_power_spectrum(y_ps, fft_size, y_ps);

    /* noise estimation */
    norm_ns_ps = noise_estimation(y_ps, fft_size, noise_ps, SNRseg, samplerate);

    SNRseg = calc_snr_seg(norm_ps, norm_ns_ps);

    /* wiener iterations */
    for (k = 0; k < iter_num; k++) {

        for (i = 0; i <= fft_size / 2; i++) {
            if (i == 0) {
                lpc_energy = 0.0;
                mean_tmp = 0.0;
            }

            for (j = 0; j <= pred_order; j++) {
                if (j == 0) {
                    /* first LPC coefficient, e.g. 1.0, is not in array */
                    xx_tmp[0] = 1.0;
                    xx_tmp[1] = 0.0;
                }
                else {
                    xx_tmp[0] += cos(j * i * 2 * M_PI / fft_size) * lpc_coeffs[j - 1];
                    xx_tmp[1] += sin(j * i * 2 * M_PI / fft_size) * lpc_coeffs[j - 1];
                }
            }
            /* calc magnitude spectrum value from xx_tmp */
            xx[i] = sqrt(xx_tmp[0] * xx_tmp[0] + xx_tmp[1] * xx_tmp[1]);
            xx[i] = 1.0 / (xx[i] * xx[i]);
            lpc_energy += xx[i];
            mean_tmp += y_ps[i] - noise_ps[i];
        }

        g = check_nan(mean_tmp / lpc_energy);

        if (g < min_energy)
            g = min_energy;

        /* wiener filtering */
        for (i = 0; i <= fft_size / 2; i++) {
            h_spec[i] = (g * xx[i]) / (g * xx[i] + noise_ps[i]);
        }

        /* Multiply FFT spectrum with gain function */
        multiply_fft_spec_with_gain(h_spec, fft_size, fft_data);

        /* IFFT */
        fftw_execute(fft_back);

        if (k < iter_num - 1) {
            for (i = 0; i < fft_size; i++) {
                if (i < datalen)
                    fft_data[i] = fft_data[i] / fft_size;
                else
                    fft_data[i] = 0;
            }

            /* calculate new LPC coefficients */
            lpc_from_data(fft_data, lpc_coeffs, fft_size, pred_order);

            /* FFT */
            fftw_execute(fft_forw);
        }
    }

    free(y_ps);
    free(noise_ps);
    free(lpc_coeffs);
    free(xx);
    free(h_spec);
}

/* Required in spectral substraction algorithm */
static double berouti(double SNR) {
    if (SNR >= -5.0 && SNR <= 20) {
        return (4 - SNR * 3 / 20);
    }
    else {
        if (SNR < -5.0) return 5.0;
        if (SNR > 20) return 1.0;
    }
    return 0;
}

/* calculate segmentary SNR */
static double calc_snr_seg(double norm_signal, double norm_noise) {
    double SNRseg;

    SNRseg = 10 * log10(check_nan(norm_signal / norm_noise));

    return SNRseg;
}

void snd_enhance_residual(double *fft_data, int fft_size, fftw_plan fft_forw, fftw_plan fft_back,
                          noise_est_func_t noise_estimation, int datalen, int samplerate) {
    /* initialize variables */
    double *y_ps = init_buffer_dbl(fft_size / 2 + 1); /* power spectrum */
    double *y_phase = init_buffer_dbl(fft_size / 2 + 1); /* phase */
    double *noise_ps = init_buffer_dbl(fft_size / 2 + 1); /* noise power spectrum */
    double norm_ps, norm_ns_ps;
    static double SNRseg = 0.0;
    int i;

    /* FFT */
    fftw_execute(fft_forw);

    calc_magnitude(fft_data, fft_size, y_ps);

    calc_phase(fft_data, fft_size, y_phase);

    norm_ps = calc_power_spectrum(y_ps, fft_size, y_ps);

    /* noise estimation */
    norm_ns_ps = noise_estimation(y_ps, fft_size, noise_ps, SNRseg, samplerate);

    SNRseg = calc_snr_seg(norm_ps, norm_ns_ps);

    for (i = 0; i <= fft_size / 2; i++) {
        noise_ps[i] = sqrt(noise_ps[i]);
    }

    /* recreate frequency spectrum from magnitude and phase */
    calc_fft_complex_data(noise_ps, y_phase, fft_size, fft_data);

    /* IFFT */
    fftw_execute(fft_back);

    free(y_ps);
    free(y_phase);
    free(noise_ps);
}
