#include "defines.h"

#define DMA_SIZE             4 
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

typedef ac_int<64, false> dma_data_t;


template<typename ARRAY_TYPE>
void dma_load(
   ARRAY_TYPE             *memory,  // byte address
   address_type            base,
   index_type              offset,  // in words, not lines
   index_type              size,    // in words, not lines
   ac_channel<dma_info_t> &dma_ctrl,
   ac_channel<dma_data_t> &dma_data)
{
   const int   bits                 = memory[0].width;
   const int   words_per_bus_cycle  = (BUS_WIDTH / bits);
   const int   bytes_per_bus_cycle  = (BUS_WIDTH / 8);
   bool        dma_done             = false;

   index_type start_address = (base / bytes_per_bus_cycle) + (offset / words_per_bus_cycle);
   index_type end_address   = (base / bytes_per_bus_cycle) + ((offset + (size - 1)) / words_per_bus_cycle);
   index_type address;
   index_type d = (offset / words_per_bus_cycle) * words_per_bus_cycle - offset;

   dma_info_t  dma_read_info;
   dma_data_t  memory_line;

   dma_read_info.index     = start_address;
   dma_read_info.length    = end_address - start_address + 1;
   dma_read_info.size      = DMA_SIZE; // 64 bits

   do {
      dma_done = dma_ctrl.nb_write(dma_read_info);  
   } while (!dma_done);

   if (dma_done) {  // force serialization between config and data transfer
      address = start_address;
      for (index_type i=0; ; i+=words_per_bus_cycle) {
         memory_line = dma_data.read();
        #pragma hls_unroll words_per_bus_cycle
         for (int j=0; j<words_per_bus_cycle; j++) {
            if (((d+j) >= 0) && ((d+j) < size)) memory[d+j].set_slc(0, memory_line.slc<bits>(j*bits));
         }
         address++;
         d += words_per_bus_cycle;
         if (d >= size) break;
      }
   }
}


template<typename ARRAY_TYPE>
void dma_load_float(
   ARRAY_TYPE             *memory,
   address_type            base,
   index_type              offset,  // in words, not lines
   index_type              size,    // in words, not lines
   ac_channel<dma_info_t> &dma_ctrl,
   ac_channel<dma_data_t> &dma_data)
{
   const int   bits                 = 32;
   const int   words_per_bus_cycle  = (BUS_WIDTH / bits);
   const int   bytes_per_bus_cycle  = (BUS_WIDTH / 8);
   bool        dma_done             = false;

   index_type start_address = (base / bytes_per_bus_cycle) + (offset / words_per_bus_cycle);
   index_type end_address   = (base / bytes_per_bus_cycle) + ((offset + (size-1)) / words_per_bus_cycle);
   index_type address;
   index_type d = (offset / words_per_bus_cycle) * words_per_bus_cycle - offset;

   dma_info_t  dma_read_info;
   dma_data_t  memory_line;

   dma_read_info.index              = start_address;
   dma_read_info.length             = end_address - start_address + 1;
   dma_read_info.size               = DMA_SIZE; // 64 bits

   ac_ieee_float32 f[BUS_WIDTH/bits];

   do {
      dma_done = dma_ctrl.nb_write(dma_read_info);  
   } while (!dma_done);

   if (dma_done) {  // force serialization between config and data transfer
      address = start_address;
      for (index_type i=0; ; i+=words_per_bus_cycle) {
         memory_line = dma_data.read();
        #pragma hls_unroll words_per_bus_cycle
         for (int j=0; j<words_per_bus_cycle; j++) {
            if (((d+j) >= 0) && ((d+j) < size)) {
               f[j].d = memory_line.slc<bits>(j*bits);
               memory[d+j] = f[j];
            }
         }
         address++;
         d += words_per_bus_cycle;
         if (d >= size) break;
      }
   }
}

