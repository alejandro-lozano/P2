#include <math.h>
#include "pav_analysis.h"

<<<<<<< HEAD
float compute_power(const float *x, unsigned int N) {
    float pot = 1e-12;
    for(unsigned int n=0; n<N; n++){
     pot += x[n]*x[n];   
=======

float compute_power(const float *x, unsigned int N) {
    float pot = 1e-12;
    for(unsigned int n=0; n<N; n++){
     pot += x[n]*x[n];  
>>>>>>> 82cfc6826bdbcf15d7fee4c2bef143326fd9660c
    }
    return 10*log10(pot/N);
}

float compute_am(const float *x, unsigned int N) {
    float res=0;
    for(int i = 0 ;i < N;i++)
<<<<<<< HEAD
    	res += fabs(x[i]);
        
    return res/N;
}

float compute_zcr(const float *x, unsigned int N, float fm) {
   float res=0;
   for(int i =1; i < N;i++){
   	if((x[i]*x[i-1]<0))
   	    res++;
=======
        res += fabs(x[i]);
       
    return res/N;
}


float compute_zcr(const float *x, unsigned int N, float fm) {
   float res=0;
   for(int i =1; i < N;i++){
    if((x[i]*x[i-1]<0))
        res++;
>>>>>>> 82cfc6826bdbcf15d7fee4c2bef143326fd9660c
   }
   return res*fm/(2*(N-1));
   
}
