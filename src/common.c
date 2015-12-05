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

#include "config.h"
#include "common.h"

/* parse configuration file */
int parse_line(char *line, const char *split, pl_rule *rules, void *data)
{
  unsigned int r, len;
  char *end = NULL, *val = NULL, *p = NULL;
  void *store;

  /* Chop off \n and \r and white space */
  p = &line[strlen(line)-1];
  while (p >= line && (
         *p == '\n' ||
         *p == '\r' ||
         *p == '\t' ||
         *p == ' ')) *p-- = '\0';

  /* Ignore comments and emtpy lines */
  if (strlen(line) == 0 ||
      line[0] == '#' ||
      line[0] == ';' ||
      (line[0] == '/' && line[1] == '/'))
  {
  return 1;
 }

  /* Get the end of the first argument */
  p = line;
  end = &line[strlen(line)-1];
  /* Skip until whitespace */
  while (p < end &&
         strncmp(p, split, strlen(split)) != 0) p++;
  /* Terminate this argument */
  *p = '\0';
  p++;

  /* Skip whitespace */
  while ( p < end &&
         *p == ' ' &&
         *p == '\t') p++;

  /* Start of the value */
  val = p+(strlen(split)-1);

  /* If starting with quotes, skip until next quote */
  if (*p == '"' || *p == '\'') {
       p++;
       /* Find next quote */
       while ( p <= end &&
              *p != *val &&
              *p != '\0') p++;
       /* Terminate */
       *p = '\0';
       /* Skip the first quote */
       val++;
     }
  /* Otherwise it is already terminated above */

  /* Walk through all the rules */
  for (r = 0; rules[r].type != PLRT_END; r++) {
       len = (int)strlen(rules[r].title);
       if (strncmp(line, rules[r].title, len) != 0) continue;

       store = (void *)((char *)data + rules[r].offset);

       switch (rules[r].type) {
         case PLRT_STRING:
               *((const char **)store) = strdup(val);
         break;

         case PLRT_INTEGER:
           *((int *)store) = atoi(val);
         break;

         case PLRT_BOOL:
           if (strcmp(val, "yes") == 0 ||
               strcmp(val, "true") == 0) {
                 *((bool *)store) = true;
            }
           else if (strcmp(val, "no") == 0 ||
             strcmp(val, "false") == 0) {
               *((bool *)store) = false;
           }
           else {
             printf(_("Unknown boolean value \"%s\" for option \"%s\"\n"), val, rules[r].title);
           }
         break;

        case PLRT_END:
         return 0;
        }
        return 1;
      }
    return 0;
}

/* sfx_mix_mono_read_double */
sf_count_t sfx_mix_mono_read_double (SNDFILE * file, double * data, sf_count_t datalen)
{
  SF_INFO info;

#if HAVE_SF_GET_INFO
  /*
  ** The function sf_get_info was in a number of 1.0.18 pre-releases but was removed
  ** before 1.0.18 final and replaced with the SFC_GET_CURRENT_SF_INFO command.
  */
        sf_get_info (file, &info);
#else
        sf_command (file, SFC_GET_CURRENT_SF_INFO, &info, sizeof (info));
#endif

        if (info.channels == 1)
             return sf_read_double (file, data, datalen);

        static double multi_data [2048];
        int k, ch, frames_read;
        sf_count_t dataout = 0;

        while (dataout < datalen) {
                int this_read;

                this_read = MIN (ARRAY_LEN (multi_data) / info.channels, datalen);

                frames_read = sf_readf_double (file, multi_data, this_read);
                if (frames_read == 0)
                      break;

                for (k = 0; k < frames_read; k++) {
                        double mix = 0.0;

                        for (ch = 0; ch < info.channels; ch++)
                               mix += multi_data [k * info.channels + ch];
                        data [dataout + k] = mix / info.channels;
                 };

                 dataout += this_read;
          };

         return dataout;
}

/* separate_channels */
int separate_channels_double (double * multi_data, double * single_data, int frames, int channels, int channel_number)
{
   int k;
   
   if (channel_number > channels) {
        fprintf(stderr, _("This recording has only %u channels."), channels);
        exit (1);
   }
   
   for (k = 0; k < frames; k++)
         single_data [k] = multi_data [k * channels + channel_number];

   return 0;
}

/* combine_channels_double */
int combine_channels_double (double * multi_data, double * single_data, int frames, int channels, int channel_number)
{
   int k;
   
   if (channel_number > channels) {
        fprintf(stderr, _("This recording has only %u channels."), channels);
        exit (1);
    }
   
    for (k = 0; k < frames; k++)
          multi_data [k * channels + channel_number] = single_data [k];

   return 0;
}

/* enhance audio file */
int enhance_audio (SNDFILE * input_file, SNDFILE * output_file, int window_size, int overlap, bool downmix)
{
  SF_INFO info;
  int noverlap, nslide, channels, ch;
  sf_count_t count, frames_read;
  double * multi_data, * prev_multi_data, * buffer, * enhanced_multi_data;
  frames_read = 0;
  sf_count_t (* ptr_read_dbl)() = sf_readf_double;
  
  #if HAVE_SF_GET_INFO
       /*
       **  The function sf_get_info was in a number of 1.0.18 pre-releases but was removed
       **  before 1.0.18 final and replaced with the SFC_GET_CURRENT_SF_INFO command.
       */
       sf_get_info (input_file, &info);
#else
       sf_command (input_file, SFC_GET_CURRENT_SF_INFO, &info, sizeof (info));
#endif
   
   if (downmix == true) {
     /* if downmix is enabled, use sfx_mix_mono_read_double */
     channels = 1;
     ptr_read_dbl = sfx_mix_mono_read_double;
   }
   else {
     channels = info.channels;
   }
       
   noverlap = floor(window_size * overlap / 100);
   nslide = window_size - noverlap;
   
   /* input buffers */
   multi_data = init_buffer_dbl(window_size * channels);
   prev_multi_data = init_buffer_dbl(noverlap * channels);
   
   /* output buffers */
   buffer = init_buffer_dbl(window_size);
   enhanced_multi_data = init_buffer_dbl(window_size * channels);
   
   do {
      if(frames_read == 0) {
        if ((count = ptr_read_dbl (input_file, multi_data, window_size)) <= 0)
             exit (1);
        memcpy((void *) prev_multi_data, (void *) (multi_data + nslide * channels), sizeof(* multi_data) * noverlap * channels);
      }
      else {
        count = ptr_read_dbl (input_file, (multi_data + noverlap * channels), nslide);
        memcpy((void *) multi_data, (void *) prev_multi_data, sizeof(* prev_multi_data) * noverlap * channels);
        memcpy((void *) prev_multi_data, (void *) (multi_data + nslide * channels), sizeof(* multi_data) * noverlap * channels);
      }
        
        frames_read += count;

        for (ch = 0; ch < channels; ch++) {
              separate_channels_double (multi_data, buffer, window_size, channels, ch);
              combine_channels_double (enhanced_multi_data, buffer, window_size, channels, ch);
        }
        
        sf_writef_double (output_file, enhanced_multi_data, window_size);
   } while (count > 0);
   
   free(multi_data);
   free(prev_multi_data);
   free(enhanced_multi_data);
   free(buffer);
       
  return 0;
}

double * init_buffer_dbl(size_t size)
{
  double * ptr = (double *) malloc (sizeof(* ptr) * size);
  
  if (ptr == NULL) {
     printf(_("\nError: malloc failed: %s\n"), strerror(errno));
     exit (1);
  }
  
   return (ptr);
}
