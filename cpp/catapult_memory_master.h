
#define INDEX_BITS          25
#define PAR_BITS            (PAR_IN + BUS_WIDTH)

typedef ac_int<PAR_BITS, false>     p_type;
typedef ac_int<INDEX_BITS, true>    index_type;

typedef ac_int<BUS_WIDTH * PAR_IN * WORD_SIZE, false> raw_memory_line;

static void set_bus_word(raw_bus_type &line, index_type index, hw_cat_type word)
{
    // isolate all the slicing crap
    line.set_slc(index * WORD_SIZE, word.slc<WORD_SIZE>(0));
}


static hw_cat_type get_bus_word(raw_bus_type &line, index_type index)
{
    hw_cat_type t;

    t.set_slc(0, line.slc<WORD_SIZE>(index * WORD_SIZE));

    return t;
}


static void set_memory_word(raw_memory_line &line, index_type index, hw_cat_type word)
{
    line.set_slc(index * WORD_SIZE, word.slc<WORD_SIZE>(0));
}


static hw_cat_type get_memory_word(raw_memory_line &line, index_type index)
{
    hw_cat_type t;

    t.set_slc(0, line.slc<WORD_SIZE>(index * WORD_SIZE));

    return t;
}


static hw_cat_type read_from_system_memory(raw_bus_type *memory, index_type offset)
{
    index_type   bus_read_address;
    index_type   bus_line_offset;
    index_type   value_offset;

    raw_bus_type input_buffer;

    static const index_type bus_width_bits = BUS_WIDTH_BITS;
    static const p_type     par_in         = PAR_IN;

    bus_read_address = (offset / par_in) >> bus_width_bits;
    bus_line_offset = (bus_read_address * par_in) << bus_width_bits;
    value_offset = offset - bus_line_offset;

    input_buffer = memory[bus_read_address];

    return get_bus_word(input_buffer, value_offset);
}




static void read_line(hw_cat_type *data, index_type data_addr, raw_memory_line *array_memory, index_type array_addr, index_type size)
{   
    raw_memory_line t;
    index_type d_offset;
    index_type mem_addr;
    index_type count;
    index_type diff;
    index_type col;
    index_type row;
    index_type min;
    index_type max;
    p_type p;
    
    static const index_type stride = STRIDE;
    
    row = array_addr / stride;
    col = array_addr % stride;
    
    min = array_addr;
    max = array_addr + size;
    
    d_offset = data_addr;
    mem_addr = row * stride;
         
    diff = min - mem_addr;
    
    count = 0;
   #pragma hls_pipeline_init_interval 1
    while (count < size) {
        t = array_memory[row];
printf("read: %016lx from row address: %d \n", t, row);
        
       #pragma hls_unroll
        for (p=0; p<stride; p++) {
            if ((min <= (mem_addr + p)) && ((mem_addr + p) < max)) {
                data[d_offset + p - diff] = get_memory_word(t, p); // test me
                //data[d_offset] = get_line_word(t, p);
                //data[d_offset].set_slc(0, t.slc<WORD_SIZE>(p*WORD_SIZE));
                //data[d_offset] = t.word[p];
                
                // d_offset++;
                //count++;
            }
        }
        d_offset += (stride - diff); // test me
        count += (stride - diff);    // test me
        diff = 0;
        mem_addr += stride;
        row++;
    }
}



static void write_line(raw_memory_line *array_memory, index_type array_addr, hw_cat_type *data, index_type data_addr, index_type size)
{
    raw_memory_line t;
    hw_cat_type buffer[STRIDE];
    index_type d_offset;
    index_type mem_addr;
    index_type count;
    index_type diff;
    index_type col;
    index_type row;
    index_type min;
    index_type max;
    p_type p;
   
    static const index_type stride = STRIDE;
   
    row = array_addr / stride;
    col = array_addr % stride;
   
    min = array_addr;
    max = array_addr + size;
   
    d_offset = data_addr;
    mem_addr = row * stride;
   
    diff = min - mem_addr;
   
    count = 0;

    while (count < size) {
        if ((((col != 0) && (count == 0))) || ((size - count) < stride)) {
             t = array_memory[row];
printf("misaligned write, read %016lx from row address: %d \n", t, row);
}

       #pragma hls_unroll
        for (p=0; p<stride; p++) {
            buffer[p] = ((min <= (mem_addr + p)) && ((mem_addr + p) < max)) ? data[d_offset - diff + p] : get_memory_word(t, p);
            set_memory_word(t, p, buffer[p]);
                //-set_line_word(t, p, data[d_offset]);
                //t.set_slc(p * WORD_SIZE, data[d_offset].slc<WORD_SIZE>(d_offset * WORD_SIZE));
                //t.word[p] = data[d_offset];
                //-d_offset++;
                //-count++;
            //}
        }

        d_offset += stride - diff;
        count += stride - diff;
        diff = 0;

printf("writing %016lx to row address %d \n", t, row);
        array_memory[row] = t;

        mem_addr += stride;
        row++;
    }
}

