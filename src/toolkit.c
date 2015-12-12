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
#include <errno.h>
#include <getopt.h>

#include "config.h"
#include "common.h"
#include "toolkit.h"
#include "snd_enhance.h"
#include "window.h"
#include "i18n.h"

static pl_rule setk_conf_rules[] = {
        {"input_file",        PLRT_STRING,  offsetof(setk_options_t, input_filename)},
        {"output_file",       PLRT_STRING,  offsetof(setk_options_t, output_filename)},
        {"frame_duration",    PLRT_INTEGER, offsetof(setk_options_t, frame_duration)},
        {"overlap",           PLRT_INTEGER, offsetof(setk_options_t, overlap)},
        {"fft_size",          PLRT_INTEGER, offsetof(setk_options_t, fft_size)},
        {"window",            PLRT_STRING,  offsetof(setk_options_t, window_type)},
        {"noise_estimation",  PLRT_STRING,  offsetof(setk_options_t, noise_est_type)},
        {"sound_enhancement", PLRT_STRING,  offsetof(setk_options_t, snd_enhance_type)},
        {"downmix",           PLRT_BOOL,    offsetof(setk_options_t, downmix)},
        {"verbose",           PLRT_BOOL,    offsetof(setk_options_t, verbosity)},
        {NULL,                PLRT_END, 0},
};

static const struct option long_options[] = {
        {"frame-dur",   required_argument, NULL, ARG_FRAME_DURATION},
        {"fft-size",    required_argument, NULL, ARG_FFT_SIZE},
        {"overlap",     required_argument, NULL, ARG_OVERLAP},
        {"input",       required_argument, NULL, ARG_INPUT_FILE},
        {"output",      required_argument, NULL, ARG_OUTPUT_FILE},
        {"window",      required_argument, NULL, ARG_WINDOW_TYPE},
        {"noise-est",   required_argument, NULL, ARG_NOISE_EST},
        {"snd-enhance", required_argument, NULL, ARG_SND_ENH},
        {"version",     no_argument,       NULL, ARG_VERSION},
        {"downmix",     no_argument,       NULL, ARG_DOWNMIX},
        {"verbose",     no_argument,       NULL, 'v'},
        {"config",      required_argument, NULL, 'c'},
        {"help",        no_argument,       NULL, 'h'},
        {NULL,          no_argument,       NULL, 0}
};

typedef sf_count_t (*snd_read_func_t)(SNDFILE *file, double *data, sf_count_t datalen);

/* read configuration from file */
static int load_config(const char *filename, setk_options_t *args);

/* output file name was not specified */
static const char *create_output_file_name(const char *input_filename);

/* check_int_range */
static void check_int_range(const char *name, int value, int lower, int upper);

/* parse arguments */
static void parse_arguments(setk_options_t *args);

/* parse configuration file */
static int parse_line(char *line, const char *split, pl_rule *rules, void *data);

/* process audio file */
static void process_audio(setk_options_t *args);

/* print file info */
static void file_info(setk_options_t *args, SF_INFO info);

