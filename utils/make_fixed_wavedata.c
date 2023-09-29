#include <stdlib.h>
#include <stdio.h>

#include "read_wave.h"

#define SAMPLE_SIZE 16000

unsigned long float_to_fixed(float f[], int par)
{
   // packs 'par' floating point values into a 32 bit array as fixed point numbers

   unsigned long bits;
   int shift;
   unsigned long mask;
   long value;
   int p;

   shift = 16/par;

   if (par == 1) mask = 0xFFFFFFFF;
   else mask = (1 << (shift * 2)) - 1;

   bits = 0;

   for (p=0; p<par; p++) {
      value = f[p] * (1 << shift);
      value &= mask;
      bits |= value << (shift * 2 * p);
   }

   return bits;
}


int main(int argument_count, char *arguments[])
{
   int r;
   int w;
   int p;
   int par;
   int line;
   int width;
   int count;
   unsigned int bits;
   float f[32];
   float wavedata[16000];
   
   if (argument_count != 4) {
      fprintf(stderr, "Usage: make_float_wavedata <wave_file> <words_per_32_bits> <bus_width> \n");
      return 1;
   }

   if (0 == read_wavefile(arguments[1], wavedata)) {
      fprintf(stderr, "error reading wavefile \"%s\" \n", arguments[1]);
      return 1;
   } 

   par = atoi(arguments[2]);

   if ((par<1) || (32<par)) {
      fprintf(stderr, "words_per_32_bits must be between 1 and 32.  Value found was: %d \n", par);
      return 1;
   }

   width = atoi(arguments[3]);

   if ((width<1) || (16<width)) {
      fprintf(stderr, "width must be between 1 and 16.  Value found was: %d \n", width);
      return 1;
   }

   count = 0;
   line = 0;

   while (count<SAMPLE_SIZE) {
      for (w=0; w<width; w++) { 
         for (p=0; p<par; p++) {
            if (count+p<SAMPLE_SIZE) {
               f[p] = wavedata[count];
            } else {
               f[p] = 0.0;
            }
         }
         if ((count<SAMPLE_SIZE) && (w==0))  printf("%06x: ", line);
         if ((count>=SAMPLE_SIZE) && (w==0)) break;
         count += par;
         bits = float_to_fixed(f, par);
         printf("%08x", bits);
      }
      if (w>0) {
         printf("\n");
         line++;
      }     
   }

   fprintf(stderr, "Processed %d values \n", count);
   return 0; 
}

