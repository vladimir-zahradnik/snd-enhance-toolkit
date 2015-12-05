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

#include <getopt.h>
#include "common.h"        /* common functions to all setk modules */

static pl_rule setk_conf_rules[] = {
  { "input_file",        PLRT_STRING,     offsetof(setk_options, input_filename) },
  { "output_file",       PLRT_STRING,     offsetof(setk_options, output_filename) },
  { "frame_duration",    PLRT_INTEGER,    offsetof(setk_options, frame_duration) },
  { "overlap",           PLRT_INTEGER,    offsetof(setk_options, overlap_percentage) },
  { "fft_size",          PLRT_INTEGER,    offsetof(setk_options, fft_size) },
  { "window",            PLRT_INTEGER,    offsetof(setk_options, window_type) },
  { "downmix",           PLRT_BOOL,       offsetof(setk_options, downmix) },
  { "verbose",           PLRT_BOOL,       offsetof(setk_options, verbosity) },
  { NULL,                PLRT_END,        0 },
};

static const struct option long_options[] = {
    { "frame-dur", required_argument, NULL, ARG_FRAME_DURATION },
    { "fft-size", required_argument, NULL, ARG_FFT_SIZE },
    { "overlap", required_argument, NULL, ARG_OVERLAP },
    { "input", required_argument, NULL, ARG_INPUT_FILE },
    { "output", required_argument, NULL, ARG_OUTPUT_FILE },
    { "window", required_argument, NULL, ARG_WINDOW_TYPE },
    { "version", no_argument, NULL, ARG_VERSION },
    { "downmix", no_argument, NULL, ARG_DOWNMIX },
    { "verbose", no_argument, NULL, 'v' },
    { "config", required_argument, NULL, 'c' },
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 0 }
};

/* read configuration from file */
static int load_config(const char *filename, setk_options * args);

/* output file name was not specified */
static const char * create_output_file_name (const char * input_filename);

/* check_int_range */
static void check_int_range (const char * name, int value, int lower, int upper);

/* parse arguments */
static void parse_arguments(setk_options * args);

/* process audio file */
static void process_file (setk_options * args);

/* print file info */
static void file_info (setk_options * args);

/* Print usage */
static void help(const char *argv0)
{

    printf(_(
             "\nSpeech Enhancement Toolkit\n"
	     "--------------------------\n\n"
             "Usage: %s [options]\n\n"
             "  -h, --help                            Show this help\n"
             "      --version                         Show version\n"
	     "  -c, --config                          Custom configuration from file\n\n"
             "  -v, --verbose                         Enable verbose operations\n\n"
             "      --input                           Input file name [mandatory]\n"
             "      --output                          Output file name\n\n"
             "      --frame-dur                       Duration of speech frame in milliseconds\n"
             "      --overlap                         Overlap of adjacent frames given as percentual value,\n"
             "                                        range <0 - 99>\n\n"
             "      --fft-size                        Size of Fast Fourier Transform\n"
	     "      --downmix                         Downmix multichannel audio to mono\n"
             "      --window [number]                 Type of window function [hamming [0], hanning [1], blackman [2],\n"
             "                                        bartlett [3], triangular [4] or boxcar [5] window]\n\n")
           , argv0);
}

int main(int argc, char **argv)
{
  /* initialize input parameters */
  static setk_options opts = {
    .frame_duration = 20,
    .fft_size = 0,
    .overlap_percentage = 50,
    .input_filename = NULL,
    .output_filename = NULL,
    .window_type = 0,
    .downmix = false,
    .verbosity = false,
    .nwind = 0,
    .winGain = 0.0
  };
  
  int opt = 0;
  int long_index = 0;
  
  /* if no argument was specified, print basic menu */
    if (argc == 1) {
       help(argv[0]);
       exit (1);
    }
  
    while( (opt = getopt_long( argc, argv, "c:vh", long_options, &long_index )) != -1 ) {
        switch( opt ) {
          case ARG_FRAME_DURATION:  /* optional */
              opts.frame_duration = atoi(optarg);
              break;
          case ARG_FFT_SIZE:  /* optional */
              opts.fft_size = atoi(optarg);
              break;
          case ARG_OVERLAP:  /* optional, default 50 % */
              opts.overlap_percentage = atoi(optarg);
              break;
          case ARG_INPUT_FILE:
              opts.input_filename = optarg;
              break;
          case ARG_OUTPUT_FILE:
              opts.output_filename = optarg;
              break;
          case ARG_VERSION:
              printf(_("Speech Enhancement Toolkit version %.2f\n"),
                       SETK_VERSION);
              break;
          case 'v':   /* verbose mode */
              opts.verbosity = true;
              break;
          case 'h':   /* help */
              help(argv[0]);
              return 0;
              break;
          case 'c':   /* custom config */
              load_config(optarg, &opts);
              break;
          case ARG_DOWNMIX: /* downmix to mono */
               opts.downmix = true;
               break;
          case ARG_WINDOW_TYPE: /* window function type */
              if (atoi(optarg) >= 0 && atoi(optarg) <= 5) {
                  opts.window_type = atoi(optarg);
                 }
              break;
          default:
              break;
        }
    }
    
    process_file (&opts);
    
    return 0;
}

