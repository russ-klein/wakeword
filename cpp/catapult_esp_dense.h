
typedef ac_fixed<32, 16, true> image_type;
typedef ac_fixed<32, 16, true> weight_type;
typedef ac_fixed<32, 16, true> bias_type;
typedef ac_int<32, false>      address_type;
typedef ac_int<17, true>       index_type; 


#define ESP_IMAGE_SIZE   17670
#define ESP_OUTPUT_SIZE    128

#define ESP_MULTIPLIERS      4

/*
#define DMA_SIZE             3 
                                // as per ESP definition

struct conf_info_t {
   address_type   image_address;
   address_type   weight_address;
   address_type   bias_address;
   address_type   output_address;
   index_type     input_vector_len;
   index_type     output_vector_len;
};

struct dma_info_t {
   ac_int<32, false> index;
   ac_int<32, false> length;
   ac_int<3, false> size;
};

typedef ac_int<32, false> dma_data_t;
*/

#include "dma_interface.h"

image_type vector_multiply(
   index_type     size,
   image_type     image_memory[ESP_IMAGE_SIZE],
   weight_type    weight_memory[ESP_IMAGE_SIZE], int base)
{
   index_type count =   0;
   image_type sum   = 0.0;

   do {
     #pragma unroll ESP_MULTIPLIERS
      for (int i=0; i<ESP_MULTIPLIERS; i++) {
         if ((count + i) < size) {
            sum += image_memory[count + i] * weight_memory[count + i];
/*
            printf("HW image_value[%6d]: %6.3f weight_value[%6d]: %6.3f product: %6.3f sum: %6.3f \n",
                   count+i, image_memory[count + i].to_double(), 
                   (count.to_int() + i) + base, weight_memory[count + i].to_double(),
                   (image_memory[count + i] * weight_memory[count + i]).to_double(),
                   sum.to_double());
*/
         }
      }
      count += ESP_MULTIPLIERS;
   } while (count < size);

   return sum;
}


template<typename ARRAY_TYPE>
void dma_load(
   ARRAY_TYPE  *memory,
   address_type  address,
   index_type    size,
   ac_channel<dma_info_t> &dma_ctrl,
   ac_channel<dma_data_t> &dma_data)
{
   bool        dma_done       = false;
   dma_info_t  dma_read_info;
   dma_data_t  datum;

   dma_read_info.index =   address;
   dma_read_info.length =  size;
   dma_read_info.size =    DMA_SIZE;

   do {
      dma_done = dma_ctrl.nb_write(dma_read_info);  
   } while (!dma_done);

   if (dma_done) {  // force serialization between config and data transfer
      for (index_type i=0; i<size; i++) {
         datum = dma_data.read();
         memory[i].set_slc(0, datum.slc<32>(0));
      }
   }
}


template<typename ARRAY_TYPE>
void dma_store(
   ARRAY_TYPE   *memory,
   address_type  address,
   index_type    size,
   ac_channel<dma_info_t> &dma_ctrl,
   ac_channel<dma_data_t> &dma_data)
{
   bool        dma_done       = false;
   dma_info_t  dma_write_info;
   dma_data_t  datum;

   dma_write_info.index =   address;
   dma_write_info.length =  size;
   dma_write_info.size =    DMA_SIZE;

   do {
      dma_done = dma_ctrl.nb_write(dma_write_info); 
   } while (!dma_done);

   if (dma_done) {  // force serialization between config and data transfer
      //for (int i=0; i<IMAGE_SIZE; i++) {
      for (int i=0; i<size; i++) {
         // pack value into bus type
         datum = 0;
         // copy bitwise from ac_float* to ac_uint32
         for (int j=0; j<32; j++) datum[j] = memory[i][j];
         dma_data.write(datum);
         if (i>= size) break;
      }
   }
}


#pragma hls_design top
   
void CCS_BLOCK(dense_esp_hw)(
   ac_channel<conf_info_t> &conf_info,
   ac_channel<dma_info_t>  &dma_read_ctrl,
   ac_channel<dma_info_t>  &dma_write_ctrl,
   ac_channel<dma_data_t>  &dma_read_chnl,
   ac_channel<dma_data_t>  &dma_write_chnl,
   ac_sync                 &done)
{
   image_type  image_memory  [ESP_IMAGE_SIZE];
   weight_type weight_memory [ESP_IMAGE_SIZE];
   bias_type   bias_memory   [ESP_OUTPUT_SIZE];
   image_type  output_memory [ESP_OUTPUT_SIZE];

   // sync, and get input parameters

   conf_info_t regs = conf_info.read();

   // get input image (used repeatedly)
   
   dma_load<image_type>(image_memory, regs.image_address, regs.input_vector_len, dma_read_ctrl, dma_read_chnl);

   // get biases 
   
   dma_load<bias_type>(bias_memory, regs.bias_address, regs.output_vector_len, dma_read_ctrl, dma_read_chnl);
   
   for (ac_int<8, false> i=0; i<regs.output_vector_len; i++) {
      
      dma_load<weight_type>(weight_memory, regs.weight_address, regs.input_vector_len, dma_read_ctrl, dma_read_chnl);

      output_memory[i] = vector_multiply(regs.input_vector_len, image_memory, weight_memory, i * regs.input_vector_len) + bias_memory[i];
   }

   dma_store<image_type>(output_memory, regs.output_address, regs.output_vector_len, dma_write_ctrl, dma_write_chnl);

   done.sync_out();
}