static void copy_from_regs(raw_memory_line *dst, index_type dst_offset, hw_cat_type *src, index_type src_offset, index_type size)
{
    // write into internal memories from an array of registers
    // *should* make it easy for catapult to pipeline access to internal memories

    index_type count;
    index_type n;

    static const index_type stride = STRIDE;

    count = 0;
    while (count < size) {
        n = stride;
        if ((size - count) < stride) n = size - count;
        write_line(dst, dst_offset, src, src_offset, n);
        count += n;
        src_offset += n;
        dst_offset += n;
    }
}


static void copy_to_regs(hw_cat_type *dst, index_type dst_offset, raw_memory_line *src, index_type src_offset, index_type size)
{
    // read out of internal memories to an array of registers
    // *should* make it easy for catapult to pipeline access to internal memories

    index_type count;
    index_type n;

    static const index_type stride = STRIDE;

    count = 0;
    while (count < size) {
        n = stride;
        if ((size - count) < stride) n = size - count; // mis-aligned at the end of transfer
        read_line(dst, dst_offset, src, src_offset, n);
        count += n;
        src_offset += n;
        dst_offset += n;
    }
}


static void load_input_array(
    raw_bus_type &output_buffer,
    hw_cat_type  *array_buffer,
    index_type    bus_min,
    index_type    bus_max,
    index_type    bus_line_start,
    index_type    bus_line_end,
    index_type    bus_alignment_offset)
{
    hw_cat_type   t;
    index_type    bus_address;
    uint1         bus_word_valid;
    index_type    word_offset;
    p_type        p;

    static const bool chatty = false;

   #pragma hls_unroll
    for (p=0; p<STRIDE; p++) {
        bus_address = bus_line_start + p;
        bus_word_valid = (((bus_min<=bus_address) && (bus_address<bus_max)) &&
                            ((bus_line_start<=bus_address) && (bus_address<bus_line_end)) &&
                            (bus_alignment_offset <= p))  ? 1 : 0;
        t = (bus_word_valid) ? array_buffer[p - bus_alignment_offset] : get_bus_word(output_buffer, p);
        set_bus_word(output_buffer, p, t);
        if (bus_word_valid) {
            if (chatty) printf("wrote memory[%d] = %5.3f \n", bus_address.to_int(), t.to_double());
        }
    }
}


static void store_into_system_memory(raw_memory_line *output_array, index_type array_offset, index_type size,  raw_bus_type *memory, index_type offset)
{
    // copy size words from the output array at array_offset into system memory at offset
    // offsets and size parameters are in units of words

    raw_bus_type output_buffer;
    index_type   line_count;
    hw_cat_type  array_buffer[STRIDE];

    index_type   bus_min;
    index_type   bus_max;
    index_type   bus_address;
    index_type   bus_write_address;
    index_type   bus_line_start;
    index_type   bus_line_end;
    index_type   bus_alignment_offset;
    index_type   bus_word_offset;
    uint1        bus_word_valid;

    index_type   array_min;
    index_type   array_max;
    index_type   array_size;
    index_type   array_address;
    index_type   array_line_start;
    index_type   array_line_end;
    index_type   array_alignment_offset;
   
    index_type   remaining_words;
    uint1        array_unaligned;
    uint1        bus_unaligned;
    uint1        first;
    uint1        last;
    index_type   start_address;
    index_type   end_address;
    index_type   count;
    index_type   w;
    index_type   b;
    p_type       p;

    static const bool       chatty         = false;
    static const index_type bus_width_bits = BUS_WIDTH_BITS;
    static const p_type     par_in         = PAR_IN;
    static const index_type bus_line_width = STRIDE;
    static const index_type stride         = STRIDE;

    first = 1;
   
    bus_min = offset;
    bus_max = offset + size;
   
    bus_write_address = (offset / par_in) >> bus_width_bits;
    bus_line_start = bus_write_address * bus_line_width;
    bus_line_end = bus_line_start + bus_line_width;
   
    array_min = array_offset;
    array_max = array_offset + size;
   
    array_line_start = (array_min / par_in) * par_in;
    array_line_end = array_line_start + stride;
   
    bus_alignment_offset = bus_min - bus_line_start;
    array_alignment_offset = array_offset % par_in;

    array_unaligned = (array_alignment_offset != 0) ? 1 : 0;
    bus_unaligned   = (bus_alignment_offset != 0)   ? 1 : 0;

    start_address = array_offset;
    end_address = array_offset + bus_line_width;

    line_count = 0;

    count = 0;
    while (count<size) {

        array_size = stride;
        if ((start_address + stride) > array_max) array_size = array_max - start_address;
        copy_to_regs(array_buffer, 0, output_array, start_address, array_size);
        // copy_to_regs(array_buffer, 0, output_array, 0, start_address + (array_size /stride));
        printf("address: %08x , array_size: %d \n", start_address, array_size);

        if (first) start_address += stride - bus_alignment_offset;
        else start_address += stride;

        if (first && ((size < bus_line_width) || (bus_min != bus_line_start))) {
            output_buffer= memory[bus_write_address];
            if (chatty) printf("read/modify/write at start \n");
        }

        else if ((size > bus_line_width) && ((bus_line_start + bus_line_width) > bus_max)) { // last bus line processed
            output_buffer = memory[bus_write_address+1];
            if (chatty) printf("read/modify/write at end \n");
        }

        load_input_array(output_buffer, array_buffer, bus_min, bus_max, bus_line_start, bus_line_end, bus_alignment_offset);

        if (first) count += bus_line_width - bus_alignment_offset;
        else count += bus_line_width;

        memory[bus_write_address] = output_buffer;

        bus_line_start += bus_line_width;
        bus_line_end += bus_line_width;

        remaining_words = size - count;

        bus_write_address++;
        bus_alignment_offset = 0;
        first = 0;
    }
}