/* Print usage */
static void help(const char *argv0) {

    printf(_(
                   "\nSound Enhancement Toolkit\n"
                           "--------------------------\n\n"
                           "Usage: %s [options]\n\n"
                           "  -h, --help                  Show this help\n"
                           "      --version               Show version\n"

                           "  -c, --config                Custom configuration from file\n\n"

                           "  -v, --verbose               Enable verbose operations\n\n"

                           "      --input                 Input file name\n"
                           "      --output                Output file name\n\n"

                           "                              If no output file name is given,\n"
                           "                              it is created based on input file name\n\n"

                           "      --frame-dur             Duration of speech frame in milliseconds,\n"
                           "                              range <10 - 30> ms\n\n"

                           "      --overlap               Overlap of adjacent frames given as percentual value,\n"
                           "                              range <0 - 99>, where '0' means no overlap\n\n"

                           "      --fft-size              Size of Fast Fourier Transform in range <0 - 2048>.\n"
                           "                              If '0' is set, FFT size is calculated automatically.\n\n"

                           "      --downmix               Downmix multichannel audio to mono\n\n"

                           "      --noise-est             Type of noise estimation algorithm\n\n"

                           "      --snd-enhance           Type of sound enhancement algorithm\n\n"

                           "      --window                Type of window function\n\n"

                           "Supported noise estimation algorithms:\n"
                           "--------------------------------------\n"
                           "vad              Simple estimation of noise spectrum\n"
                           "                 using Voice Activity Detection [VAD] techniques\n"
                           "hirsch           Hirsch method of noise estimation\n"
                           "doblinger        Doblinger method of noise estimation\n"
                           "mcra             MCRA method of noise estimation\n"
                           "mcra2            MCRA 2 method of noise estimation\n\n"

                           "If none of algorithms is selected, VAD will be used.\n\n"

                           "Supported sound enhancement algorithms:\n"
                           "---------------------------------------\n"
                           "specsub          Basic spectral substraction algorithm\n"
                           "mmse             Minimum Mean Square Error algorithm\n"
                           "wiener-as        Wiener filtering algorithm based on a priori SNR estimation\n"
                           "wiener-iter      Basic iterative Wiener filtering algorithm\n"
                           "residual         Residual output for measuring noise estimation algorithms\n\n"

                           "If none of algorithms is selected, basic spectral substraction will be used.\n\n"

                           "Supported window functions:\n"
                           "---------------------------------------\n"
                           "hamming          Hamming window\n"
                           "hann             Hann window\n"
                           "blackman         Blackman window\n"
                           "bartlett         Bartlett window\n"
                           "triangular       Triangular window\n"
                           "rectangular      Rectangular window\n"
                           "nutall           Nutall window\n\n"

                           "If none of window functions is selected, Hamming window will be used.\n\n"

           ), argv0);
}

int main(int argc, char **argv) {
    /* initialize input parameters */
    static setk_options_t opts = {
            .frame_duration = 20,
            .fft_size = 0,
            .overlap = 50,
            .input_filename = NULL,
            .output_filename = NULL,
            .window_type = NULL,
            .noise_est_type = NULL,
            .snd_enhance_type = NULL,
            .downmix = false,
            .verbosity = false,
            .window_size = 0,
    };

    int opt = 0;
    int long_index = 0;

    /* if no argument was specified, print basic menu */
    if (argc == 1) {
        help(argv[0]);
        exit(1);
    }

    while ((opt = getopt_long(argc, argv, "c:vh", long_options, &long_index)) != -1) {
        switch (opt) {
            case ARG_FRAME_DURATION:  /* optional */
                opts.frame_duration = atoi(optarg);
                break;
            case ARG_FFT_SIZE:  /* optional */
                opts.fft_size = (size_t) atoi(optarg);
                break;
            case ARG_OVERLAP:  /* optional, default 50 % */
                opts.overlap = atoi(optarg);
                break;
            case ARG_INPUT_FILE:
                opts.input_filename = optarg;
                break;
            case ARG_OUTPUT_FILE:
                opts.output_filename = optarg;
                break;
            case ARG_VERSION:
                printf(_("Sound Enhancement Toolkit Version %d.%d\n"), snd_tk_VERSION_MAJOR, snd_tk_VERSION_MINOR);
                break;
            case 'v':   /* verbose mode */
                opts.verbosity = true;
                break;
            case 'h':   /* help */
                help(argv[0]);
                return 0;
            case 'c':   /* custom config */
                load_config(optarg, &opts);
                break;
            case ARG_DOWNMIX: /* downmix to mono */
                opts.downmix = true;
                break;
            case ARG_WINDOW_TYPE: /* window function type */
                opts.window_type = optarg;
                break;
            case ARG_NOISE_EST: /* noise estimation algorithm */
                opts.noise_est_type = optarg;
                break;
            case ARG_SND_ENH: /* sound enhancement algorithm */
                opts.snd_enhance_type = optarg;
                break;
            default:
                break;
        }
    }

    process_audio(&opts);

    return 0;
}

