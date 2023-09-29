#include <stdio.h>

#include "defines.h"
#include "cat_access.h"
#include "dense.h"

void dense_sw(
              float *input_image,
              float *weights,
              float *biases,
              float *output_image,
              int num_units,
              int unit_count,
              int output_image_elements,
              int relu,
              int bias)
{

    int i, n, c;
    float sum;
    float bias_value;
    int chatty = 0;

    for (i=0; i<output_image_elements; i++) {
        sum = 0.0;
        for (n=0; n<num_units; n++) {
            for (c=0; c<unit_count; c++) {
                sum += input_image[n * unit_count + c] * weights[(i*num_units*unit_count)+n*unit_count+c];
                if (chatty) printf("SW image_value[%6d]: %6.3f weight_value[%6d]: %6.3f product: %6.3f sum: %6.3f \n",
                       n * unit_count + c, input_image[n * unit_count + c], 
                       (i*num_units*unit_count) + n * unit_count + c, weights[(i*num_units*unit_count)+n*unit_count+c], 
                       input_image[n * unit_count + c] * weights[(i*num_units*unit_count)+n*unit_count+c], sum);
            }
        }
        if (bias) {
            bias_value = biases[i];
            output_image[i] = sum + bias_value;
        } else {
            output_image[i] = sum;
        }
        if (relu) {
            if (output_image[i] <0) output_image[i] = 0;
        }
    }
}

#ifdef HOST
#ifdef ALGORITHMIC_VERSION

void dense_hw(
              cat_memory_type *memory,
              int input_image,
              int weights,
              int biases,
              int output_image,
              int num_units,
              int unit_count,
              int output_image_elements,
              int relu,
              int bias)
{

    int i, n, c;
    hw_cat_type sum;
    hw_cat_type bias_value;
    int chatty = 0;

    for (i=0; i<output_image_elements; i++) {
        sum = 0.0;
        for (n=0; n<num_units; n++) {
            for (c=0; c<unit_count; c++) {
                sum += get_cat_value(memory, input_image + n * unit_count + c) *
                       get_cat_value(memory, weights + (i*num_units*unit_count)+n*unit_count+c);
                // sum += input_image[n * unit_count + c] * weights[(i*num_units*unit_count)+n*unit_count+c];
                // if (chatty) printf("image_value: %5.3f weight_value: %5.3f product: %5.3f sum: %5.3f \n",
                //       input_image[n * unit_count + c], weights[(i*num_units*unit_count)+n*unit_count+c], 
                //       input_image[n * unit_count + c] * weights[(i*num_units*unit_count)+n*unit_count+c], sum);
            }
        }
        if (bias) {
            bias_value = get_cat_value(memory, biases + i); // biases[i];
            set_cat_value(memory, output_image + i, sum + bias_value); // output_image[i] = sum + bias_value;
        } else {
            set_cat_value(memory, output_image + i, sum); // output_image[i] = sum;
        }
        if (relu) {
            //if (output_image[i] <0) output_image[i] = 0;
            if (get_cat_value(memory, output_image + i) < 0) set_cat_value(memory, output_image + i, 0); // output_image[i] = 0;
        }
    }
}

#endif // ALGORITHMIC_VERSION

#ifdef ESP_VERSION

#include "catapult_esp_dense.h"

void dense_hw (
              cat_memory_type *memory,
              int input_image,
              int weights,
              int biases,
              int output_image,
              int num_units,
              int unit_count,
              int output_image_elements,
              int relu,
              int bias)
{
   ac_channel<conf_info_t>  conf_info;
   ac_channel<dma_info_t>   dma_read_ctrl;
   ac_channel<dma_info_t>   dma_write_ctrl;
   ac_channel<dma_data_t>   dma_read_chnl;
   ac_channel<dma_data_t>   dma_write_chnl;
   ac_sync                  done;
   conf_info_t              regs;
   int i;

   regs.image_address = input_image;
   regs.weight_address = weights;
   regs.bias_address   = biases;
   regs.output_address = output_image;
   regs.input_vector_len = unit_count;
   regs.output_vector_len = output_image_elements;

   conf_info.write(regs);

   // first write image
    
   for (int i=0; i<unit_count; i++) {
      dma_read_chnl.write(memory[input_image + i]);
   }

   // then biases
   
   for (i=0; i<output_image_elements; i++) {
      dma_read_chnl.write(memory[biases + i]);
   }

   // then weights
   
   for (i=0; i<unit_count * output_image_elements; i++) {
      dma_read_chnl.write(memory[weights + i]);
   }

   // call accelerator
   
   dense_esp_hw(conf_info, dma_read_ctrl, dma_write_ctrl, dma_read_chnl, dma_write_chnl, done);

   // unload results
   
   for (i=0; i<output_image_elements; i++) {
       memory[output_image + i] = dma_write_chnl.read();
   }

   done.sync_in();
}

#endif

#ifdef ARCHITECTURE_VERSION

#include "catapult_dense.h"

void dense_hw(
              cat_memory_type *memory,
              int input_image,
              int weights,
              int biases,
              int output_image,
              int num_units,
              int unit_count,
              int output_image_elements,
              int relu,
              int bias)
{
    ac_channel<bool> go;
    ac_channel<bool> done;

    cat_memory_type debug_signal;

    raw_bus_type *memory_base = memory;

    go.write(1);

    catapult_dense(
               debug_signal,
               go,
               done,
               bias,
               relu,
               memory_base,
               input_image,
               weights,
               biases,
               output_image,
               num_units,
               unit_count,
               output_image_elements);

    done.read();
}

#endif // not ALGORITHMIC_VERSION
#else  // not HOST

// embedded version of dense_hw goes here


void dense_hw(
              cat_memory_type *memory,
              int input_image,
              int weights,
              int biases,
              int output_image,
              int num_units,
              int unit_count,
              int output_image_elements,
              int relu,
              int bias)
{
  return;
}
#endif // not HOST
