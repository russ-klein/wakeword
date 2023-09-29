
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
               int bias);

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
               int bias);

