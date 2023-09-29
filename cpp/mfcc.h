

#ifndef __MFCC_H__INCLUDED__
#define __MFCC_H__INCLUDED__

#include <stdio.h>
#include "defines.h"
#include "catapult_accel.h"
#include "cat_access.h"
#include "power_spectrum.h"
#include <math.h>
#include <stdlib.h>
using namespace std;
#include <iostream>
#include <limits> 
#include <cmath>
#include <string.h>

#define ALPHA 0.95
#define FRAME_WIDTH 320
#define N FRAME_WIDTH
#define N_BY_2 N/2
#define N_BY_2PLUS1 161
#define MFCC_STRIDE 160
#define N_FILTERS 20
#define SAMPLE_SIZE 16000 
#define PADDED_SIGNAL_SZ 16320
#define N_FRAMES 101 
#define X_TOTAL 32320
#define PS_TOTAL 16261
#define FE_TOTAL 2020
#define MIN_POSVAL 2.22044605e-16 
#define NFILTERS_X4 0.0125
#define NFILTERS_X2 0.025


void mfcc(float *waveform, float *spectrogram);

void mfcc_hw(float *mfcc_memory, float *waveform, float *spectrogram);

#endif
