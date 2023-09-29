
#include "power_spectrum.h"
#include "mfcc.h"

//MFCC SW Version
void power_spectrum_sw(
                       float *frames, 
                       float *power_spec)

{
   const complex<float> F_coeff[51520] = {
     #include "f_coefficients.h"
   };

   complex<float> product = {0,0};

   for(int k =0; k < X_TOTAL/FRAME_WIDTH; k++) {
     for(int j = 0; j < N_BY_2PLUS1; j++) {
       //product = {0,0};
       float product1 = 0.0, product2 = 0.0;
       //float product2 = 0;   
       // Loop for calculate dot product
       for(int i = 0; i < FRAME_WIDTH; i++) {
         complex<float> temp = F_coeff[i+j*FRAME_WIDTH];
         product1 = product1 + ((float)frames[i+k*FRAME_WIDTH] * temp.real());
         product2 = product2 + ((float)frames[i+k*FRAME_WIDTH] * temp.imag());
       }
       product = {product1, -1*product2};
       power_spec[j+k*N_BY_2PLUS1] = (abs(product))*(abs(product));
     }
   }
}

#ifdef HOST
#ifdef ALGORITHMIC_VERSION

void power_spectrum_hw(
                      float *memory,
                      int frames_offset,  
                      int power_spec_offset)
{
    #include "f_coefficients_hw.h"  
/*
    ac_float<8,5,6,AC_RND> abs_out_squared;
    ac_fixed<32,2, true,AC_RND, AC_SAT> ac_frames;

    ac_complex<ac_fixed<15, 2, true,AC_RND, AC_SAT> > temp;
    ac_fixed<32,4,true,AC_RND, AC_SAT> product_r = 0;//checked //24,4
    ac_fixed<32,4,true,AC_RND, AC_SAT> product_i = 0;//checked //24,4
    ac_fixed<64,16,false,AC_RND, AC_SAT> abs_in;
    ac_fixed<32,4,false,AC_RND, AC_SAT> abs_out; //checked //20,4

    ac_ieee_float32 ac_float_frames;
    ac_ieee_float32 ac_float_power_spec;
    ac_complex<ac_fixed<32,4,true,AC_RND, AC_SAT> > product;*/

/////Algorithm works fine for this. MFCC Accuracy goes down as bitwidth decreases. Bit inference works fine even with MFCC lesser accuracy
    ac_float<9,2,6,AC_RND> abs_out_squared;//12
    ac_fixed<20,1, true,AC_RND, AC_SAT> ac_frames;

    ac_complex<ac_fixed<15, 2, true,AC_RND, AC_SAT> > temp;
    ac_fixed<25,5,true,AC_RND, AC_SAT> product_r = 0;
    ac_fixed<25,5,true,AC_RND, AC_SAT> product_i = 0;
    ac_fixed<40,9,false,AC_RND, AC_SAT> abs_in;
    ac_fixed<26,5,false,AC_RND, AC_SAT> abs_out; 

    ac_ieee_float32 ac_float_frames;
    ac_ieee_float32 ac_float_power_spec;
    ac_complex<ac_fixed<25,5,true,AC_RND, AC_SAT> > product;


/*    ac_float<8,5,6,AC_RND> abs_out_squared;
    ac_fixed<32,1, true,AC_RND, AC_SAT> ac_frames;
    ac_complex<ac_fixed<32, 2, true,AC_RND, AC_SAT> > temp;
    ac_fixed<64,14,true,AC_RND, AC_SAT> product_r = 0;
    ac_fixed<64,14,true,AC_RND, AC_SAT> product_i = 0;
    ac_fixed<128,34,false,AC_RND, AC_SAT> abs_in;
    ac_fixed<64,14,false,AC_RND, AC_SAT> abs_out; 

    ac_ieee_float32 ac_float_frames;
    ac_ieee_float32 ac_float_power_spec;
    ac_complex<ac_fixed<64,14,true,AC_RND, AC_SAT> > product;*/

    for(int k =0; k < X_TOTAL/FRAME_WIDTH; k++)
    {    
      for(int j = 0; j < N_BY_2PLUS1; j++)
      {
        product_r = 0;
        product_i = 0;
        product.set_r(product_r);
        product.set_i(product_i);
        for(int i = 0; i < FRAME_WIDTH; i++)
        {
         
          ac_float_frames = memory[i+k*FRAME_WIDTH];
          ac_frames = ac_float_frames.convert_to_ac_fixed<32,2, true,AC_RND, AC_SAT>( );

          if(i==0 || j==0) {
            product_r = product_r + ac_frames; 
          } else {
            temp = F_coeff[(i-1)+((j-1)*(FRAME_WIDTH-1))]; 
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
        memory[power_spec_offset + (j+k*N_BY_2PLUS1)] = ac_float_power_spec.to_float();
      }
    }
}

#endif // ALGORITHMIC_VERSION

#ifdef ESP_VERSION

#include "catapult_esp_power_spectrum.h"

void power_spectrum_hw(
                      ac_ieee_float32 memory[0x10000],
                      ac_int<32,false> frames_offset,  
                      ac_int<32,false> power_spec_offset)
{

   ac_channel<ps_conf_info_t>     conf_info;
   ac_channel<dma_info_t>      dma_read_ctrl;
   ac_channel<dma_info_t>      dma_write_ctrl; 
   ac_channel<dma_data_t>   dma_read_chnl;
//   ac_channel<ac_ieee_float32>   dma_read_chnl;
   ac_channel<dma_data_t>  dma_write_chnl;
//   ac_channel<ac_ieee_float32>  dma_write_chnl;
   ac_sync                     done;
   ps_conf_info_t                 regs;
   int i;

   regs.frame_address = frames_offset;
   regs.power_spec_address = power_spec_offset;
   regs.framewidth_vector_len = FRAME_WIDTH;
   regs.power_spec_vector_len = N_BY_2PLUS1;
   regs.no_of_frames = N_FRAMES;
   regs.mfcc_memory = memory;

//   ac_ieee_float32 delete1, delete2;
   conf_info.write(regs);

   // first write frames

    dma_data_t memory_line;
//int count =0;

   for (int i=0; i<101; i++) {
    for(int j=0; j< 320; j+=2) {
      int index =  i*160+j; //i*320+j;
//      memory_line.set_slc(0, memory[frames_offset/2 + i].data_ac_int());
//      memory_line.set_slc(32, memory[frames_offset/2 + i+1].data_ac_int());
      memory_line.set_slc(0, memory[frames_offset/2 + index].data_ac_int());
      memory_line.set_slc(32, memory[frames_offset/2 + index+1].data_ac_int());
      dma_read_chnl.write(memory_line);
//      count++;
   }
   dma_read_chnl.write(memory_line);
  }

//   cout << "count from power spec=160 " << count << endl;
   


   // call accelerator
   
   power_spectrum_esp_hw(conf_info, dma_read_ctrl, dma_write_ctrl, dma_read_chnl, dma_write_chnl, done);

   // unload power spectrum hw results
  
   for (i=0; i<PS_TOTAL; i+=2) {
//         memory[power_spec_offset+i] = dma_write_chnl.read(); 
       ac_int<64,false> tem = dma_write_chnl.read(); 
//       delete1.d= tem.slc<32> (0);
//       delete2.d= tem.slc<32> (32); 
//       cout << delete1 << endl;
//       cout << delete2 << endl;
       memory[power_spec_offset + i].d = tem.slc<32> (0);
       memory[power_spec_offset + i+1].d = tem.slc<32> (32); 
//       cout << "memory_hw[" << power_spec_offset + i <<"]= " << memory[power_spec_offset + i] << endl;
//       cout << "memory_hw["<< power_spec_offset + i+1 <<"]= " << memory[power_spec_offset + i+1] << endl;
   }

   done.sync_in();


}

#endif //ESP_VERSION

/*
#ifdef ARCHITECTURE_VERSION
#include "catapult_power_spectrum.h"
void power_spectrum_hw(
                      cat_memory_type *memory,
                      int frames_offset, 
                      int power_spec_offset)
{
    ac_fixed<10,1, true,AC_RND, AC_SAT> ac_frames[X_TOTAL]; 
    //ac_float<8,5,6,AC_RND> 
    ac_fixed<64, 16, true> ac_power_spec;//convert to ac float ieee
    for(int i = 0; i < X_TOTAL; i++)
    {
      ac_frames[i] = get_cat_value(memory, frames_offset + i);
    }
{
     ac_channel<bool> go;
     ac_channel<bool> done;
     cat_memory_type debug_signal;
     raw_bus_type *memory_base = memory;
     go.write(1);
     catapult_power_spectrum(
                            debug_signal,
                            go,
                            done,
                            memory_base,
                            frames_offset,
                            power_spec_offset);
// for (int i = 0; i < PS_TOTAL; i = i + 1){
//    set_cat_value(mfcc_memory, power_spec_offset + i, ac_power_spec[i].to_double());//figure what needs to be done here
//}
    done.read();
}
#endif // not ALGORITHMIC_VERSION
#else  // not HOST
// embedded version of dense_hw goes here
void power_spectrum_hw(
                      cat_memory_type *memory,
                      int frames_offset, 
                      int power_spec_offset)
{
  return;
}*/
#endif // not HOST