/* read configuration from file */
static int load_config(const char *filename, setk_options_t *args) {
    FILE *f;
    char buf[1000];
    unsigned int line = 0;

    f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, _("Could not open config file \"%s\": %s\n"), filename, strerror(errno));
        return 1;
    }

    while (fgets(buf, sizeof(buf), f)) {
        line++;
        if (parse_line(buf, " ", setk_conf_rules, (void *) args)) continue;
        printf(_("Unknown configuration statement on line %u of %s: \"%s\"\n"), line, filename, buf);
    }
    fclose(f);

    return 0;
}

/* no output file was specified, append _enhanced.[extension] to input file name */
static const char *create_output_file_name(const char *input_filename) {
    char *tmp_name = alloca((strlen(input_filename) + 10) * sizeof(*tmp_name));
    char *p_ext = NULL;
    const char *output_filename = NULL;

    if (strcpy(tmp_name, input_filename) == NULL) {
        printf(_("Error: Unable to copy string: %s\n"), sf_strerror(NULL));
        exit(1);
    }

    /* file has no extension */
    if ((p_ext = strrchr(tmp_name, '.')) == NULL) {
        output_filename = strdup(strcat(tmp_name, "_enhanced"));
    }

    else {
        *p_ext = '\0';
        strcat(tmp_name, "_enhanced");
        p_ext = strrchr(input_filename, '.');
        output_filename = strdup(strcat(tmp_name, p_ext));
    }
    return (output_filename);
}

/* check_int_range */
static void check_int_range(const char *name, int value, int lower, int upper) {
    if (value < lower || value > upper) {
        printf("Error : '%s' parameter must be in range [%d, %d]\n", name, lower, upper);
        exit(1);
    };
}

/* parse arguments */
static void parse_arguments(setk_options_t *args) {
    check_int_range("frame duration", args->frame_duration, 10, 30);
    check_int_range("fft size", (int) args->fft_size, 0, FFT_MAX);
    check_int_range("overlap percentage", args->overlap, 0, 99);

    /* no input file was specified */
    if (args->input_filename == NULL) {
        puts(_("No input file was specified."));
        exit(1);
    }

    /* no output file was specified, append _enhanced.[extension] to input file name */
    if (args->output_filename == NULL)
        args->output_filename = create_output_file_name(args->input_filename);

    /* input and output file names are the same */
    if (strcmp(args->input_filename, args->output_filename) == 0) {
        puts(_("Error: input and output file names are the same."));
        exit(1);
    }
}

