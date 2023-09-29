import keyword_model
import read_wave
import dsp

#create inference model
model = keyword_model.keyword_nopad_model()

#you can train it, but to save time, load a known good set of weights
model.load_weights('small_weights')

#create an mfcc from a wave file (pick any of 'zero' thru 'nine')
number_data = read_wave.read_wavefile('../testdata/three.wav')
number_mfcc = dsp.mfcc(number_data, fs=16000)
number_mfcc = number_mfcc.reshape((1,) + number_mfcc.shape + (1,))

#Make a prediction
p = model.predict(number_mfcc)
print((p*100).astype(int))
