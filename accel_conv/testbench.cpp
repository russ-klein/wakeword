
#include <stdlib.h>
#include <stdio.h>

#include "defines.h"
#include "conv_2d.h"
#include "cat_access.h"

void initialize_test_data(
    float *a,
    int    size)
{
    // initialize array to values between -10.0 to 10.0
    static bool first = true;
    int i;

    if (first) {
      first = false;
      srand(0x2020);
    }

    for (i=0; i<size; i++) {
       a[i] = (10.0 * (float) rand()/RAND_MAX) - 5.0;
    }
}

float fabs(float f)
{
    if (f<0) return -f;
    else return f;
}

int close(float f1, float f2)
{
    if (fabs(f1-f2) < 0.01) return 1;
    else return 0;
} 

int compare_results(float *f1, float *f2, int size)
{
    int i;
    int errors = 0;

    for (i=0; i<size; i++) {
       if (!close(f1[i], f2[i])) {
          errors++;
          printf("Mismatch: location: %d %8.6f %8.6f \n", i, f1[i], f2[i]);
       }
    }
    return errors;
}

#define NUM_INPUT_IMAGES 1
#define NUM_OUTPUT_IMAGES 2

#define IMAGE_HEIGHT 101
#define IMAGE_WIDTH   20

#define WEIGHT_HEIGHT  7
#define WEIGHT_WIDTH  20

#define IMAGE_SIZE   (IMAGE_HEIGHT * IMAGE_WIDTH)
#define WEIGHT_SIZE  (WEIGHT_HEIGHT * WEIGHT_WIDTH * NUM_INPUT_IMAGES * NUM_OUTPUT_IMAGES)

#define OUT_LEN ((IMAGE_HEIGHT - FILTER_HEIGHT + 1) * NUM_OUTPUT_IMAGES)

int main()
{
    float image[IMAGE_SIZE];
    float weights[WEIGHT_SIZE];
    float biases[NUM_OUTPUT_IMAGES];
    float sw_output[OUT_LEN];
    float hw_output[OUT_LEN];

    cat_memory_type hw_memory[0x10000];

    int relu = 1;
    int bias = 1;
    int r;

    int image_offset   = 0;
    int weight_offset  = image_offset   + IMAGE_SIZE;
    int bias_offset    = weight_offset  + WEIGHT_SIZE;
    int output_offset  = bias_offset    + OUT_LEN;

    initialize_test_data(image, sizeof(image)/sizeof(image[0]));
    initialize_test_data(weights, sizeof(weights)/sizeof(weights[0]));
    initialize_test_data(biases, sizeof(biases)/sizeof(biases[0]));

    conv2d_sw(image, weights, biases, sw_output, NUM_INPUT_IMAGES, NUM_OUTPUT_IMAGES,
              IMAGE_HEIGHT, IMAGE_WIDTH, FILTER_HEIGHT, FILTER_WIDTH, relu, bias);
    
    copy_to_cat(hw_memory, image_offset,  image,   IMAGE_SIZE);
    copy_to_cat(hw_memory, weight_offset, weights, WEIGHT_SIZE);
    copy_to_cat(hw_memory, bias_offset,   biases,  NUM_OUTPUT_IMAGES);

    conv2d_hw(hw_memory, image_offset, weight_offset, bias_offset, output_offset, NUM_INPUT_IMAGES, NUM_OUTPUT_IMAGES,
              IMAGE_HEIGHT, IMAGE_WIDTH, FILTER_HEIGHT, FILTER_WIDTH, relu, bias);

    copy_from_cat(hw_memory, hw_output, output_offset, OUT_LEN);

    r = compare_results(sw_output, hw_output, OUT_LEN);

    if (r) {
       printf("Failure!\n");
    } else {
       printf("Success!\n");
    }    
    return r;
}