/* process audio file */
static void process_audio(setk_options_t *args) {
    /* parse command line arguments */
    parse_arguments(args);

    /* initialize variables */
    snd_enh_func_t sound_enhancement;
    noise_est_func_t noise_estimation;
    window_func_t window_function;


    SNDFILE *input_file, *output_file;
    SF_INFO info;
    snd_read_func_t sndfile_read;
    int noverlap, nslide;
    sf_count_t count, frames_read = 0;
    double *multi_data, *prev_multi_data, *window;
    double *fft_data, *es_old_multi;
    double winGain;
    sndfile_read = sf_readf_double;

    /* open input file */
    if ((input_file = sf_open(args->input_filename, SFM_READ, &info)) == NULL) {
        printf(_("Error: Unable to open input file '%s': %s\n"), args->input_filename, sf_strerror(NULL));
        sf_close(input_file);
        exit(1);
    }

    do {

        /* compute window size and make it even */
        (((args->window_size) = WINDOW_SIZE (args->frame_duration, info.samplerate)) % 2 != 0)
        ? (args->window_size) += 1 : (args->window_size);

        /* if computed optimal FFT size is greater than FFT_MAX, decrease
         * frame_duration by one millisecond - repeated until FFT size <= FFT_MAX */
        if ((args->fft_size) == 0 || (args->fft_size) > FFT_MAX) {
            args->fft_size = OPTIMAL_FFT_SIZE(args->window_size);

            if ((args->fft_size) > FFT_MAX)
                args->frame_duration -= 1;
        }

    } while ((args->fft_size) > FFT_MAX);

    /* Force output to mono. */
    if ((args->downmix)) {
        info.channels = 1;
        sndfile_read = sfx_mix_mono_read_double;
    }

    /* print file info */
    if (args->verbosity)
        file_info(args, info);

    /* open output file */
    if ((output_file = sf_open(args->output_filename, SFM_WRITE, &info)) == NULL) {
        printf(_("Error: Unable to open output file '%s': %s\n"), args->output_filename, sf_strerror(NULL));
        sf_close(output_file);
        exit(1);
    }

    if ((args->window_size) > (args->fft_size)) {
        printf(_("%s : Error: Size of FFT is less than window size\n"), __func__);
        exit(1);
    }

    /* write info tag into audio file */
    sf_set_string(output_file, SF_STR_TITLE, args->output_filename);
    sf_set_string(output_file, SF_STR_COMMENT, "Enhanced audio signal");
    sf_set_string(output_file, SF_STR_SOFTWARE, "Sound Enhancement Toolkit");
    sf_set_string(output_file, SF_STR_COPYRIGHT, "No copyright.");

    noverlap = (int) floor((args->window_size) * (args->overlap) / 100);
    nslide = (int) (args->window_size) - noverlap;

    multi_data = init_buffer_dbl((size_t) (args->window_size) * info.channels);
    prev_multi_data = init_buffer_dbl((size_t) noverlap * info.channels);
    es_old_multi = init_buffer_dbl((size_t) nslide * info.channels);

    /* Window function */
    window_function = parse_window_type(args->window_type, args->verbosity);
    window = init_buffer_dbl(args->window_size);

    /* Sound enhancement algorithm */
    sound_enhancement = parse_snd_enhance_type(args->snd_enhance_type, args->verbosity);

    /* Noise estimation algorithm */
    noise_estimation = parse_noise_est_type(args->noise_est_type, args->verbosity);

    /* fft transform data */
    fft_data = init_buffer_dbl(args->fft_size);

    fftw_plan fft_forw = fftw_plan_r2r_1d((int) args->fft_size, fft_data, fft_data, FFTW_R2HC, FFTW_MEASURE);
    fftw_plan fft_back = fftw_plan_r2r_1d((int) args->fft_size, fft_data, fft_data, FFTW_HC2R, FFTW_MEASURE);

    do {
        if (frames_read == 0) {
            if ((count = sndfile_read(input_file, multi_data, (int) args->window_size)) <= 0)
                exit(1);
            memcpy((void *) prev_multi_data, (void *) (multi_data + nslide * info.channels),
                   sizeof(*multi_data) * noverlap * info.channels);
        }
        else {
            count = sndfile_read(input_file, (multi_data + noverlap * info.channels), nslide);
            memcpy((void *) multi_data, (void *) prev_multi_data, sizeof(*prev_multi_data) * noverlap * info.channels);
            memcpy((void *) prev_multi_data, (void *) (multi_data + nslide * info.channels),
                   sizeof(*multi_data) * noverlap * info.channels);
        }

        frames_read += count;
        printf("%s\r", show_time(info.samplerate, (int) frames_read));

        for (int ch = 0; ch < info.channels; ++ch) {
            memset(fft_data, 0, sizeof(*fft_data) * (args->fft_size)); /* initialize fft array to zero values */

            separate_channels_double(multi_data, fft_data, (int) args->window_size, info.channels, ch);

            winGain = apply_window(fft_data, args->window_size, window_function);
            winGain = nslide / winGain;

            sound_enhancement(fft_data, args->fft_size, fft_forw, fft_back, noise_estimation, args->window_size,
                              info.samplerate);

            /* Add-and-Overlap */
            for (int i = 0; i < nslide; ++i) {
                fft_data[i] = winGain * (fft_data[i] / (args->fft_size) + es_old_multi[ch * nslide + i]);
            }

            for (int i = 0; i < nslide; ++i) {
                es_old_multi[ch * nslide + i] = fft_data[i + noverlap] / (args->fft_size);
            }

            combine_channels_double(multi_data, fft_data, nslide, info.channels, ch);
        }

        sf_writef_double(output_file, multi_data, nslide);
    } while (count > 0);

    if (args->verbosity)
        puts(_("\n\nFinished audio processing."));

    fftw_destroy_plan(fft_forw);
    fftw_destroy_plan(fft_back);
    free(fft_data);
    free(multi_data);
    free(prev_multi_data);
    free(es_old_multi);
    free(window);
    sf_close(output_file);
    sf_close(input_file);

}

