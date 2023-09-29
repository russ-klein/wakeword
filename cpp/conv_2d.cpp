#include <stdio.h>

#include "defines.h"
#include "cat_access.h"
#include "conv_2d.h"

static inline int in_bounds(
              int r,
              int c,
              int height,
              int width)
{
    if (r < 0)        return 0;
    if (r >= height)  return 0;
    if (c < 0)        return 0;
    if (c >= width)   return 0;
    return 1;
}


static inline int offset(int row, int col, int image, int height, int width, int images)
{
    int start_of_array = image * height * width;
    int pixel_offset = (row * width) + col;
     
    return start_of_array + pixel_offset;
}


#define OUT_OFFSET(ROW, COL, IMAGE) offset(ROW, COL, IMAGE, out_height, out_width, num_output_images)
#define IN_OFFSET(ROW, COL, IMAGE) offset(ROW, COL, IMAGE, height, width, num_input_images)

static inline int weight_offset(int row, int col, int input_image, int output_image, int height, int width, int num_input_images, int num_output_images)
{
    return output_image * height * width * num_input_images + input_image * height * width + row * width + col;
}


#define WEIGHT_OFFSET(ROW, COL, IN_IMAGE, OUT_IMAGE) weight_offset(ROW, COL, IN_IMAGE, OUT_IMAGE, filter_height, filter_width, num_input_images, num_output_images)

void conv2d_sw(
               float *image,
               float *weights,
               float *biases,
               float *output_image,
               int num_input_images,
               int num_output_images,
               int height,
               int width,
               int filter_height,
               int filter_width,
               int relu,
               int bias)
{
    int  o, i, fr, fc, r, c, rr, cc, r1, c1;
    float sum;
    float max;
    float n;
    float image_value;
    float weight_value;
    float bias_value;
    int image_index;
    int weight_index;
    int output_index;
    int input_index;

    const int stride = 2;
    const int chatty = 0;
    
    int out_height = height - filter_height + 1;
    int out_width = width - filter_width + 1;

    for (o=0; o<num_output_images; o++) {
        for (i=0; i<num_input_images; i++) {
            for (r=0; r<out_height; r++) {
                for (c=0; c<out_width; c++) {
                    sum = 0.0;
                    for (fr=0; fr<filter_height; fr++) {
                        for (fc=0; fc<filter_width; fc++) {

                            image_index = IN_OFFSET(r+fr, c+fc, i);
                            weight_index = WEIGHT_OFFSET(fr, fc, i, o);

                            image_value = image[image_index];
                            weight_value = weights[weight_index];

                            if (chatty) printf("SW image_index: %d weight_index: %d image_value[%d][%d]: %5.3f weight_value: %5.3f = %5.3f \n",
                                                   image_index, weight_index, r+fr, c+fc, image_value, weight_value, image_value * weight_value);
                            sum += image_value * weight_value;
                        }
                    }
                    output_index = OUT_OFFSET(r, c, o);
                    if (i==0) n = sum; else n = sum + output_image[output_index];
                    output_image[output_index] = n;
                    if (chatty) printf("output[%d] = %6.2f \n", output_index, output_image[output_index]);
                }
            }
        }
        if (bias) {  
            for (r=0; r<out_height; r++) {
                for (c=0; c<out_width; c++) {
                    output_index = OUT_OFFSET(r, c, o);
                    image_value = output_image[output_index];
                    bias_value = biases[o];
                    output_image[output_index] = image_value + bias_value;
                }
            }
        }
        if (relu) {
            for (r=0; r<out_height; r++) {
                for (c=0; c<out_width; c++) {
                    output_index = OUT_OFFSET(r, c, o);
                    n = output_image[output_index];
                    if (n<0) output_image[output_index] = 0.0;
                }
            }
        }
    }
}


#ifdef HOST
#ifdef ALGORITHMIC_VERSION

