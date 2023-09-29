
typedef ac_fixed<32, 16, true> image_type;
typedef ac_fixed<32, 16, true> weight_type;
typedef ac_fixed<32, 16, true> bias_type;
typedef ac_int<32, false>      address_type;
typedef ac_int<17, true>       index_type; 


#define KERNEL_HEIGHT        7
#define KERNEL_WIDTH        20

#define IMAGE_HEIGHT       101
#define IMAGE_WIDTH         20

#define ESP_IMAGE_SIZE     (IMAGE_HEIGHT * IMAGE_WIDTH)
#define ESP_KERNEL_SIZE    (KERNEL_HEIGHT * KERNEL_WIDTH)
#define ESP_OUTPUT_SIZE    (IMAGE_HEIGHT - KERNEL_HEIGHT + 1)
#define ESP_BIAS_SIZE      200

#define ESP_MULTIPLIERS      2

#define DMA_SIZE             3 
                                // as per ESP definition

struct conf_info_t {
   address_type   image_address;
   address_type   weight_address;
   address_type   bias_address;
   address_type   output_address;
   index_type     num_input_images;
   index_type     num_output_images;
   index_type     height;
   index_type     width;
   index_type     filter_height;
   index_type     filter_width;
   bool           relu;
   bool           bias;
};

struct dma_info_t {
   ac_int<32, false> index;
   ac_int<32, false> length;
   ac_int<3, false> size;
};

typedef ac_int<32, false> dma_data_t;


void convolve(
   image_type     image_memory[ESP_IMAGE_SIZE],
   weight_type    weight_memory[ESP_KERNEL_SIZE],
   image_type     output_memory[ESP_OUTPUT_SIZE],
   bool           first)
{
   weight_type    coefficient_regs[KERNEL_HEIGHT][KERNEL_WIDTH];  // in catapult, pin these to registers
   image_type     image_slice[KERNEL_HEIGHT][IMAGE_WIDTH];       // in catapult, pin these to registers
   image_type     sum;

   // load registers

  #pragma hls_unroll
   for (int row=0; row<KERNEL_HEIGHT; row++) {
     #pragma hls_unroll
      for (int col=0; col<KERNEL_WIDTH; col++) {
         coefficient_regs[row][col] = weight_memory[row * KERNEL_WIDTH + col];
         image_slice[row][col] = image_memory[row * IMAGE_WIDTH + col];
      }
   }

   for (int image_row=KERNEL_HEIGHT; image_row<IMAGE_HEIGHT+1; image_row++) { // for each row
      // perform the multiplication
      sum = 0.0;
     #pragma hls_unroll
      for (int row=0; row<KERNEL_HEIGHT; row++) {
         for (int col=0; col<KERNEL_WIDTH; col++) {
            sum += image_slice[row][col] * coefficient_regs[row][col];

            int image_index = (image_row + row - KERNEL_HEIGHT) * IMAGE_WIDTH + col;
            int weight_index = (row * KERNEL_WIDTH) + col;
            int im_row = image_row + row - KERNEL_HEIGHT;
            int im_col = col;
            float image_value = image_slice[row][col].to_double();
            float weight_value = coefficient_regs[row][col].to_double(); 
//          printf("HW image_index: %d weight_index: %d image_value[%d][%d]: %5.3f weight_value: %5.3f = %5.3f \n",
//                     image_index, weight_index, im_row, im_col, image_value, weight_value, image_value * weight_value);

         }
      }

//    printf("output[%d] = %6.2f \n", image_row - KERNEL_HEIGHT * ESP_OUTPUT_SIZE, sum.to_double());
      if (first) output_memory[image_row-KERNEL_HEIGHT]  = sum;
      else       output_memory[image_row-KERNEL_HEIGHT] += sum;

      if (image_row < IMAGE_HEIGHT) {
         // shift the image up
        #pragma hls_unroll
         for (int row=0; row<KERNEL_HEIGHT-1; row++) {
            for (int col=0; col<KERNEL_WIDTH; col++) {
               image_slice[row][col] = image_slice[row+1][col];
            }
         }
        #pragma hls_unroll
         for (int col=0; col<KERNEL_WIDTH; col++) {
            image_slice[KERNEL_HEIGHT-1][col] = image_memory[image_row * KERNEL_WIDTH + col];
         }
      }
   }
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
   dma_info_t  dma_read_info;
   dma_data_t  datum;

   dma_read_info.index =   address;
   dma_read_info.length =  size;
   dma_read_info.size =    DMA_SIZE;

   do {
      dma_done = dma_ctrl.nb_write(dma_read_info); 
   } while (!dma_done);

   if (dma_done) {  // force serialization between config and data transfer
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
   
void CCS_BLOCK(conv2d_esp_hw)(
   ac_channel<conf_info_t> &conf_info,
   ac_channel<dma_info_t>  &dma_read_ctrl,
   ac_channel<dma_info_t>  &dma_write_ctrl,
   ac_channel<dma_data_t>  &dma_read_chnl,
   ac_channel<dma_data_t>  &dma_write_chnl,
   ac_sync                 &done)
{
   image_type  image_memory  [ESP_IMAGE_SIZE];
   weight_type weight_memory [ESP_KERNEL_SIZE];
   bias_type   bias_memory   [ESP_BIAS_SIZE];
   image_type  output_memory [ESP_OUTPUT_SIZE];

   // sync, and get input parameters

   conf_info_t regs = conf_info.read();

   // get biases 
   
   dma_load<bias_type>(bias_memory, regs.bias_address, regs.num_output_images, dma_read_ctrl, dma_read_chnl);
   
   for (ac_int<8, false> i=0; i<regs.num_input_images; i++) { // for each input image
      
      // get input image (used repeatedly)
   
      dma_load<image_type>(image_memory, regs.image_address + i * ESP_IMAGE_SIZE, ESP_IMAGE_SIZE, dma_read_ctrl, dma_read_chnl);

      for (ac_int<8, false> o=0; o<regs.num_output_images; o++) { // for each output image

         if (i>0) dma_load<image_type>(output_memory, regs.output_address + o * ESP_OUTPUT_SIZE, ESP_OUTPUT_SIZE, dma_read_ctrl, dma_read_chnl);

         dma_load<weight_type>(weight_memory, regs.weight_address + (i * regs.num_output_images + o) * ESP_KERNEL_SIZE, ESP_KERNEL_SIZE, dma_read_ctrl, dma_read_chnl);
      
         convolve(image_memory, weight_memory, output_memory, i==0); // sizes are fixed!!

         if (regs.bias) {
            for (int i=0; i<ESP_OUTPUT_SIZE; i++) {
               output_memory[i] += bias_memory[o];
            }
         }

         if (regs.relu && ((regs.num_input_images - 1) == i)) {
            for (int i=0; i<ESP_OUTPUT_SIZE; i++) {
               if (output_memory[i]<0.0) output_memory[i] = 0.0;
            }
         }
         
         dma_store<image_type>(output_memory, regs.output_address + o * ESP_OUTPUT_SIZE, ESP_OUTPUT_SIZE, dma_write_ctrl,dma_write_chnl);
      }
   }

   done.sync_out();
}
