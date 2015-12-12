/*************************************************************
**
**      Window function prototypes from :
**
**      http://en.wikipedia.org/wiki/Window_function
**
**************************************************************/
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "window.h"
#include "i18n.h"

window_func_t parse_window_type(const char *name, bool verbose) {
    if (name == NULL) {
        if (verbose)
            puts(_("No window type was specified. Using default."));
        return calc_hamming_window;
    }
    if (strcmp(name, "hamming") == 0)
        return calc_hamming_window;
    if (strcmp(name, "hann") == 0)
        return calc_hann_window;
    if (strcmp(name, "blackman") == 0)
        return calc_blackman_window;
    if (strcmp(name, "bartlett") == 0)
        return calc_bartlett_window;
    if (strcmp(name, "triangular") == 0)
        return calc_triangular_window;
    if (strcmp(name, "rectangular") == 0)
        return calc_rectangular_window;
    if (strcmp(name, "nuttall") == 0)
        return calc_nuttall_window;

    if (verbose)
        puts(_("Error: Unknown window type. Using default."));
    return calc_hamming_window;
}

char *get_window_name(const char *name) {
    if (name == NULL) {
        return (_("Hamming window (default)"));
    }
    if (strcmp(name, "hamming") == 0)
        return (_("Hamming window"));
    if (strcmp(name, "hann") == 0)
        return (_("Hann window"));
    if (strcmp(name, "blackman") == 0)
        return (_("Blackman window"));
    if (strcmp(name, "bartlett") == 0)
        return (_("Bartlett window"));
    if (strcmp(name, "triangular") == 0)
        return (_("Triangular window"));
    if (strcmp(name, "rectangular") == 0)
        return (_("Rectangular window"));
    if (strcmp(name, "nuttall") == 0)
        return (_("Nutall window"));

    return (_("Hamming window (default)"));
}

/* apply_window */
double apply_window(double *data, size_t datalen, window_func_t calc_window) {
    static double window[WINDOW_MAX];
    static size_t window_len = 0;
    static double winGain;

    if (window_len != datalen) {
        window_len = datalen;
        if (datalen > ARRAY_LEN (window)) {
            printf(_("%s : datalen >  MAX_HEIGHT\n"), __func__);
            exit(1);
        };

        winGain = calc_window(window, datalen);
    };

    for (size_t n = 0; n < datalen; ++n)
        data[n] *= window[n];

    return winGain;
}

/* hamming window */
double calc_hamming_window(double *data, size_t datalen) {
    double winGain = 0.0;

    for (size_t n = 0; n < datalen; ++n) {
        data[n] = ((n >= 0) && (n <= datalen - 1)) ? 0.54 - 0.46 * cos(2 * M_PI * n / (datalen - 1)) : 0;
        winGain += data[n];
    }

    return winGain;
}

/* hann window */
double calc_hann_window(double *data, size_t datalen) {
    double winGain = 0.0;

    for (size_t n = 0; n < datalen; ++n) {
        data[n] = ((n >= 0) && (n <= datalen - 1)) ? 0.5 * (1 - cos(2 * M_PI * (n + 1) / (datalen + 1))) : 0;
        winGain += data[n];
    }

    return winGain;
}

/* blackman window */
double calc_blackman_window(double *data, size_t datalen) {
    double winGain = 0.0;

    for (size_t n = 0; n < datalen; n++) {
        data[n] = ((n >= 0) && (n <= datalen - 1)) ?
                  0.42 - 0.5 * cos(2 * M_PI * n / (datalen - 1)) + 0.08 * cos(4 * M_PI * n / (datalen - 1)) : 0;
        winGain += data[n];
    }

    return winGain;
}

/* bartlett window */
double calc_bartlett_window(double *data, size_t datalen) {
    double winGain = 0.0;

    for (size_t n = 0; n < datalen; ++n) {
        if ((datalen % 2) == 0) { /* n is even */
            if ((n >= 0) && (n <= datalen / 2 - 1))
                data[n] = 2.0 * n / (datalen - 1);
            else if ((n >= datalen / 2) && (n <= datalen - 1))
                data[n] = 2.0 * (datalen - n - 1) / (datalen - 1);
            else
                data[n] = 0.0;
        }
        else { /* n is odd */
            if ((n >= 0) && (n <= (datalen - 1) / 2))
                data[n] = 2.0 * n / (datalen - 1);
            else if ((n > (datalen - 1) / 2) && (n <= datalen - 1))
                data[n] = 2 - 2.0 * n / (datalen - 1);
            else
                data[n] = 0.0;
        }
        winGain += data[n];
    }

    return winGain;
}

/* triangular window */
double calc_triangular_window(double *data, size_t datalen) {
    double winGain = 0.0;

    for (size_t n = 0; n < datalen; ++n) {
        if ((datalen % 2) == 0) { /* n is even */
            if ((n >= 0) && (n <= datalen / 2 - 1))
                data[n] = (2.0 * n + 1) / datalen;
            else if ((n >= datalen / 2) && (n <= datalen - 1))
                data[n] = (2.0 * (datalen - n) - 1) / datalen;
            else
                data[n] = 0.0;
        }
        else { /* n is odd */
            if ((n >= 0) && (n <= (datalen - 1) / 2))
                data[n] = 2.0 * (n + 1) / (datalen + 1);
            else if ((n > (datalen - 1) / 2) && (n <= datalen - 1))
                data[n] = 2.0 * (datalen - n) / (datalen + 1);
            else
                data[n] = 0.0;
        }
        winGain += data[n];
    }

    return winGain;
}

/* rectangular window */
double calc_rectangular_window(double *data, size_t datalen) {
    double winGain = 0.0;

    for (size_t n = 0; n < datalen; ++n) {
        data[n] = ((n >= 0) && (n < datalen)) ? 1 : 0;
        winGain += data[n];
    }

    return winGain;
}

/* nuttall_window */
double calc_nuttall_window(double *data, size_t datalen) {
    const double a[4] = {0.355768, 0.487396, 0.144232, 0.012604};
    double scale;
    double winGain = 0.0;

    for (size_t n = 0; n < datalen; ++n) {
        scale = M_PI * n / (datalen - 1);

        data[n] = a[0] - a[1] * cos(2.0 * scale) + a[2] * cos(4.0 * scale) - a[3] * cos(6.0 * scale);
        winGain += data[n];
    };

    return winGain;
}

