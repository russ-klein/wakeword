#include "tb_functions.h"

#include "mc_scverify.h"

#define FRAME_LEN 32320
#define OUT_LEN    16261 

/*

////
// This testbench contains golden input and golden output values for test zero in header files golden_in0_frames_hw.h and golden_out0_ps_hw.h
// This testbench also contains power_spectrum_sw() function call from software which can be used as golden. Either can be used for testing. 
////

*/

CCS_MAIN(int argc, char **argv)
{
// Comment golden_in0_frames_hw.h and golden_out0_ps_hw.h if you want to use power_spectrum_sw(frames,sw_output) as golden;
    #include "golden_in0_frames_hw.h"
    #include "golden_out0_ps_hw.h"

// Uncomment frames and sw_output if you want to use power_spectrum_sw(frames,sw_output) as golden;
//    float frames[FRAME_LEN];
//    float sw_output[OUT_LEN];

    float hw_output[OUT_LEN];

    //static ac_std_float<32,8> hw_memory[0x10000];
    static ac_ieee_float32 hw_memory[0x10000];
    static float           sw_memory[0x1000000];

    int r;

    int frames_offset   = 0;
    int power_spec_offset  = frames_offset   + FRAME_LEN;

    initialize_test_data(frames, sizeof(frames)/sizeof(frames[0]));
    
    //GOLDEN SW
   power_spectrum_sw(frames,sw_output);  
   
   for(int i=0; i < FRAME_LEN; i++){
     hw_memory[frames_offset + i] = frames[i];
   }

   //DUT
   CCS_DESIGN(power_spectrum_hw)(hw_memory, frames_offset, power_spec_offset);

   for(int i=0; i < OUT_LEN; i++){
     hw_output[i] = hw_memory[power_spec_offset+i].to_float();
   }

    r = compare_results(sw_output, hw_output, OUT_LEN);

    if (r) {
       printf("Failure!\n");
    } else {
       printf("Success!\n");
    }    
   
    CCS_RETURN(0);
}
