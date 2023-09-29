

#include "catapult_memory_master.h"

typedef ac_int<FILTER_BITS, false>  filter_index_type;

hw_cat_type conv_relu_fn(hw_cat_type n)
{
    if (n<0) return 0;
    return n;
}


void perform_relu(bool relu, raw_memory_line *image_out, raw_memory_line *image_in, index_type image_height, index_type image_width)
{   
    hw_cat_type values[STRIDE];
    index_type count;
    index_type i;
    
    static const index_type stride = STRIDE; 
    const index_type l_area = image_height * image_width;
    
   #pragma hls_pipeline_init_interval 1
    for (count = 0; count < l_area; count += stride) {
        copy_to_regs(values, 0, image_in, count, stride);
       #pragma hls_unroll
        for (i=0; i<STRIDE; i++) {
            values[i] = (relu) ? conv_relu_fn(values[i]) : values[i];
        }
        copy_from_regs(image_out, count, values, 0, stride);
    }
}


void perform_convolution(
                         raw_memory_line   *input_image,
                         raw_memory_line   *filter,
                         raw_memory_line   *output_image,
                         index_type         input_image_number,
                         hw_cat_type        bias)
                         // index_type         image_height,
                         // index_type         image_width)
{
    hw_cat_type partial_sum_buffer[STRIDE];
    hw_cat_type products[STRIDE][FILTER_HEIGHT * FILTER_WIDTH];
    hw_cat_type sums;
    hw_cat_type feature_load[STRIDE];
    static hw_cat_type shift_register[FILTER_SIZE];

    // registers for computations
    hw_cat_type filter_regs[FILTER_SIZE];
    hw_cat_type image_regs[FILTER_SIZE];
    hw_cat_type product_array[FILTER_SIZE];
    hw_cat_type output_array[186];

    hw_cat_type input_regs[STRIDE];
    hw_cat_type output_regs[STRIDE];

    // filter_index_type fr;
    // filter_index_type fc;
    // index_type output_index;
    // index_type loop_entry;
    // index_type image_index;
    // index_type target_pixel;
    // index_type tail_pixel;
    // index_type lead_pixel;
    // index_type shift_offset;
    // index_type p_lead_pixel;
    // index_type p_image_index;
    // index_type p_target_pixel;
    // index_type f_index;
    // index_type p_index;
    // index_type num;
    // index_type row;
    // index_type col;
    // index_type pr;
    // index_type pc;
    // index_type rr;
    // index_type cc;
    // p_type p;

    index_type image_offset;
    index_type out_index;
    index_type i;
    index_type c;
    index_type r;

    // static const index_type tail_round_up = TAIL_ROUND_UP - STRIDE;
    // static const index_type margin_round_up = MARGIN_ROUND_UP;
    // static const index_type area = AREA;
    // static const index_type mid_point_height = (FILTER_HEIGHT - 1) / 2;
    // static const index_type mid_point_width  = (FILTER_WIDTH - 1) / 2;
    // static const index_type stride = STRIDE;
    // static const index_type pixels_to_shift = AREA + SHIFT_REGISTER_SIZE;
    // static const bool chatty = false;
    static const index_type out_values = (IMAGE_HEIGHT - FILTER_HEIGHT + 1);

    // lead_pixel = the number of the pixel at the start of the shift_register
    // target_pixel = the number of the pixel at the center of the convolution kernel (lead_pixel + margin)
    // tail_pixel = the last pixel in the shift register (lead_pixel + shift_register_size)
    // total pixels needed to be shifted through is AREA + SHIFT_REGISTER_SIZE - (STRIDE -1)

    static const index_type filter_width      = (FILTER_WIDTH);
    static const index_type filter_height     = (FILTER_HEIGHT);
    static const index_type filter_size       = (FILTER_SIZE);
    static const index_type image_width       = (IMAGE_WIDTH);

    copy_to_regs(filter_regs, 0, filter, 0, filter_size);
    copy_to_regs(image_regs, 0, input_image, 0, filter_size);

    image_offset = filter_size;

static bool first = false;
static bool second = false;

   #pragma hls_pipeline_init_interval 1
    for (out_index = 0; out_index < out_values; out_index++) {
       hw_cat_type sum = 0;
       for (i = 0; i < filter_size; i++) {
          product_array[i] = filter_regs[i] * image_regs[i];
if (first) printf("HW: %3d product[%3d]: %8.4f = weight: %8.4f * image: %8.4f \n", out_index.to_int(), i.to_int(), product_array[i].to_double(), filter_regs[i].to_double(), image_regs[i].to_double());
       }
       for (i = 0; i< filter_size; i++) {
          sum += product_array[i];
       }
if (first) printf("sum = %8.4f bias = %8.4f sum+bias: %8.4f \n", sum.to_double(), bias.to_double(), (sum+bias).to_double());
       output_array[out_index] = sum + bias;
//if (first || second) if ((out_index.to_int() % 10) == 0) printf("\n");
//if (first || second) printf("hw: %2d %8.4f ", second?out_index.to_int()+95:out_index.to_int(), output_array[out_index].to_double()); 
       for (r=0; r<filter_height-1; r++) {
          for (c=0; c<filter_width; c++) {
             image_regs[r * filter_width + c] = image_regs[(r+1) * filter_width + c];
          }
       }


       copy_to_regs(image_regs, (filter_height-1) * filter_width, input_image, image_offset, image_width);

       image_offset += filter_width;

    } 
    copy_from_regs(output_image, 0, output_array, 0, 186);
if (second) second = false;
if (first)  {first=false; second=true;}
}
/*
main_convolve_loop:
    for (tail_pixel = 0; tail_pixel < pixels_to_shift; tail_pixel += stride) {

        target_pixel = tail_pixel - margin_round_up;
        lead_pixel = target_pixel - tail_round_up;

        get_shift_in_values(feature_load, input_image, target_pixel, stride, area);

        shift_by_stride(shift_register, feature_load, SHIFT_REGISTER_SIZE);

        if ((lead_pixel  < 0) || (lead_pixel > area) || (input_image_number == 0)) {
           #pragma hls_unroll
            for (p=0; p<STRIDE; p++) {
                partial_sum_buffer[p] = bias; // 0.0;
            }
        } else {
            copy_to_regs(partial_sum_buffer, 0, output_image, lead_pixel, stride);
        }

       #pragma hls_unroll
        for (p=0; p<STRIDE; p++) {
            p_target_pixel = target_pixel + p;
            p_lead_pixel = lead_pixel + p;
            compute_row_col(p_lead_pixel, pr, pc, WIDTH);

            sums = 0;

            if ((0 <= p_lead_pixel) && (p_lead_pixel < area)) {

               #pragma hls_unroll
            conv_outer_loop:
                for (fr=0; fr<FILTER_HEIGHT; fr++) {

                   #pragma hls_unroll
                conv_inner_loop:
                    for (fc=0; fc<FILTER_WIDTH; fc++) {

                        rr = pr + fr - mid_point_height;
                        cc = pc + fc - mid_point_width;
                        shift_offset = fr * WIDTH + fc + p;
                        f_index = fr * FILTER_WIDTH + fc;

                        products[p][f_index] = (hw_in_bounds(rr, cc, HEIGHT, WIDTH)) ? filter_regs[f_index] * shift_register[shift_offset] : 0.0;

                        if (chatty) {
                            if (hw_in_bounds(rr, cc, HEIGHT, WIDTH)) {
                                printf("image_value[%d][%d]: %5.3f weight_value: %5.3f \n", rr.to_int(), cc.to_int(), shift_register[shift_offset].to_double(), filter_regs[f_index].to_double());
                            }
                        }

                        sums += products[p][f_index];
                    }
                }

                if (chatty) printf("sum[%d][%d] = %5.3f prior sum: %5.3f \n", pr.to_int(), pc.to_int(), sums.to_double(), partial_sum_buffer[p].to_double());

                partial_sum_buffer[p] += sums;
                if (chatty) {
                    if ((output_index % WIDTH)==0) printf("\n");
                    if (sums <0.001) printf("  -   ");
                    else printf("%5.2f ", sums.to_double());
                }
            }
        }
        if ((0 <= lead_pixel) && (lead_pixel < area)) {
            num = stride;
            if ((area - lead_pixel) < stride) {
               num = area - lead_pixel;
            }
            copy_from_regs(output_image, lead_pixel, partial_sum_buffer, 0, num);
        }
    }
}
*/

