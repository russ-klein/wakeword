#ifndef __POWER_SPECTRUM_H__INCLUDED__
#define __POWER_SPECTRUM_H__INCLUDED__

#include <ac_int.h>
#include <ac_std_float.h>
#include "ac_int.h"
#include "ac_fixed.h"
#include "ac_math.h"
#include <ac_math/ac_hcordic.h>
#include "ac_complex.h"
#include <complex>   

#include "ac_sync.h"
#include "mc_scverify.h"


#include "ac_channel.h"


void power_spectrum_sw(
                       float *frames, 
                       float *power_spec);

void power_spectrum_hw(
                      float *memory,
                      int frames_offset,
                      int power_spec_offset);

void power_spectrum_hw(
                      ac_ieee_float32 memory[0x10000],
                      ac_int<32,false> frames_offset,  
                      ac_int<32,false> power_spec_offset);

#endif
