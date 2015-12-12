#ifndef HAVE_TOOLKIT_H
#define HAVE_TOOLKIT_H

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

/* parse_line() rules */
enum pl_ruletype {
    PLRT_STRING, /* Offset points to a String (strdup()) */
    PLRT_INTEGER, /* Offset points to a Integer (unsigned int) */
    PLRT_BOOL, /* Offset points to a Boolean. */
    PLRT_END      /* End of rules */
};

/* command line rules */
enum setk_arguments_t {
    ARG_WINDOW_TYPE,
    ARG_NOISE_EST,
    ARG_SND_ENH,
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

typedef struct setk_global_args_t {
    /* these values may be changed by user */
    int frame_duration;
    /* --frame-dur option      */
    size_t fft_size;
    /* --fft-size option       */
    int overlap;
    /* --overlap option        */
    const char *input_filename;
    /* --input option          */
    const char *output_filename;
    /* --output option         */
    const char *window_type;
    /* --window option         */
    const char *noise_est_type;
    /* --estimate option       */
    const char *snd_enhance_type;       /* --enhance option        */
    bool downmix;                        /* --downmix option        */
    bool verbosity;
    /* -v or --verbose option  */
    size_t window_size;                     /* size of window          */
} setk_options_t;

#endif