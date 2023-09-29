
#ifndef __DEFINES_H_INCLUDED__
#define __DEFINES_H_INCLUDED__

// defines for local compile, should be set on cc line
//
// #define WEIGHT_MEMORY  - use a memory region for weights, else use a C++ variable. Required if cross compiling for ARM.
// #define FIXED_POINT    - use ac_fixed for weights and features, else use floats
// #define HOST           - compiled to run on the HOST, else compiled to hardware.  Define HOST for running with C++ DUT.
// #define SLAVE          - uses slave interface on accelerator, else use master
// #define ARM            - cross compile for ARM
//

// this file defines:
//   PAR_IN           number of values per 32 bit memory location
//   WORD_SIZE        number of bits in the numeric representation of weights, biases and features
//   INTEGER_BITS     number of integer bits
//   FRACTIONAL_BITS  number of fractional bits
//   BUS_WIDTH        number of bits on the bus
//   STRIDE           number of values read per bus cycle
// 
//   bus_type
//   cat_memory_type
//   raw_bus_type     ac_int of full bus width
//   bus type         array of cat_memory_types for full bus_width

#define SAMPLE_SIZE 16000
#define SPECTROGRAM_SIZE (101 * 20)

#define IMAGE_HEIGHT 101
#define IMAGE_WIDTH   20

#define FILTER_HEIGHT  7
#define FILTER_WIDTH  20

#define FILTER_BITS    6

#define IMAGE_SIZE   (IMAGE_HEIGHT * IMAGE_WIDTH)
#define FILTER_SIZE  (FILTER_HEIGHT * FILTER_WIDTH)

#ifdef HOST
#include "ac_int.h"
#include "ac_channel.h"
#include "mc_scverify.h"
#include "ac_fixed.h"
#include "ac_float.h"
#include "ac_std_float.h"
#include "ac_sync.h"
#endif

#ifndef PAR_IN
#define PAR_IN  (1)
#endif // ndef PAR_IN

#define PAR_OUT (PAR_IN)
#define WEIGHT_MEMORY_SIZE (100000)

#ifdef FIXED_POINT

#if PAR_IN==1
#define WORD_SIZE 32
#define INTEGER_BITS 16
#endif // PAR_IN == 1

#if PAR_IN==2
#define WORD_SIZE 16
#define INTEGER_BITS 8
#endif // PAR_IN == 2

#if PAR_IN==3
#define WORD_SIZE 10
#define INTEGER_BITS 5
#endif // PAR_IN == 3

#if PAR_IN==4
#define WORD_SIZE 8
#define INTEGER_BITS 4
#endif // PAR_IN == 4

#if PAR_IN==5
#define WORD_SIZE 6
#define INTEGER_BITS 3
#endif // PAR_IN == 5

#define FRACTIONAL_BITS (WORD_SIZE-INTEGER_BITS)

// bus width must be power of 2, and represents number of 32 bit elements that can be passed on the bus
// BUS_WIDTH_BITS == 0 means 32 bit bus
// BUS_WIDTH_BITS == 1 means 64 bit bus
// BUS_WIDTH_BITS == 2 means 128 bit bus
// BUS_WIDTH_BITS == 3 means 256 bit bus
// BUS_WIDTH_BITS == 4 means 512 bit bus
// BUS_WIDTH_BITS == 5 means 1024 bit bus

#ifdef A53
 #define BUS_WIDTH_BITS 1
#else // not A53
 #define BUS_WIDTH_BITS 0
#endif

#define BUS_WIDTH (1<<BUS_WIDTH_BITS)
#define STRIDE (PAR_IN * BUS_WIDTH)

#ifdef HOST

typedef ac_fixed<WORD_SIZE, INTEGER_BITS, true, AC_RND, AC_SAT> hw_cat_type;
//typedef hw_cat_type                               cat_memory_type[STRIDE];
//typedef struct {hw_cat_type buffer[STRIDE];}  bus_type;

typedef ac_int<32 * BUS_WIDTH, false>             raw_bus_type; 
typedef raw_bus_type                              cat_memory_type;

//typedef struct {hw_cat_type buffer[PAR_IN];}    cat_memory_type_struct;
//typedef struct {hw_cat_type buffer[STRIDE];}    internal_memory_line_type;

#else // not HOST (i.e. EMBEDDED)

typedef unsigned int cat_memory_type;
typedef unsigned int hw_cat_type;
typedef float        sw_cat_type;

#endif // not HOST
#else // not FIXED_POINT

typedef float hw_cat_type;
typedef hw_cat_type cat_memory_type[PAR_IN];

#endif // else not FIXED_POINT

#ifdef ARM
typedef unsigned int bus_type;
#ifdef M3
// static bus_type *weight_memory = (bus_type *) 0x40000000;
#else  // A53
// static bus_type *weight_memory = (bus_type *) 0x20000000;
#endif
#else // not ARM
// static raw_bus_type         weight_memory[WEIGHT_MEMORY_SIZE];
#endif // not ARM

static const char program_name[] = "wakeword";

#endif
