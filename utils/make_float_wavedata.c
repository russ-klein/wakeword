#include <stdlib.h>
#include <stdio.h>

#include "read_wave.h"

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
   float wavedata[SAMPLE_SIZE];
   
   if (argument_count != 3) {
      fprintf(stderr, "Usage: make_float_wavedata <wave_file> <bus_width> \n");
      return 1;
   }

   if (0 == read_wavefile(arguments[1], wavedata)) {
      fprintf(stderr, "error reading wavefile \"%s\" \n", arguments[1]);
      return 1;
   } 

   width = atoi(arguments[2]);

   if ((width<1) || (16<width)) {
      fprintf(stderr, "width must be between 1 and 16.  Value found was: %d \n", width);
      return 1;
   }

   count = 0;
   line = 0;

   while (count<SAMPLE_SIZE) {
      for (w=0; w<width; w++) { 
         bits = *(int*)(void*)(&(wavedata[count]));
         if ((count>=SAMPLE_SIZE) && (w==0)) break;
         if ((count<SAMPLE_SIZE) && (w==0))  printf("%06d: ", line);
         printf("%08x", bits);
         // printf("%f ", wavedata[count]);
         count++;
      }
      if (w>0) {
         printf("\n");
         line++;
      }     
   }

   fprintf(stderr, "Processed %d values \n", count);
   return 0; 
}

