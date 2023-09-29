#include "mfcc.h"

float hw_modified_fbank[3220] = {  
                       #include "fbank.h" 
                       };

float hw_modified_window[320] =  {  
                       #include "window.h" 
                        };
float hw_modified_cos_array[400] = {  
                       #include "cos_array.h" 
                       };
float hw_modified_cepstral_lifter[20] = {  
                       #include "cepstral_lifter.h" 
                       };

void hw_modified_preemphasis(float *input, float *output){
 output[0] = input[0];
 for(int i=1; i<SAMPLE_SIZE; i++){
  output[i] = (input[i] - ALPHA * input[i-1]);
 }
}

void hw_modified_edge_padding(float *input, float *output){
 for (int i= N_BY_2; i< SAMPLE_SIZE + N_BY_2; i++){
  output[i]= input[i - N_BY_2];
 }

 for (int i=0; i< N_BY_2; i++){
  output[N_BY_2-1-i]= input[i+1];
  output[SAMPLE_SIZE + N_BY_2 + i] = input[SAMPLE_SIZE-2-i];
 }
}

void hw_modified_to_frames(float *input, float *hw_modified_window, float *output){
 for(int j = 0; j<N_FRAMES; j++){
  for(int i = 0; i<FRAME_WIDTH; i++){
   output[FRAME_WIDTH*j+i] = input[j*MFCC_STRIDE + i] * hw_modified_window[i]; 
  }
 }
}

void hw_modified_func_sum(float *input, float *log_output){
 for(int j = 0; j<N_FRAMES; j++){
  float accum = 0; 
  for(int i = 0; i<N_BY_2PLUS1; i++){
   accum += input[N_BY_2PLUS1*j+i]; 
  }
  if(accum == 0){
   accum = MIN_POSVAL;
  }
  log_output[j] = log(accum);
 }   
}

void hw_modified_func_mean(float *input, float *output){
 for(int i = 0; i < N_FILTERS; i++){
  for(int j=0; j< N_FRAMES; j++){
   output[i] += input[j*N_FILTERS+i];
  }
  output[i]=output[i]/N_FRAMES;
 }
}

void mfcc_hw(float *mfcc_memory, float *waveform, float *spectrogram){

//MEL spectrogram
 float after_preemphasis[SAMPLE_SIZE];
 hw_modified_preemphasis(waveform, after_preemphasis);

 float padded_signal[PADDED_SIGNAL_SZ];  
 hw_modified_edge_padding(after_preemphasis, padded_signal);

 float frames[X_TOTAL];
 hw_modified_to_frames(padded_signal, hw_modified_window, frames); 

 int frames_offset = 0;
 int power_spec_offset = X_TOTAL;

 int frame_sz = X_TOTAL;
 int powerspec_sz = PS_TOTAL;
 float power_spec[PS_TOTAL];
 
//HW FUCNTIONS 
 memcpy(mfcc_memory+frames_offset, frames, sizeof(frames)); 
 power_spectrum_hw(mfcc_memory, frames_offset, power_spec_offset);
 memcpy(power_spec, mfcc_memory+power_spec_offset, sizeof(power_spec));

 float log_frame_energy[N_FRAMES];
 hw_modified_func_sum(power_spec, log_frame_energy);

 float filter_energies[FE_TOTAL]={0};
 //filter_energies = power_spec @ fbank.T
 for(int i=0; i< N_FRAMES; i++){ 
  for(int j=0; j< N_FILTERS; j++){
   for(int k=0; k< N_BY_2PLUS1; k++){
    filter_energies[(i*N_FILTERS)+j] += power_spec[i*N_BY_2PLUS1 + k] * hw_modified_fbank[j*N_BY_2PLUS1 + k];
   }
    if(filter_energies[(i*N_FILTERS)+j] == 0){
     filter_energies[(i*N_FILTERS)+j] = MIN_POSVAL;
    }
    filter_energies[(i*N_FILTERS)+j] = 10 * log10(filter_energies[(i*N_FILTERS)+j]);
  }  
 }

//DCT
float mfccs[N_FILTERS*N_FILTERS]= {0};
for(int f =0; f < N_FRAMES; f++){
 float out[N_FILTERS]= {0};
 for(int k=0; k< N_FILTERS; k++){
  for(int n=0; n< N_FILTERS; n++){
   out[k] += filter_energies[f*N_FILTERS+n] * hw_modified_cos_array[k*20+n];
  }
  if(k==0){
   out[k] = out[k] * 2 * (sqrt(NFILTERS_X4));
  }else{
   out[k] = out[k] * 2 * (sqrt(NFILTERS_X2));
  }
  mfccs[N_FILTERS*f+k] = out[k];
 }
}

for(int i =0; i < N_FRAMES; i++){
 for(int j =0; j < N_FILTERS; j++){
  mfccs[N_FILTERS*i+j] = mfccs[N_FILTERS*i+j] * hw_modified_cepstral_lifter[j];
 }
}

float mfccs_mean[N_FILTERS]={0};
hw_modified_func_mean(mfccs, mfccs_mean);

for(int i=0; i<N_FRAMES; i++){
 for(int j=0; j<N_FILTERS; j++){
 mfccs[i*N_FILTERS+j] = mfccs[i*N_FILTERS+j] - mfccs_mean[j];
 }
}

//replace_intercept - the 0th MFCC coefficient doesn't tell us anything about the spectrum replace it with the log of the frame energy for something more informative.
for(int i=0; i<N_FRAMES; i++){
 mfccs[i*N_FILTERS] = log_frame_energy[i];
}

for (int i=0; i< N_FRAMES * N_FILTERS; i++){
  spectrogram[i] = mfccs[i];
}
 
}