/* print file info */
static void file_info(setk_options_t *args, SF_INFO info) {
    printf(_("-----------------------------------------\n"));
    printf(_("I N F O R M A T I O N :\n"));
    printf(_("-----------------------------------------\n"));
    printf(_("Input File: %s\n"), args->input_filename);
    printf(_("Output File: %s\n"), args->output_filename);
    printf(_("Duration: %s\n"), show_time(info.samplerate, info.frames));
    printf(_("Samplerate: %d Hz\n"), info.samplerate);
    printf(_("Channels: %d\n"), info.channels);
    printf(_("-----------------------------------------\n"));
    printf(_("Downmix to mono: %s\n"), istrue_bool(args->downmix));
    printf(_("Frame Duration: %d ms\n"), args->frame_duration);
    printf(_("Overlap: %d %%\n"), args->overlap);
    printf(_("Window Size: %d samples\n"), (int) args->window_size);
    printf(_("FFT size: %d samples\n"), (int) args->fft_size);
    printf(_("Window Function: %s\n"), get_window_name(args->window_type));
    printf(_("Noise Estimation Algorithm: %s\n"), get_noise_est_name(args->noise_est_type));
    printf(_("Sound Enhancement Algorithm: %s\n"), get_snd_enhance_name(args->snd_enhance_type));
    printf(_("-----------------------------------------\n\n"));
}

/* parse configuration file */
static int parse_line(char *line, const char *split, pl_rule *rules, void *data) {
    unsigned int r, len;
    char *end = NULL, *val = NULL, *p = NULL;
    void *store;

    /* Chop off \n and \r and white space */
    p = &line[strlen(line) - 1];
    while (p >= line && (
            *p == '\n' ||
            *p == '\r' ||
            *p == '\t' ||
            *p == ' '))
        *p-- = '\0';

    /* Ignore comments and emtpy lines */
    if (strlen(line) == 0 ||
        line[0] == '#' ||
        line[0] == ';' ||
        (line[0] == '/' && line[1] == '/')) {
        return 1;
    }

    /* Get the end of the first argument */
    p = line;
    end = &line[strlen(line) - 1];
    /* Skip until whitespace */
    while (p < end &&
           strncmp(p, split, strlen(split)) != 0)
        p++;
    /* Terminate this argument */
    *p = '\0';
    p++;

    /* Skip whitespace */
    while (p < end &&
           *p == ' ' &&
           *p == '\t')
        p++;

    /* Start of the value */
    val = p + (strlen(split) - 1);

    /* If starting with quotes, skip until next quote */
    if (*p == '"' || *p == '\'') {
        p++;
        /* Find next quote */
        while (p <= end &&
               *p != *val &&
               *p != '\0')
            p++;
        /* Terminate */
        *p = '\0';
        /* Skip the first quote */
        val++;
    }
    /* Otherwise it is already terminated above */

    /* Walk through all the rules */
    for (r = 0; rules[r].type != PLRT_END; ++r) {
        len = (int) strlen(rules[r].title);
        if (strncmp(line, rules[r].title, len) != 0) continue;

        store = (void *) ((char *) data + rules[r].offset);

        switch (rules[r].type) {
            case PLRT_STRING:
                *((const char **) store) = strdup(val);
                break;

            case PLRT_INTEGER:
                *((int *) store) = atoi(val);
                break;

            case PLRT_BOOL:
                if (strcmp(val, "yes") == 0 ||
                    strcmp(val, "true") == 0) {
                    *((bool *) store) = true;
                }
                else if (strcmp(val, "no") == 0 ||
                         strcmp(val, "false") == 0) {
                    *((bool *) store) = false;
                }
                else {
                    printf(_("Unknown boolean value \"%s\" for option \"%s\"\n"), val, rules[r].title);
                }
                break;

            case PLRT_END:
                return 0;

            default:
                // ignore the input as it is a non-standard config statement
                break;
        }
        return 1;
    }
    return 0;
}
