import read_wave
import mfcc_to_header_file
import dsp

#create an mfcc from a wave file (pick any of 'zero' thru 'nine')
file_list = [ '../testdata/zero.wav',
              '../testdata/one.wav',
              '../testdata/two.wav',
              '../testdata/three.wav',
              '../testdata/four.wav',
              '../testdata/five.wav',
              '../testdata/six.wav',
              '../testdata/seven.wav',
              '../testdata/eight.wav',
              '../testdata/nine.wav' ]


header_file = mfcc_to_header_file.Header_writer('../include/ref_mfccs.h')

for i in range(len(file_list)):
   waveform = read_wave.read_wavefile(file_list[i])
   mfcc = dsp.mfcc(waveform, fs=16000)
   header_file.write_mfcc(mfcc, i, stride=5)

header_file.close()


