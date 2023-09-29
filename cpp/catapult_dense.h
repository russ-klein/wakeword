
#include "catapult_memory_master.h"

hw_cat_type relu_fn(hw_cat_type n)
{
    if (n<0) return 0;
    return n;
}

 
hw_cat_type load_weights_and_multiply(raw_bus_type *memory, index_type offset, index_type num_input_elements, raw_memory_line *input_image)
{
    raw_memory_line input_buffer;
    hw_cat_type     values[STRIDE];
    hw_cat_type     weights[STRIDE];
    hw_cat_type     sum;
    index_type      count;
    index_type      i;

    static const bool chatty = false;
    static const index_type stride = STRIDE;

    count = 0;
    sum = 0.0;

    while (count < num_input_elements) {
        load_from_system_memory(memory, offset + count, stride, &input_buffer, 0);
        copy_to_regs(values, 0, input_image, count, stride);
        copy_to_regs(weights, 0, &input_buffer, 0, stride);
       #pragma hls_unroll
        for (i=0; i<STRIDE; i++) {
            sum += (count < num_input_elements) ? values[i] * weights[i] : 0.0;
            count += (count < num_input_elements) ? 1 : 0;
            if (count < num_input_elements) {
                if (chatty) printf("sum: %5.3f = image value: %5.3f * weight: %5.3f \n",
                                   sum.to_double(), values[i].to_double(), weights[i].to_double());
            }
        }
    }
    return sum;
}


#pragma hls_design top

void catapult_dense(
                 cat_memory_type   &debug_signal,
                 ac_channel<bool>  &go,
                 ac_channel<bool>  &done,
                 bool               use_bias,
                 bool               relu,
                 raw_bus_type       memory[0x100000],
                 index_type         image_offset,
                 index_type         weight_offset,
                 index_type         bias_offset,
                 index_type         output_offset,
                 index_type         num_units,
                 index_type         num_input_values,
                 index_type         num_output_values)
{
    index_type   i;
    index_type   o;
    index_type   image_pointer;
    index_type   weight_pointer;
    index_type   output_pointer;
    index_type   out_pointer;
    index_type   count;

    p_type       p;
    hw_cat_type  value;
    raw_memory_line line;

    hw_cat_type  dense_out_buffer[STRIDE];

    raw_memory_line dense_in_mem[(20000 + (STRIDE -1))/STRIDE];
    raw_memory_line dense_out_mem[(1000 + (STRIDE -1))/STRIDE];

    hw_cat_type  bias_values[500];

    static const index_type stride = STRIDE;

    go.read();

    load_from_system_memory(memory, image_offset, num_input_values, dense_in_mem, 0);

    // if (use_bias) load_from_system_memory(memory, bias_offset, num_output_values, bias_values, 0);
    // todo: write efficient load from bus to hw_cat_type array

    if (use_bias) for (i=0; i<num_output_values; i++) bias_values[i] = read_from_system_memory(memory, bias_offset + i);

    count = 0;
    out_pointer = 0;
    while (count<num_output_values) {
        for (p=0; p<stride; p++) {
            if ((out_pointer + p) < num_output_values) {
                hw_cat_type t;
                t = load_weights_and_multiply(memory, weight_offset + num_input_values * count, num_input_values, dense_in_mem);
                if (use_bias) t += bias_values[count];
                if (relu) t = relu_fn(t);
                dense_out_buffer[p] = t; // load_weights_and_multiply(memory, weight_offset + num_input_values * count, num_input_values, dense_in_mem);
                count++;
            } else {
                dense_out_buffer[p] = 0.0;
            }
        }
        copy_from_regs(dense_out_mem, out_pointer, dense_out_buffer, 0, stride);
        out_pointer += stride;
    }

    store_into_system_memory(dense_out_mem, 0, num_output_values, memory, output_offset);

    done.write(1);
}