static index_type load_input_buffer(
    raw_bus_type input_buffer,
    hw_cat_type *array_buffer,

    index_type   bus_min,
    index_type   bus_max,
    index_type  &bus_line_start,
    index_type  &bus_line_end,
    index_type   start_address,
    index_type   end_address,
    index_type   bus_alignment_offset)
{
    // loads STRIDE words into the the array buffer
    hw_cat_type  t;
    index_type   bus_word_offset;
    index_type   current_address;
    index_type   bus_address;
    index_type   count;
    uint1        bus_word_valid;
    uint1        in_range;
   
    index_type   w;
    p_type       p;
   
    static const bool chatty = false;
    static const index_type stride = STRIDE;
   
    count = 0;
    current_address = 0;
   
    bus_address = bus_line_start;
   
   #pragma hls_unroll
    for (p=0; p<STRIDE; p++) {
        bus_word_valid = ((bus_min <= bus_address) && (bus_address < bus_max)) ? 1 : 0;
        t = (bus_word_valid) ? get_bus_word(input_buffer, p) : 0.0;
        array_buffer[current_address] = t;

        current_address += (bus_word_valid) ? 1 : 0;
        count += (bus_word_valid) ? 1 : 0;

        if (bus_word_valid) {
            if (chatty) printf("loaded array_buffer[%d] = %5.3f \n", current_address.to_int(), t.to_double());
        }

        bus_address++;
    }

    bus_line_start += stride;
    bus_line_end += stride;

    return count;
}


static void load_from_system_memory(raw_bus_type *memory, index_type offset, index_type size, raw_memory_line *input_array, index_type array_offset)
{
    raw_bus_type input_buffer;
    hw_cat_type  transfer_buffer[STRIDE];

    index_type   bus_min;
    index_type   bus_max;
    index_type   bus_read_address;
    index_type   bus_line_start;
    index_type   bus_line_end;
    index_type   bus_alignment_offset;

    index_type   array_min;
    index_type   array_max;
    index_type   array_address;
    index_type   array_line_start;
    index_type   array_line_end;
    index_type   array_alignment_offset;

    index_type   alignment_offset;
    uint1        array_unaligned;
    uint1        bus_unaligned;
    index_type   start_address;
    index_type   end_address;
    index_type   num;
    index_type   count;
    index_type   b;

    static const bool          chatty          = false;
    static const index_type    bus_width_bits  = BUS_WIDTH_BITS;
    static const p_type        par_in          = PAR_IN;
    static const index_type    stride          = STRIDE;

    if (chatty) printf("reading: %d values from memory at address: %d \n", size.to_int(), offset.to_int());
    bus_min = offset;
    bus_max = offset + size;

    bus_read_address = (offset / par_in) >> bus_width_bits;
    bus_line_start = bus_read_address * stride;
    bus_line_end = bus_line_start + stride;

    bus_alignment_offset = bus_min - bus_line_start;

    bus_unaligned   = (bus_alignment_offset != 0)   ? 1 : 0;

    start_address = array_offset;
    end_address = stride;
    array_max = array_offset + size;

    count = 0;

   #pragma hls_pipeline_init_interval 1
    while (count<size) {

        input_buffer = memory[bus_read_address];
        if (chatty) {
            printf("read line: %d at address: %d \n", bus_read_address.to_int(), bus_read_address.to_int() * STRIDE);
            for (int i=0; i<STRIDE; i++) {
                hw_cat_type t;
                t.set_slc(0, input_buffer.slc<WORD_SIZE>(i*WORD_SIZE));
                printf("%5.3f ", t.to_double());
            }
            printf("\n");
        }

        num = load_input_buffer(input_buffer, transfer_buffer, bus_min, bus_max, bus_line_start, bus_line_end,
                              start_address, end_address, bus_alignment_offset);
        count += num;
        bus_read_address++;

        copy_from_regs(input_array, start_address, transfer_buffer, 0, num);

        start_address += num;
        end_address += num;
    }
}


