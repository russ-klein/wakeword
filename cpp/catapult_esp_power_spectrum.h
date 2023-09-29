#include <stdio.h>

#include "mfcc.h"
#include "power_spectrum.h"

typedef ac_ieee_float32 frames_type;
typedef ac_ieee_float32 power_spectrum_type;
typedef ac_int<32, false>      address_type;
typedef ac_int<17, true>       index_type; 

#define ESP_FRAME_SIZE   320 
#define ESP_POWER_SPEC_SIZE    161 
#define ESP_MAC      3
#define DMA_SIZE             4 
                                // as per ESP definition

struct ps_conf_info_t {
   index_type   frame_address;
   index_type   power_spec_address;
   index_type     framewidth_vector_len;
   index_type     power_spec_vector_len;
   index_type     no_of_frames;
   address_type   mfcc_memory;
};

#include "dma_interface.h"


#include "f_coefficients_hw.h" 


power_spectrum_type power_spectrum_dot_product(
                      frames_type memory[ESP_FRAME_SIZE], 
                      ac_int<32,false> j,
                      ac_int<10,false> framewidth_sz )
{
    ac_float<8,5,6,AC_RND> abs_out_squared;
    ac_fixed<12,1, true,AC_RND, AC_SAT> ac_frames;

    ac_complex<ac_fixed<8, 2, true,AC_RND, AC_SAT> > temp;
    ac_fixed<20,5,true,AC_RND, AC_SAT> product_r = 0;
    ac_fixed<20,5,true,AC_RND, AC_SAT> product_i = 0;
    ac_fixed<32,9,false,AC_RND, AC_SAT> abs_in;
    ac_fixed<20,5,false,AC_RND, AC_SAT> abs_out; 

    ac_ieee_float32 ac_float_frames;
    ac_ieee_float32 ac_float_power_spec;
    ac_complex<ac_fixed<20,5,true,AC_RND, AC_SAT> > product;

    product_r = 0;
    product_i = 0;
    product.set_r(product_r);
    product.set_i(product_i);

    #pragma  hls_pipeline_init_interval 1
    #pragma  hls_unroll 2
    for(int i = 0; i < framewidth_sz; i++)
    {
      ac_frames = memory[i].convert_to_ac_fixed<12,1, true,AC_RND, AC_SAT>( );

      if(i==0 || j==0) {
        product_r = product_r + ac_frames; 
      } else {
        temp = F_coeff[(i-1)+((j.to_int()-1)*(framewidth_sz.to_int()-1))]; 
        product_r = product_r + (ac_frames * temp.real( ));
        product.set_i(product.imag() + (ac_frames * temp.imag( )));
        //product_i = product_i + (ac_frames * temp.imag( ));
      } 
    }

    abs_in = product_r * product_r + product.imag() * product.imag();
    //abs_in = product_r * product_r + product_i * product_i;
    ac_math::ac_sqrt(abs_in, abs_out); 
    abs_out_squared = abs_out * abs_out; 
    ac_float_power_spec = ac_ieee_float<binary32> (abs_out_squared);
    return (ac_float_power_spec);
}

#pragma hls_design top
   
void CCS_BLOCK(power_spectrum_esp_hw)(
   ac_channel<ps_conf_info_t> &conf_info,
   ac_channel<dma_info_t>  &dma_read_ctrl,
   ac_channel<dma_info_t>  &dma_write_ctrl,
   ac_channel<dma_data_t>  &dma_read_chnl,
   ac_channel<dma_data_t>  &dma_write_chnl,
   ac_sync                 &done)
{

   frames_type  frames_memory  [ESP_FRAME_SIZE];
   power_spectrum_type  power_spectrum_memory [ESP_POWER_SPEC_SIZE];

   ps_conf_info_t regs = conf_info.read();
   
   printf(">> number of frames: %d \n", regs.no_of_frames);
   printf(">> esp_frame_size:   %d \n", ESP_FRAME_SIZE);
   printf(">> esp_power_spec_size: %d \n", ESP_POWER_SPEC_SIZE);

   for (ac_int<7, false> f=0; f<regs.no_of_frames; f++) { 

      dma_load_float<frames_type>(frames_memory, regs.mfcc_memory, regs.frame_address+f*regs.framewidth_vector_len, regs.framewidth_vector_len, dma_read_ctrl, dma_read_chnl);

      #pragma  hls_pipeline_init_interval 1
      for(ac_int<8, false> j=0; j<regs.power_spec_vector_len; j++) { 
        
        power_spectrum_memory[j] = power_spectrum_dot_product(frames_memory,j,regs.framewidth_vector_len); 

      }   


      dma_store_float<power_spectrum_type>(power_spectrum_memory, regs.mfcc_memory, regs.power_spec_address+f*regs.power_spec_vector_len, regs.power_spec_vector_len, dma_write_ctrl, dma_write_chnl, dma_read_ctrl, dma_read_chnl); 
 }
   done.sync_out();
}
