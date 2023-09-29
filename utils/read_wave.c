
#include <stdio.h>
#include <string.h>

int read_wavefile(char *filename, float *buffer)
{
    char *program_name = "read_wavefile";

    FILE *f = fopen(filename, "r");
    short raw_data[SAMPLE_SIZE];
    int i;
    int r;
    int read_size;

    int chatty = 0;
    
    char  riff[5]          = { 00, 00, 00, 00, 00 };
    int   chunk_size;
    int   data_size;
    char  file_format[5]   = { 00, 00, 00, 00, 00 };
    char  sub_chunk1_id[5] = { 00, 00, 00, 00, 00 };
    int   sub_chunk1_size;
    short audio_format;
    short num_channels;
    int   sample_rate;
    int   byte_rate;
    short block_align;
    short bits_per_sample;
    char  sub_chunk2_id[5] = { 00, 00, 00, 00, 00 };
    int   sub_chunk2_size;

    if (f == NULL) {
       fprintf(stderr, "Unable to open file \"%s\" for reading \n", filename);
       perror(program_name);
       return 0;
    }
    
    //
    // Read 44 byte header
    //

    //-- Chunk ID, should be 'RIFF'

    r = fread(riff, 1, 4, f);
    if (r != 4) {
       fprintf(stderr, "Unable to read RIFF field from file \"%s\" \n", filename);
       perror(program_name);
       fclose(f);
       return 0;
    }

    if (strcmp(riff, "RIFF")) {
       fprintf(stderr, "\"%s\" does not appear to be a propoer RIFF file. Expected to see chunk ID \"RIFF\", but found: %s \n", filename, riff);
       fclose(f);
       return 0;
    }

    if (chatty) printf("Chunk ID: %s \n", riff);

    //-- Size of the data payload

    r = fread(&chunk_size, 4, 1, f);
    if (r != 1) {
       fprintf(stderr, "Unable to read chunk size field from file \"%s\" \n", filename);
       perror(program_name);
       fclose(f);
       return 0;
    }

    data_size = chunk_size - 36;

    if (chatty) printf("Wave file has %d bytes of data \n", data_size);

    //-- Format identifier, should be "WAVE"

    r = fread(file_format, 1, 4, f);
    if (r != 4) {
       fprintf(stderr, "Unable to read file_format field from file \"%s\" \n", filename);
       perror(program_name);
       fclose(f);
       return 0;
    }

    if (strcmp(file_format, "WAVE")) {
       fprintf(stderr, "\"%s\" does not appear to be a proper WAVE file. Expected to see format of \"WAVE\", but found: %s \n", filename, file_format);
       fclose(f);
       return 0;
    }

    if (chatty) printf("File format: %s \n", file_format);

    //-- Sub-chunk1 ID, should be "fmt "
    
    r = fread(sub_chunk1_id, 1, 4, f);
    if (r != 4) {
       fprintf(stderr, "Unable to read sub_chunk_id1 field from file \"%s\" \n", filename);
       perror(program_name);
       fclose(f);
       return 0;
    }

    if (strcmp(sub_chunk1_id, "fmt ")) {
       fprintf(stderr, "\"%s\" does not appear to have the right format. Expected to see sub-chunk1 id of \"fmt \", but found: %s \n", filename, file_format);
       fclose(f);
       return 0;
    }

    //-- Sub-chunk1 size

    r = fread(&sub_chunk1_size, 4, 1, f);
    if (r != 1) {
       fprintf(stderr, "Unable to read sub-chunk1 size field from file \"%s\" \n", filename);
       perror(program_name);
       fclose(f);
       return 0;
    }

    if (chatty) printf("Sub-chunk1 size: %d \n", sub_chunk1_size);

    //-- Audio format, should be 1

    r = fread(&audio_format, 2, 1, f);
    if (r != 1) {
       fprintf(stderr, "Unable to read audio format field from file \"%s\" \n", filename);
       perror(program_name);
       fclose(f);
       return 0;
    }

    if (audio_format != 1) {
       fprintf(stderr, "\"%s\" is not a PCM waveform.  Expected audio format to be 1, but found: %d \n", filename, audio_format);
       fclose(f);
       return 0;
    }

    if (chatty) printf("audio format: %d \n", audio_format);

    //-- Number of channels, 1 for mono, 2 for stereo

    r = fread(&num_channels, 2, 1, f);
    if (r != 1) {
       fprintf(stderr, "Unable to read number of channels field from file \"%s\" \n", filename);
       perror(program_name);
       fclose(f);
       return 0;
    }

    if (chatty) printf("Number of channels: %d \n", num_channels);

    //-- Sample rate
   
    r = fread(&sample_rate, 4, 1, f);
    if (r != 1) {
       fprintf(stderr, "Unable to read sample rate field from file \"%s\" \n", filename);
       perror(program_name);
       fclose(f);
       return 0;
    }

    if (chatty) printf("Sample rate: %d \n", sample_rate);

    //-- Byte rate
   
    r = fread(&byte_rate, 4, 1, f);
    if (r != 1) {
       fprintf(stderr, "Unable to read byte rate field from file \"%s\" \n", filename);
       perror(program_name);
       fclose(f);
       return 0;
    }

    if (chatty) printf("Byte rate: %d \n", byte_rate);

    //-- Block alignment

    r = fread(&block_align, 2, 1, f);
    if (r != 1) {
       fprintf(stderr, "Unable to read aligmnent field from file \"%s\" \n", filename);
       perror(program_name);
       fclose(f);
       return 0;
    }

    if (chatty) printf("Block alignment: %d \n", block_align);

    //-- Bits per sample

    r = fread(&bits_per_sample, 2, 1, f);
    if (r != 1) {
       fprintf(stderr, "Unable to read bits per sample field from file \"%s\" \n", filename);
       perror(program_name);
       fclose(f);
       return 0;
    }

    if (chatty) printf("Bits per sample: %d \n", bits_per_sample);

    //-- Sub-chunk2 ID, should be "data"

    r = fread(sub_chunk2_id, 1, 4, f);
    if (r != 4) {
       fprintf(stderr, "Unable to read sub_chunk_id2 field from file \"%s\" \n", filename);
       perror(program_name);
       fclose(f);
       return 0;
    }

    if (strcmp(sub_chunk2_id, "data")) {
       fprintf(stderr, "\"%s\" does not appear to be a proper WAVE file. Expected to see format of \"data\", but found: %s \n", filename, sub_chunk2_id);
       fclose(f);
       return 0;
    }

    //-- Sub-chunk2 size

    r = fread(&sub_chunk2_size, 4, 1, f);
    if (r != 1) {
       fprintf(stderr, "Unable to read sub-chunk2 size field from file \"%s\" \n", filename);
       perror(program_name);
       fclose(f);
       return 0;
    }

    if (chatty) printf("Sub-chunk2 size: %d \n", sub_chunk2_size);

    //
    // Read data values
    //

    fseek(f, 44, SEEK_SET);
    
    read_size = data_size / (bits_per_sample/8);
    if (read_size > SAMPLE_SIZE) read_size = SAMPLE_SIZE;

    r = fread(raw_data, (bits_per_sample/8), read_size, f);
    if (r != read_size) {
       fprintf(stderr, "Unable to read data payload from \"%s\" \n", filename);
       perror(program_name);
       return 0;
    }


/*
    // clip quite part in the front of the waveform.  Experimental

    for (i=0; i<read_size; i++) {
       if (abs(raw_data[i]) > 164) break;
    }

    int clip = i;

    for (i=0; i<read_size-clip; i++) {
       raw_data[i] = raw_data[i+clip];
    }

    printf("clipped %d \n", clip);

    read_size -= clip;
*/



    // pad to SAMPLE_SIZE if too short

    if (read_size < SAMPLE_SIZE) {
       for (i=read_size; i<SAMPLE_SIZE; i++) {
          raw_data[i] = 0;
       }
    }
    
    // scale to -1.0 to 1.0

    for (i=0; i<SAMPLE_SIZE; i++) {
       buffer[i] = ((float)raw_data[i])/32767.0;
if (i<20) printf("buffer[%d] = %f \n", i, buffer[i]);
    }
    
    fclose(f);

    return 1;
}

