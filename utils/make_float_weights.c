#include <stdlib.h>
#include <stdio.h>

int main(int argument_count, char *arguments[])
{
   int r;
   int w;
   int p;
   int par;
   int line;
   int width;
   int count;
   unsigned long bits;
   float f[32];
   FILE *binary_floats;
   
   if (argument_count != 3) {
      fprintf(stderr, "Usage: make_float_weights <weight_file> <bus_width> \n");
      return 1;
   }

   binary_floats = fopen(arguments[1], "r");

   if (NULL == binary_floats) {
      fprintf(stderr, "Unable to open file '%s' for reading \n", arguments[1]);
      perror("make_fixed_weights");
      return 1;
   }

   width = atoi(arguments[2]);

   if ((width<1) || (16<width)) {
      fprintf(stderr, "width must be between 1 and 16.  Value found was: %d \n", width);
      return 1;
   }

   count = 0;
   line = 0;

   while (!feof(binary_floats)) {
      for (w=0; w<width; w++) { 
         r = fread(&bits, 4, 1, binary_floats);
         if ((r>0) && (w==0))  printf("%06x: ", line);
         if ((r==0) && (w==0)) break;
         count += r;
         printf("%08lx", bits);
      }
      if (w>0) {
         printf("\n");
         line++;
      }     
   }
   fclose(binary_floats);

   fprintf(stderr, "Processed %d values \n", count);
   return 0; 
}