/* read configuration from file */
static int load_config(const char *filename, setk_options * args)
{
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
static const char * create_output_file_name (const char * input_filename)
{
    char *tmp_name = alloca( (strlen(input_filename)+10) * sizeof(char) );
    char *p_ext = NULL;
    const char * output_filename = NULL;
    
    if (strcpy(tmp_name, input_filename) == NULL) {
        perror("Unable to copy string");
        exit (1);
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
static void check_int_range (const char * name, int value, int lower, int upper)
{
  if (value < lower || value > upper) {
       printf ("Error : '%s' parameter must be in range [%d, %d]\n", name, lower, upper) ;
       exit (1);
      };
}

/* parse arguments */
static void parse_arguments(setk_options * args)
{
  check_int_range ("frame duration", args -> frame_duration, 10, 30);
  check_int_range ("fft size", args -> fft_size, 0, FFT_MAX);
  check_int_range ("overlap percentage", args -> overlap_percentage, 0, 99);
  
  /* no input file was specified */
    if (args -> input_filename == NULL) {
         puts(_("No input file was specified."));
         exit (1);
    }
  
  /* no output file was specified, append _enhanced.[extension] to input file name */
    if (args -> output_filename == NULL)
        args -> output_filename = create_output_file_name(args -> input_filename);
  
    /* input and output file names are the same */
    if (strcmp (args -> input_filename, args -> output_filename) == 0) {
         puts (_("Error: input and output file names are the same."));
         exit (1);
    }
}

/* process audio file */
static void process_file (setk_options * args)
{
  /* parse command line arguments */
  parse_arguments(args);
  
  SNDFILE * input_file, * output_file;
  SF_INFO sfinfo;
  
  /* open input file */
    if ((input_file = sf_open(args -> input_filename, SFM_READ, &sfinfo)) == NULL) {
          printf(_("Error: Unable to open input file '%s': %s\n"), args -> input_filename, sf_strerror (NULL));
          sf_close (input_file);
          exit (1);
     }

    do {
    
    /* compute window size and make it even */
    (((args -> nwind) = WINDOW_SIZE (args -> frame_duration, sfinfo.samplerate)) % 2 != 0) ? (args -> nwind) +=1 : (args -> nwind);
    
      /* if computed optimal FFT size is greater than FFT_MAX, decrease
       * frame_duration by one millisecond - repeated until FFT size <= FFT_MAX */
      if ( (args -> fft_size) == 0 || (args -> fft_size) > FFT_MAX ) {
        args -> fft_size = OPTIMAL_FFT_SIZE(args -> nwind);
      
      if ((args -> fft_size) > FFT_MAX)
            args -> frame_duration -= 1;
      }
    
    } while ( (args -> fft_size) > FFT_MAX );
  
    /* Force output to mono. */
    if ((args -> downmix) == true) {
         sfinfo.channels = 1;
    }
  
    /* open output file */
    if ((output_file = sf_open(args -> output_filename, SFM_WRITE, &sfinfo)) == NULL) {
          printf(_("Error: Unable to open output file '%s': %s\n"), args -> output_filename, sf_strerror (NULL));
          sf_close (output_file);
          exit (1);
     }
     
     /* print file info */
     if (args -> verbosity == true)
         file_info (args);
     
     sf_set_string (output_file, SF_STR_TITLE, args -> output_filename);
     sf_set_string (output_file, SF_STR_COMMENT, "Enhanced audio signal");
     sf_set_string (output_file, SF_STR_SOFTWARE, "Speech Enhancement Toolbox");
     sf_set_string (output_file, SF_STR_COPYRIGHT, "No copyright.");
     
     enhance_audio(input_file, output_file, args -> nwind, args -> overlap_percentage, args -> downmix);

     sf_close(output_file);
     sf_close(input_file);
  
}

/* print file info */
static void file_info(setk_options * args)
 {
   printf(_("-----------------------------------------\n"));
   printf(_("I N F O R M A T I O N :\n"));
   printf(_("-----------------------------------------\n"));
   printf(_("Input File: %s\n"), args -> input_filename);
   printf(_("Frame Duration: %d\n"), args -> frame_duration);
   printf(_("Window Size: %d\n"), args -> nwind);
   printf(_("Overlap: %d %%\n"), args -> overlap_percentage);
   printf(_("Number of FFT samples: %d\n"), args -> fft_size);
   printf(_("Output File: %s\n"), args -> output_filename);
   printf(_("Downmix to mono: %s\n"), istrue_bool(args -> downmix));
   printf(_("Output Format: same as source\n"));
   printf(_("-----------------------------------------\n\n"));
}

