#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sndfile.h>

#include "vad.h"
#include "vad_docopt.h"

#define DEBUG_VAD 0x1

int main(int argc, char *argv[]) {
  int verbose = 0; /* To show internal state of vad: verbose = DEBUG_VAD; */

  SNDFILE *sndfile_in, *sndfile_out = 0;
  SF_INFO sf_info;
  FILE *vadfile;
  int n_read = 0, i;

  VAD_DATA *vad_data;
  VAD_STATE state, last_state, original_state; //original_state es el último estado de voz o siencio que se ha estado
                                              //Se usa para hacer comparaciones

  float *buffer, *buffer_zeros;
  int frame_size;         /* in samples */
  float frame_duration;   /* in seconds */
  unsigned int t, last_t, start_sv_t; /* in frames */
                          //start_sv_t es el tiempo en el que empieza el silencio o la voz

  char	*input_wav, *output_vad, *output_wav;

  DocoptArgs args = docopt(argc, argv, /* help */ 1, /* version */ "2.0");

  verbose    = args.verbose ? DEBUG_VAD : 0;
  input_wav  = args.input_wav;
  output_vad = args.output_vad;
  output_wav = args.output_wav;

  //alfa0 y alfa1 es un Real, devuelve cadena de texto, llamar función que convierta cad. txt. a Real
  float alfa0 = 5.28;
  float alfa1 = 2.78;
  float alfa2 = 5.2;

  if (input_wav == 0 || output_vad == 0) {
    fprintf(stderr, "%s\n", args.usage_pattern);
    return -1;
  }

  /* Open input sound file */
  if ((sndfile_in = sf_open(input_wav, SFM_READ, &sf_info)) == 0) {
    fprintf(stderr, "Error opening input file %s (%s)\n", input_wav, strerror(errno));
    return -1;
  }

  if (sf_info.channels != 1) {
    fprintf(stderr, "Error: the input file has to be mono: %s\n", input_wav);
    return -2;
  }

  /* Open vad file */
  if ((vadfile = fopen(output_vad, "wt")) == 0) {
    fprintf(stderr, "Error opening output vad file %s (%s)\n", output_vad, strerror(errno));
    return -1;
  }

  /* Open output sound file, with same format, channels, etc. than input */
  if (output_wav) {
    if ((sndfile_out = sf_open(output_wav, SFM_WRITE, &sf_info)) == 0) {
      fprintf(stderr, "Error opening output wav file %s (%s)\n", output_wav, strerror(errno));
      return -1;
    }
  }

  vad_data = vad_open(sf_info.samplerate, alfa0, alfa1, alfa2);
  /* Allocate memory for buffers */
  frame_size   = vad_frame_size(vad_data);
  buffer       = (float *) malloc(frame_size * sizeof(float));
  buffer_zeros = (float *) malloc(frame_size * sizeof(float));
  for (i=0; i< frame_size; ++i) buffer_zeros[i] = 0.0F;

  frame_duration = (float) frame_size/ (float) sf_info.samplerate;
  last_state = ST_UNDEF;
  original_state = ST_SILENCE; 

  for (t = last_t = start_sv_t= 0 ; ; t++) { /* For each frame ... */
    /* End loop when file has finished (or there is an error) */
    if  ((n_read = sf_read_float(sndfile_in, buffer, frame_size)) != frame_size) break;

    if (sndfile_out != 0) {
      /* TODO: copy all the samples into sndfile_out */
      sf_write_float(sndfile_out, buffer, frame_size); ////////////
    }

    state = vad(vad_data, buffer);
    if (verbose & DEBUG_VAD) vad_show_state(vad_data, stdout);
    /* TODO: print only SILENCE and VOICE labels */
    
    if(state != last_state){ 
      if(t !=last_t){
        
        if((original_state==ST_VOICE && state==ST_SILENCE && last_state==ST_MAYBE_SILENCE) || (original_state==ST_SILENCE && state==ST_VOICE && last_state==ST_MAYBE_VOICE)){
        //Vemos si hay cambio de Silencio a Voz o Voz a Silencio

          fprintf(vadfile, "%.5f\t%.5f\t%s\n", start_sv_t * frame_duration, (last_t-1) * frame_duration, state2str(original_state));
          
          if (sndfile_out != 0 && original_state == ST_SILENCE) { ////////////
            /* TODO: go back and write zeros in silence segments */
            sf_seek(sndfile_out, start_sv_t *frame_size, SEEK_SET);
           
            sf_write_float(sndfile_out, buffer_zeros, 161*frame_size);
          }

          start_sv_t = last_t-1;
          original_state = state;
        }
      }

      if(last_state == ST_UNDEF){
        last_state = state;
      }

      last_state = state;
      last_t = t;

    }
    
    
  }

  state = vad_close(vad_data);
  /* TODO: what do you want to print, for last frames? */
  if (t != last_t){
    if(state==ST_VOICE || state==ST_SILENCE){
      
      fprintf(vadfile, "%.5f\t%.5f\t%s\n", start_sv_t * frame_duration, t * frame_duration + n_read / (float) sf_info.samplerate, state2str(state));
      
      if (sndfile_out != 0 && original_state == ST_SILENCE) { ////////////
            /* TODO: go back and write zeros in silence segments */
            sf_seek(sndfile_out, start_sv_t *frame_size, SEEK_SET);
            
            sf_write_float(sndfile_out, buffer_zeros, 161*frame_size);
          }
    
    }
    else{
      if(original_state == ST_SILENCE){
        state = ST_SILENCE;

        if(sndfile_out !=0){
          sf_seek(sndfile_out, start_sv_t *frame_size, SEEK_SET);
          
          sf_write_float(sndfile_out, buffer_zeros, 161*frame_size);
        }

      }
      else{
        state = ST_VOICE;
      }
      fprintf(vadfile, "%.5f\t%.5f\t%s\n", start_sv_t * frame_duration, t * frame_duration + n_read / (float) sf_info.samplerate, state2str(state));

    }
  }
  
    

  /* clean up: free memory, close open files */
  free(buffer);
  free(buffer_zeros);
  sf_close(sndfile_in);
  fclose(vadfile);
  if (sndfile_out) sf_close(sndfile_out);
  return 0;
}
