
#include "catapult_memory_master.h"
#include "mfcc.h"
#include "power_spectrum.h"

static ac_complex<ac_fixed<8, 2, true,AC_RND, AC_SAT> > F_coeff[51040] = { 
     #include "f_coefficients_hw.h" 
};

hw_cat_type power_spectrum_dot_product(raw_memory_line *memory, index_type j)
{
    hw_cat_type     hw_cat_power_spec;

    ac_float<8,5,6,AC_RND> abs_out_squared;
    ac_fixed<32,1, true,AC_RND, AC_SAT> ac_frames;

    ac_complex<ac_fixed<8, 2, true,AC_RND, AC_SAT> > temp;
    ac_fixed<32,4,true,AC_RND, AC_SAT> product_r = 0;
    ac_fixed<32,4,true,AC_RND, AC_SAT> product_i = 0;
    ac_fixed<32,4,false,AC_RND, AC_SAT> abs_in;
    ac_fixed<32,2,false,AC_RND, AC_SAT> abs_out; 

    ac_complex<ac_fixed<32,4,true,AC_RND, AC_SAT> > product;

    product_r = 0;
    product_i = 0;
    #pragma hls_unroll STRIDE
    #pragma  hls_pipeline_init_interval 1
    for(int i = 0; i < FRAME_WIDTH; i++)
    {
      ac_frames = memory[i].convert_to_ac_fixed<32,1, true,AC_RND, AC_SAT>( );
      if(i==0 || j==0) {
        product_r = product_r + ac_frames;
        product.set_r(product_r);
        product.set_i(product_i*-1);
      } else {
        temp = F_coeff[(i-1)+((j.to_int()-1)*(FRAME_WIDTH-1))]; 
        product_r = product_r + (ac_frames * temp.real( ));
        product_i = product_i + (ac_frames * temp.imag( ));
        product.set_r(product_r);
        product.set_i(product_i*-1);
      } 
    }

    abs_in = product_r * product_r + product_i * product_i;
    ac_math::ac_sqrt(abs_in, abs_out); 
    abs_out_squared = abs_out * abs_out;
    hw_cat_power_spec = abs_out_squared;
    return hw_cat_power_spec;
}

#pragma hls_design top
void catapult_power_spectrum(
                 cat_memory_type   &debug_signal,
                 ac_channel<bool>  &go,
                 ac_channel<bool>  &done,
                 raw_bus_type       memory[0x10000],
                 index_type         frames_offset,
                 index_type         power_spec_offset)
{

    hw_cat_type  power_spectrum_out_buffer[STRIDE];

    raw_memory_line frames_mem[(20000 + (STRIDE -1))/STRIDE];
    raw_memory_line power_spec_out_mem[(1000 + (STRIDE -1))/STRIDE];

    index_type ps_index = N_BY_2PLUS1;
    static const index_type stride = STRIDE;

    go.read();

   for (index_type i=0; i<N_FRAMES; i++)
   { 
      load_from_system_memory(memory, frames_offset, i, frames_mem, 0);

      for(index_type j=0; j<ps_index; j++) { 
        power_spectrum_out_buffer[j] = power_spectrum_dot_product (frames_mem, j);
      } 
      copy_from_regs(power_spec_out_mem, i, power_spectrum_out_buffer, 0, stride);
   }

   store_into_system_memory(power_spec_out_mem, 0, ps_index, memory, output_offset);

   one.write(1);
}