void conv2d_hw(
               cat_memory_type *memory,
               int image,
               int weights,
               int biases,
               int output_image,
               int num_input_images,
               int num_output_images,
               int height,
               int width,
               int filter_height,
               int filter_width,
               int relu,
               int bias)
{
    int  o, i, fr, fc, r, c, rr, cc, r1, c1;
    hw_cat_type sum;
    hw_cat_type max;
    hw_cat_type n;
    hw_cat_type image_value;
    hw_cat_type weight_value;
    hw_cat_type bias_value;
    int image_index;
    int weight_index;
    int output_index;
    int input_index;

    const int chatty = 0;
    
    int out_height = height - filter_height + 1;
    int out_width = width - filter_width + 1;

    for (o=0; o<num_output_images; o++) {
        for (i=0; i<num_input_images; i++) {
            for (r=0; r<out_height; r++) {
                for (c=0; c<out_width; c++) {
                    sum = 0.0;
                    for (fr=0; fr<filter_height; fr++) {
                        for (fc=0; fc<filter_width; fc++) {

                            image_index = IN_OFFSET(r+fr, c+fc, i);
                            weight_index = WEIGHT_OFFSET(fr, fc, i, o);

                            image_value = get_cat_value(memory, image + image_index);
                            weight_value = get_cat_value(memory, weights + weight_index);

                            if (chatty) printf("SW image_index: %d weight_index: %d image_value[%d][%d]: %5.3f weight_value: %5.3f = %5.3f \n",
                                                   image_index, weight_index, r+fr, c+fc, image_value.to_double(), 
                                                   weight_value.to_double(), image_value.to_double() * weight_value.to_double());

                            sum += image_value * weight_value;
                        }
                    }
                    output_index = OUT_OFFSET(r, c, o);
                    if (i==0) n = sum; else n = sum + get_cat_value(memory, output_image + output_index);
                    set_cat_value(memory, output_image + output_index, n);
                    if (chatty) printf("output[%d] = %5.3f \n", output_index, n.to_double());
                }
            }
        }
        if (bias) {  
            for (r=0; r<out_height; r++) {
                for (c=0; c<out_width; c++) {
                    output_index = OUT_OFFSET(r, c, o);
                    image_value = get_cat_value(memory, output_image + output_index);
                    bias_value = get_cat_value(memory, biases + o);
                    set_cat_value(memory, output_image + output_index, image_value + bias_value);
                }
            }
        }
        if (relu) {
            for (r=0; r<out_height; r++) {
                for (c=0; c<out_width; c++) {
                    output_index = OUT_OFFSET(r, c, o);
                    n = get_cat_value(memory, output_image + output_index);
                    if (n<0) set_cat_value(memory, output_image + output_index, 0.0);
                }
            }
        }
    }
}

#endif 

#ifdef ESP_VERSION

#include "catapult_esp_conv2d.h"

void conv2d_hw(
               cat_memory_type *memory,
               int image,
               int weights,
               int biases,
               int output_image,
               int num_input_images,
               int num_output_images,
               int height,
               int width,
               int filter_height,
               int filter_width,
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

   regs.image_address       = image;
   regs.weight_address      = weights;  
   regs.bias_address        = biases;
   regs.output_address      = output_image;
   regs.num_input_images    = num_input_images;
   regs.num_output_images   = num_output_images;
   regs.height              = height;
   regs.width               = width;
   regs.filter_height       = filter_height;
   regs.filter_width        = filter_width;
   regs.relu                = relu;
   regs.bias                = bias;

   conf_info.write(regs);

   // first write biases

   for (i=0; i<num_output_images; i++) {
      dma_read_chnl.write(memory[biases + i]);
   }

   // first write image

   for (int i=0; i<height*width; i++) {
      dma_read_chnl.write(memory[image + i]);
   }

   // then weights

   for (i=0; i<filter_height*filter_width*num_output_images; i++) {
      dma_read_chnl.write(memory[weights + i]);
   }

   // call accelerator

   conv2d_esp_hw(conf_info, dma_read_ctrl, dma_write_ctrl, dma_read_chnl, dma_write_chnl, done);

   // unload results

   for (i=0; i<95*num_output_images; i++) {
       memory[output_image + i] = dma_write_chnl.read();
   }

   done.sync_in();
}

#endif

#ifdef ARCHITECTURE_VERSION

#include "catapult_conv2d.h"

void conv2d_hw(
               cat_memory_type *memory,
               int image,
               int weights,
               int biases,
               int output_image,
               int num_input_images,
               int num_output_images,
               int height,
               int width,
               int filter_height,
               int filter_width,
               int relu,
               int bias)
{
    ac_channel<bool> go;
    ac_channel<bool> done;

    cat_memory_type debug_signal;

    raw_bus_type *memory_base = memory;

    go.write(1);

    catapult_conv2d(
               debug_signal,
               go,
               done,
               bias,
               relu,
               memory_base,
               image,
               weights,
               biases,
               output_image,
               num_input_images,
               num_output_images);

    done.read();
}

#endif // not ALGORITHMIC_VERSION
#else  // not HOST

// embedded version of conv2d_hw goes here

void conv2d_hw(
               cat_memory_type *memory,
               int image,
               int weights,
               int biases,
               int output_image,
               int num_input_images,
               int num_output_images,
               int height,
               int width,
               int filter_height,
               int filter_width,
               int relu,
               int bias)
{
  return;
}
#endif // not HOST