template<typename ARRAY_TYPE>
void dma_store(
   ARRAY_TYPE             *memory,
   address_type            base,
   index_type              offset,  // in words, not lines
   index_type              size,    // in words, not lines
   ac_channel<dma_info_t> &dma_write_ctrl,
   ac_channel<dma_data_t> &dma_write_data,
   ac_channel<dma_info_t> &dma_read_ctrl,
   ac_channel<dma_data_t> &dma_read_data)
{
   const int   bits                 = memory[0].width;
   const int   words_per_bus_cycle  = (BUS_WIDTH / bits);
   const int   bytes_per_bus_cycle  = (BUS_WIDTH / 8);
   bool        dma_done             = false;

   index_type start_address = (base / bytes_per_bus_cycle) + (offset / words_per_bus_cycle);
   index_type end_address   = (base / bytes_per_bus_cycle) + ((offset + (size-1)) / words_per_bus_cycle);
   index_type address;
   index_type d = (offset / words_per_bus_cycle) * words_per_bus_cycle - offset;
   index_type e = ((offset + size) / words_per_bus_cycle) * words_per_bus_cycle - (offset + size);

   dma_info_t  dma_read_info;
   dma_info_t  dma_write_info;

   dma_write_info.index             = start_address;
   dma_write_info.length            = end_address - start_address + 1;
   dma_write_info.size              = DMA_SIZE; // 64 bits

   dma_data_t  memory_line;
   dma_data_t  last_line;

   bool aligned_start = (d == 0);
   bool aligned_end   = (e == 0);

   if (!aligned_start || ((aligned_start & !aligned_end) && (start_address==end_address))) {
      dma_read_info.index = start_address;
      dma_read_info.length = 1;
      dma_read_info.size = DMA_SIZE;
     
      dma_done = false;

      do {
         dma_done = dma_read_ctrl.nb_write(dma_read_info);
      } while (!dma_done);

      if (dma_done) memory_line = dma_read_data.read();
   }

   if (!aligned_end && (end_address != start_address)) {
      dma_read_info.index = end_address;
      dma_read_info.length = 1;
      dma_read_info.size = DMA_SIZE;

      dma_done = false;

      do {
         dma_done = dma_read_ctrl.nb_write(dma_read_info);
      } while (!dma_done);
      
      if (dma_done) last_line = dma_read_data.read();
   }
      
   dma_done = false;

   do {
      dma_done = dma_write_ctrl.nb_write(dma_write_info);  
   } while (!dma_done);

   if (dma_done) {  // force serialization between config and data transfer
      address = start_address;
      for (index_type i=0; ; i+=words_per_bus_cycle) {
         if ((address == end_address) && (start_address != end_address)) memory_line = last_line;
        #pragma hls_unroll words_per_bus_cycle
         for (int j=0; j<words_per_bus_cycle; j++) {
            if (((d+j) >= 0) && ((d+j) < size)) {
              // copy bitwise
             #pragma hls_unroll bits
              for (int b=0; b<bits; b++) memory_line[j*bits+b] = memory[d+j][b];
            }
         }
         dma_write_data.write(memory_line);
         address++;
         d += words_per_bus_cycle;
         if (d >= size) break;
      }
   }
}


template<typename ARRAY_TYPE>
void dma_store_float(
   ARRAY_TYPE             *memory,
   address_type            base,
   index_type              offset,  // in words, not lines
   index_type              size,    // in words, not lines
   ac_channel<dma_info_t> &dma_write_ctrl,
   ac_channel<dma_data_t> &dma_write_data,
   ac_channel<dma_info_t> &dma_read_ctrl,
   ac_channel<dma_data_t> &dma_read_data)
{
   const int   bits                 = 32;
   const int   words_per_bus_cycle  = (BUS_WIDTH / bits);
   const int   bytes_per_bus_cycle  = (BUS_WIDTH / 8);
   bool        dma_done             = false;

   index_type start_address = (base / bytes_per_bus_cycle) + (offset / words_per_bus_cycle);
   index_type end_address   = (base / bytes_per_bus_cycle) + ((offset + (size-1)) / words_per_bus_cycle);
   index_type address;
   index_type d = (offset / words_per_bus_cycle) * words_per_bus_cycle - offset;
   index_type e = ((offset + size) / words_per_bus_cycle) * words_per_bus_cycle - (offset + size);

   dma_info_t  dma_read_info;
   dma_info_t  dma_write_info;

   dma_write_info.index             = start_address;
   dma_write_info.length            = end_address - start_address + 1;
   dma_write_info.size              = DMA_SIZE; // 64 bits

   dma_data_t  memory_line;
   dma_data_t  last_line;

   ac_ieee_float32 f[BUS_WIDTH/32];

   bool aligned_start = (d == 0);
   bool aligned_end   = (e == 0);

   if (!aligned_start) {
      dma_read_info.index = start_address;
      dma_read_info.length = 1;
      dma_read_info.size = DMA_SIZE;
     
      dma_done = false;

      do {
         dma_done = dma_read_ctrl.nb_write(dma_read_info);
      } while (!dma_done);

      if (dma_done) memory_line = dma_read_data.read();
   }

   if (!aligned_end && (end_address != start_address)) {
      dma_read_info.index = end_address;
      dma_read_info.length = 1;
      dma_read_info.size = DMA_SIZE;

      dma_done = false;

      do {
         dma_done = dma_read_ctrl.nb_write(dma_read_info);
      } while (!dma_done);
      
      if (dma_done) last_line = dma_read_data.read();
   }
      
   dma_done = false;

   do {
      dma_done = dma_write_ctrl.nb_write(dma_write_info);  
   } while (!dma_done);

   if (dma_done) {  // force serialization between config and data transfer
      address = start_address;
      for (index_type i=0; ; i+=words_per_bus_cycle) {
         if ((address == end_address) && (start_address != end_address)) memory_line = last_line;
        #pragma hls_unroll words_per_bus_cycle
         for (int j=0; j<words_per_bus_cycle; j++) {
            if (((d+j) >= 0) && ((d+j) < size)) {
               // copy bitwise
              #pragma hls_unroll bits
               f[j] = memory[d+j];
               memory_line.set_slc(j*bits, f[j].data_ac_int());
            }
         }
         dma_write_data.write(memory_line);
         address++;
         d += words_per_bus_cycle;
         if (d >= size) break;
      }
   }
}

