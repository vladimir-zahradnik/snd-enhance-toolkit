#include "noise_est.h"
#include "i18n.h"
#include <string.h>

noise_est_func_t parse_noise_est_type(const char *name, bool verbose) {
    if (name == NULL) {
        if (verbose == true)
            puts(_("No noise estimation algorithm was specified. Using default."));
        return vad_estimation;
    }
    if (strcmp(name, "vad") == 0)
        return vad_estimation;
    if (strcmp(name, "hirsch") == 0)
        return hirsch_estimation;
    if (strcmp(name, "doblinger") == 0)
        return doblinger_estimation;
    if (strcmp(name, "mcra") == 0)
        return mcra_estimation;
    if (strcmp(name, "mcra2") == 0)
        return mcra2_estimation;

    if (verbose == true)
        puts(_("Error: Unknown noise estimation algorithm. Using default."));
    return vad_estimation;
}

char *get_noise_est_name(const char *name) {
    if (name == NULL) {
        return (_("VAD estimation (default)"));
    }
    if (strcmp(name, "vad") == 0)
        return (_("VAD noise estimation"));
    if (strcmp(name, "hirsch") == 0)
        return (_("Hirsch noise estimation"));
    if (strcmp(name, "doblinger") == 0)
        return (_("Doblinger noise estimation"));
    if (strcmp(name, "mcra") == 0)
        return (_("MCRA noise estimation"));
    if (strcmp(name, "mcra2") == 0)
        return (_("MCRA 2 noise estimation"));

    return (_("VAD estimation (default)"));
}

/* hirsch noise estimation */
double hirsch_estimation(const double *ns_ps, int fft_size, double *noise_ps, double SNRseg, int samplerate) {
    /* initialize static variables */
    static double P[FFT_MAX / 2 + 1];
    static double noise_ps_old[FFT_MAX / 2 + 1];
    static int n = 1; /* check number of calls */
    const double as = 0.85;
    const double beta = 1.5;
    double norm_ns_ps = 0.0;

    int i;

    if (n == 1) {
        memcpy((void *) P, (void *) ns_ps, sizeof(*P) * (fft_size / 2 + 1));
        memcpy((void *) noise_ps_old, (void *) ns_ps, sizeof(*noise_ps) * (fft_size / 2 + 1));

        for (i = 0; i <= fft_size / 2; i++)
            norm_ns_ps += noise_ps_old[i];
    }
    else {
        for (i = 0; i <= fft_size / 2; i++) {
            P[i] = as * P[i] + (1 - as) * ns_ps[i];
            if (P[i] < beta * noise_ps_old[i])
                noise_ps_old[i] = as * noise_ps_old[i] + (1 - as) * P[i];

            norm_ns_ps += noise_ps_old[i];
        }
    }

    n++;
    memcpy((void *) noise_ps, (void *) noise_ps_old, sizeof(*noise_ps) * (fft_size / 2 + 1));
    return norm_ns_ps;
}

/* simple VAD noise estimation */
double vad_estimation(const double *ns_ps, int fft_size, double *noise_ps, double SNRseg, int samplerate) {
    static double noise_ps_old[FFT_MAX / 2 + 1];
    const int nf_sabsent = 6; /* speech absent frames */
    const double thres = 3.0;
    const double G = 0.9;
    static int frame = 0;
    double norm_ns_ps = 0.0;

    int i;

    if (frame < nf_sabsent) {
        for (i = 0; i <= fft_size / 2; i++) {
            noise_ps_old[i] = noise_ps_old[i] + ns_ps[i] / nf_sabsent;
            norm_ns_ps += noise_ps_old[i];
        }
    }
    else {
        /* --- implement a simple VAD detector -------------- */
        for (i = 0; i <= fft_size / 2; i++) {
            if (SNRseg < thres) {
                noise_ps_old[i] = G * noise_ps_old[i] + (1 - G) * ns_ps[i];
            }
            norm_ns_ps += noise_ps_old[i];
        }
    }

    frame++;
    memcpy((void *) noise_ps, (void *) noise_ps_old, sizeof(*noise_ps) * (fft_size / 2 + 1));
    return norm_ns_ps;
}

/* doblinger noise estimation */
double doblinger_estimation(const double *ns_ps, int fft_size, double *noise_ps, double SNRseg, int samplerate) {
    /* initialize static variables */
    static double pxk_old[FFT_MAX / 2 + 1];
    static double pnk_old[FFT_MAX / 2 + 1];
    const double alpha = 0.7;
    const double beta = 0.96;
    const double gamma = 0.998;
    static int n = 1; /* check number of calls */
    double pxk, pnk;
    double norm_ns_ps = 0.0;
    int i;

    if (n == 1) {
        memcpy((void *) pxk_old, (void *) ns_ps, sizeof(*pxk_old) * (fft_size / 2 + 1));
        memcpy((void *) pnk_old, (void *) ns_ps, sizeof(*pnk_old) * (fft_size / 2 + 1));

        for (i = 0; i <= fft_size / 2; i++)
            norm_ns_ps += pnk_old[i];
    }
    else {
        for (i = 0; i <= fft_size / 2; i++) {
            pxk = alpha * pxk_old[i] + (1 - alpha) * ns_ps[i];
            if (pnk_old[i] <= pxk)
                pnk = (gamma * pnk_old[i]) + (((1 - gamma) / (1 - beta)) * (pxk - beta * pxk_old[i]));
            else
                pnk = pxk;

            norm_ns_ps += pnk;
            pxk_old[i] = pxk;
            pnk_old[i] = pnk;
        }
    }

    n++;
    memcpy((void *) noise_ps, (void *) pnk_old, sizeof(*noise_ps) * (fft_size / 2 + 1));
    return norm_ns_ps;
}

