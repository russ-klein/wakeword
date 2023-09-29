import numpy as np
import struct

def read_wavefile(filename):
   # if chatty print header as debug messages
   chatty =  False
   return_array = [];

   ''' returns a numpy array floating point array of PCM values from 'filename' and scaled from -1.0 to 1.0 '''

   wave_file = open(filename, 'rb');

   #
   # read 44 byte header
   #

   #-- chunk id, should be 'RIFF'

   chunk_id = wave_file.read(4)

   if (chunk_id != b'RIFF'):
      print(filename, ' does not appear to be a proper RIFF file. Expected to see chunk ID "RIFF", but found: ', chunk_id)
      return return_array

   if chatty:
      print('Chunk ID: RIFF');

   #-- size of the data payload

   chunk_size_string = wave_file.read(4);
   chunk_size = struct.unpack('I', chunk_size_string)[0]
   data_size = chunk_size - 36 # just the way the size is encoded 

   if chatty:
      print('Wave file has ', data_size, ' bytes of data')

   #-- format identifier, should be 'WAVE'

   file_format = wave_file.read(4)
   
   if (file_format != b'WAVE'):
      print(filename, ' does not appear to be a proper WAVE file.  Expected to see format of "WAVE", but found: ', file_format)
      return return_array
   
   if chatty:
      print('File format: WAVE')

   #-- sub-chunk ID, should be fmt

   sub_chunk1_id = wave_file.read(4)

   if (sub_chunk1_id != b'fmt '):
      print(filename, ' does not appear to hve the right format.  Expected to see sub-chunk1 id of "fmt ", but found: ', sub_chunk1_id)
      return return_array

   if chatty:
      print('Sub-chunk1 ID: fmt')

   #-- sub-chunk size

   sub_chunk1_size_string = wave_file.read(4)
   sub_chunk1_size = struct.unpack('I', sub_chunk1_size_string)[0]

   if chatty:
      print('Sub-chunk1 size: ', sub_chunk1_size)

   #-- audio format string, should be 1

   audio_format_string = wave_file.read(2)
   audio_format = struct.unpack('H', audio_format_string)[0]

   if (audio_format != 1):
      print(filename, ' is not a PCM waveform.  Expected audio format to be 1, but found: ', audio_format)

   if chatty:
      print('Audio format: ', audio_format)

   #-- number of channels, should be 1 (mono) or 2 (stereo)

   num_channels_string = wave_file.read(2)
   num_channels = struct.unpack('H', num_channels_string)[0]

   if chatty:
      print('Number of channels: ', num_channels)

   #-- sample rate

   sample_rate_string = wave_file.read(4) 
   sample_rate = struct.unpack('I', sample_rate_string)[0]

   if chatty:
      print('Sample rate: ', sample_rate)


   #-- byte rate

   byte_rate_string = wave_file.read(4)
   byte_rate = struct.unpack('I', byte_rate_string)[0]

   if chatty:
      print('Byte rate: ', byte_rate)

   #-- alignment

   block_align_string = wave_file.read(2)
   block_align = struct.unpack('H', block_align_string)[0]

   if chatty:
      print('Block align: ', block_align)

   #-- bits per sample

   bits_per_sample_string = wave_file.read(2) 
   bits_per_sample = struct.unpack('H', bits_per_sample_string)[0]

   if chatty:
      print('Bits per sample: ', bits_per_sample)

   #-- sub chunk2 id, should be 'data'

   sub_chunk2_id = wave_file.read(4)

   if (sub_chunk2_id != b'data'):
      print(filename, ' does not appear to have the right format.  Expected to see sub-chunk2 id of "data", but found: ', sub_chunk2_id)
      return return_array

   if chatty:
      print('Sub-chunk2 ID: data')

   #-- sub chunk2 size

   sub_chunk2_size_string = wave_file.read(4)
   sub_chunk2_size = struct.unpack('I', sub_chunk2_size_string)[0]

   if chatty:
      print('Sub-chunk2 size: ', sub_chunk2_size)

   #
   # read data values
   #

   ch0 = []
   ch1 = []   

   for i in range(int(sub_chunk2_size/((bits_per_sample/8) * num_channels))):
      sample = wave_file.read(2)
      ch0.append((struct.unpack('h', sample))[0])
      if num_channels > 1:
         sample = wave_file.read(2)
         ch1.append((struct.unpack('h', sample))[0])
  
   wave_file.close()

   # if wave is less than one second, pad with 0s

   if len(ch0) < 16000:
      ch0 = ch0 + (16000 - len(ch0)) * [0]

   if len(ch0) > 16000:
      ch0 = ch0[:16000]

   # format as numpy floating point array, and scale to -1.0 to 1.0

   if num_channels > 1:
      return_array = np.array(ch0)/32768.0, np.array(ch1)/32768.0
   else:
      return_array = np.array(ch0)/32768.0

   return return_array

