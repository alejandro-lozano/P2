#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "pav_analysis.h"
#include "vad.h"

const float FRAME_TIME = 10.0F; /* in ms. */

/* 
 * As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
 * only this labels are needed. You need to add all labels, in case
 * you want to print the internal state in string format
 */

const char *state_str[] = {
  "UNDEF", "S", "V", "INIT", "MAYBE_VOICE", "MAYBE_SILENCE"
};

const char *state2str(VAD_STATE st) {
  return state_str[st];
}

/* Define a datatype with interesting features */
typedef struct {
  float zcr;
  float p;
  float am;
} Features;

/* 
 * TODO: Delete and use your own features!
 */

Features compute_features(const float *x, int N) {
  /*
   * Input: x[i] : i=0 .... N-1 
   * Ouput: computed features
   */
  /* 
   * DELETE and include a call to your own functions
   *
   * For the moment, compute random value between 0 and 1 
   */
  Features feat;
  feat.p = compute_power(x,N);
  feat.zcr = compute_zcr(x, N, 16000);
  feat.am = compute_am(x, N);

  return feat;
}

/* 
 * TODO: Init the values of vad_data
 */

VAD_DATA * vad_open(float rate, float alfa0, float alfa1, float alfa2) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;

  vad_data->alfa0 = alfa0;
  vad_data->alfa1 = alfa1;
  vad_data->alfa2 = alfa2;
  vad_data->llindar0 = 0;
  vad_data->llindar1 = 0;
  vad_data->contador = 0;

  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {
  /* 
   * TODO: decide what to do with the last undecided frames
   */
  VAD_STATE state = vad_data->state;

  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

/* 
 * TODO: Implement the Voice Activity Detection 
 * using a Finite State Automata
 */

VAD_STATE vad(VAD_DATA *vad_data, float *x) {

  /* 
   * TODO: You can change this, using your own features,
   * program finite state automaton, define conditions, etc.
   */

  Features f = compute_features(x, vad_data->frame_length); 
  vad_data->last_feature = f.p; /* save feature, in case you want to show */


  float tiempo_MAYBE = FRAME_TIME*vad_data->contador*1e-3; //tiempo que usaremos para ver cuanto
                                                            //hemos estado en un estado MAYBE

  switch (vad_data->state) {
  case ST_INIT:
  // inicializamos llindars 1 y 2 para su uso posterior y cambiamos el estado a silencio, de acuerdo con las instrucciones
    vad_data->llindar0 = f.p + vad_data->alfa0; 
    vad_data->llindar1 = vad_data->llindar0 + vad_data->alfa1; //hacemos que el umbral1 sea más grande que el umbral0
    vad_data->llindar2 = vad_data->llindar1 + vad_data->alfa2;
    vad_data->zcr = f.zcr;
    vad_data->state = ST_SILENCE;
    break;

  case ST_SILENCE:
    if (f.p > vad_data->llindar0 || f.zcr > vad_data->zcr - vad_data->alfa2)
      vad_data->state = ST_MAYBE_VOICE;// Nos movemos a MAYBE_VOICE si estamos por encima del umbral o los cruces por cero son altos
    break;

  case ST_VOICE:
    if (f.p < vad_data->llindar1 && vad_data->zcr + vad_data->alfa2 > f.zcr){
      vad_data->state = ST_MAYBE_SILENCE;//Nos movemos a MAYBE_SILENCE si estamos por debajo del umbral o los cruces por cero son bajos
    }
    break;

  case ST_MAYBE_VOICE:
    if(f.p > vad_data->llindar0){
      if(tiempo_MAYBE > 0.06 && f.p > vad_data->llindar2){
        vad_data->state = ST_VOICE;
        vad_data->contador = 0; //Como salimos de maybe actulizamos el contador
      }
      else{
        vad_data->contador ++; //si seguimos entonces añadimos uno al contador de tramas
      }
    }
    else{
      vad_data->state = ST_SILENCE;
      vad_data->contador = 0; //Reinciciamos contador ya que salimos del estado maybe
    }
  break;

  case ST_MAYBE_SILENCE:
    
    if(f.p < vad_data->llindar1){
      if(tiempo_MAYBE > 0.16 && f.p < vad_data->llindar0){
        
        vad_data->state = ST_SILENCE;
        vad_data->contador = 0; //Como salimos de maybe actulizamos el contador
        
      }
      
      else{
        vad_data->contador ++; //si seguimos entonces añadimos uno al contador de tramas
      }
    }
    else{
      vad_data->state = ST_VOICE;
      vad_data->contador = 0; //Reinciciamos contador ya que salimos del estado maybe
    }
    
  break;

  case ST_UNDEF:
    break;
  }

  if (vad_data->state == ST_SILENCE ||
      vad_data->state == ST_VOICE || vad_data->state == ST_MAYBE_SILENCE ||
      vad_data->state == ST_MAYBE_VOICE)
    return vad_data->state;
  else
    return ST_UNDEF;
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}