#pragma hls_design top
void catapult_conv2d(
                 cat_memory_type &debug_signal,
                 ac_channel<bool> &go,
                 ac_channel<bool> &done,
                 bool use_bias,
                 bool relu,
                 raw_bus_type memory    [0x100000],
                 index_type image_offset,
                 index_type weight_offset,
                 index_type bias_offset,
                 index_type output_offset,
                 index_type num_input_images,
                 index_type num_output_images)
{
    index_type   i;
    index_type   o;
    index_type   image_pointer;
    index_type   weight_pointer;
    index_type   output_pointer;

    raw_memory_line output_image_pr_mem[((IMAGE_SIZE) + (STRIDE - 1))/STRIDE];
    raw_memory_line output_image_mem[((IMAGE_SIZE) + (STRIDE - 1))/STRIDE];
    raw_memory_line input_image_mem[1][((IMAGE_SIZE) + (STRIDE - 1))/STRIDE];
    raw_memory_line filter_mem[((FILTER_SIZE) + (STRIDE - 1))/STRIDE];

    hw_cat_type  bias_values[186];
    hw_cat_type  bias_value;

    static const index_type image_height = IMAGE_HEIGHT;
    static const index_type image_width  = IMAGE_WIDTH;
    static const index_type filter_size = FILTER_SIZE;
    static const index_type image_size  = IMAGE_SIZE;
    static const index_type stride = STRIDE;

    go.read();

    // read in all images for the layer
    image_pointer = image_offset;
    for (i=0; i<num_input_images; i++) {
        // load feature map from external memory into internal memory
        load_from_system_memory(memory, image_pointer, image_size, input_image_mem[i], 0);
        image_pointer += (image_size);
        //printf("convolution input image: %d \n", i);
        //print_image(input_image_mem[i], image_height, image_width);
    }

    // if (use_bias) load_from_system_memory(memory, bias_offset, num_output_images, bias_values, 0);
    // todo: write efficient load from bus to hw_cat_type array

    // read in all the bias values (usually small)
    if (use_bias) for (i=0; i<num_output_images; i++) bias_values[i] = read_from_system_memory(memory, bias_offset + i);

    output_pointer = output_offset;
    weight_pointer = weight_offset;

    for (o=0; o<num_output_images; o++) {
        for (i=0; i<num_input_images; i++) {
            // load filter from external memory into internal memory
            load_from_system_memory(memory, weight_pointer, filter_size, filter_mem, 0);
            if (use_bias) bias_value = bias_values[o]; else bias_value = 0.0;
            perform_convolution(input_image_mem[i], filter_mem, output_image_pr_mem, i, bias_value); // , image_height, image_width);
            weight_pointer += filter_size;
        }
        perform_relu(relu, output_image_mem, output_image_pr_mem, image_height, image_width);
        store_into_system_memory(output_image_mem, 0, 95, memory, output_pointer);
        output_pointer += 95; // (image_height * image_width); // output_pointer++;
    }

    done.write(1);
}