/* mcra noise estimation */
double mcra_estimation(const double *ns_ps, int fft_size, double *noise_ps, double SNRseg, int samplerate) {
    /* initialize static variables */
    static double P[FFT_MAX / 2 + 1];
    static double P_min[FFT_MAX / 2 + 1];
    static double P_tmp[FFT_MAX / 2 + 1];
    static double pk[FFT_MAX / 2 + 1];
    static double noise_ps_old[FFT_MAX / 2 + 1];
    const double ad = 0.95;
    const double as = 0.8;
    const int L = 100;
    const int delta = 5;
    const int ap = 0.2;
    double Srk, adk;
    int Ikl;
    static int n = 1; /* check number of calls */
    double norm_ns_ps = 0.0;
    int i;

    if (n == 1) {
        memcpy((void *) P, (void *) ns_ps, sizeof(*P) * (fft_size / 2 + 1));
        memcpy((void *) P_min, (void *) ns_ps, sizeof(*P_min) * (fft_size / 2 + 1));
        memcpy((void *) P_tmp, (void *) ns_ps, sizeof(*P_tmp) * (fft_size / 2 + 1));
        memcpy((void *) noise_ps_old, (void *) ns_ps, sizeof(*noise_ps_old) * (fft_size / 2 + 1));

        for (i = 0; i <= fft_size / 2; i++)
            norm_ns_ps += P[i];
    }
    else {
        for (i = 0; i <= fft_size / 2; i++) {
            P[i] = as * P[i] + (1 - as) * ns_ps[i];

            if (n % L == 0) {
                P_min[i] = MIN (P_tmp[i], P[i]);
                P_tmp[i] = P[i];
            }
            else {
                P_min[i] = MIN (P_min[i], P[i]);
                P_tmp[i] = MIN (P_tmp[i], P[i]);
            }

            Srk = check_nan(P[i] / P_min[i]);

            Ikl = (Srk > delta) ? 1 : 0;

            pk[i] = ap * pk[i] + (1 - ap) * Ikl;

            adk = ad + (1 - ad) * pk[i];

            noise_ps_old[i] = adk * noise_ps_old[i] + (1 - adk) * ns_ps[i];

            norm_ns_ps += noise_ps_old[i];
        }
    }

    n++;
    memcpy((void *) noise_ps, (void *) noise_ps_old, sizeof(*noise_ps) * (fft_size / 2 + 1));
    return norm_ns_ps;
}

/* mcra 2  noise estimation */
double mcra2_estimation(const double *ns_ps, int fft_size, double *noise_ps, double SNRseg, int samplerate) {
    /* initialize static variables */
    static double noise_ps_old[FFT_MAX / 2 + 1];
    static double pxk_old[FFT_MAX / 2 + 1];
    static double pnk_old[FFT_MAX / 2 + 1];
    static double pk[FFT_MAX / 2 + 1];
    static double delta[FFT_MAX / 2 + 1];
    const double ad = 0.95;
    const double ap = 0.2;
    const double beta = 0.8;
    const double gamma = 0.998;
    const double alpha = 0.7;
    int freq_res = samplerate / fft_size;
    int k_1khz = 1000 / freq_res;
    int k_3khz = 3000 / freq_res;
    static int n = 1; /* check number of calls */
    double Srk, adk;
    int Ikl;
    double pxk, pnk;
    double norm_ns_ps = 0.0;
    int i;

    if (n == 1) {
        memcpy((void *) pxk_old, (void *) ns_ps, sizeof(*pxk_old) * (fft_size / 2 + 1));
        memcpy((void *) pnk_old, (void *) ns_ps, sizeof(*pnk_old) * (fft_size / 2 + 1));
        memcpy((void *) noise_ps_old, (void *) ns_ps, sizeof(*noise_ps_old) * (fft_size / 2 + 1));

        /* calculate delta */
        for (i = 0; i <= fft_size / 2; i++) {
            if (i < k_1khz)
                delta[i] = 2;
            if (i >= k_1khz && i < k_3khz)
                delta[i] = 2;
            else
                delta[i] = 5;

            norm_ns_ps += noise_ps_old[i];
        }
    }
    else {
        for (i = 0; i <= fft_size / 2; i++) {
            pxk = alpha * pxk_old[i] + (1 - alpha) * ns_ps[i];
            if (pnk_old[i] <= pxk)
                pnk = (gamma * pnk_old[i]) + (((1 - gamma) / (1 - beta)) * (pxk - beta * pxk_old[i]));
            else
                pnk = pxk;

            norm_ns_ps += pnk;
            pxk_old[i] = pxk;
            pnk_old[i] = pnk;

            Srk = check_nan(pxk / pnk);
            Ikl = (Srk > delta[i]) ? 1 : 0;
            pk[i] = ap * pk[i] + (1 - ap) * Ikl;
            adk = ad + (1 - ad) * pk[i];
            noise_ps_old[i] = adk * noise_ps_old[i] + (1 - adk) * pxk;
        }
    }

    n++;
    memcpy((void *) noise_ps, (void *) noise_ps_old, sizeof(*noise_ps) * (fft_size / 2 + 1));
    return norm_ns_ps;
}
