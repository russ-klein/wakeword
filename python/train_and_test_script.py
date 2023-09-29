import numpy as np

import file_access
import read_wave
import keyword_model
import dsp

#load training data

dataset = file_access.create_features_and_labels(['zero', 'one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine'], training_size = 15000, testing_size=1000)
#dataset = file_access.create_features_and_labels(['zero', 'one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine'], training_size = 100, testing_size=100)
print('data set retrieved')
training_features = dataset[0].reshape(dataset[0].shape + (1, ))
training_labels   = np.array(file_access.digits_to_categorical(dataset[1]))
testing_features  = dataset[2].reshape(dataset[2].shape + (1, ))
testing_labels    = np.array(file_access.digits_to_categorical(dataset[3]))

model = keyword_model.keyword_nopad_model()

model.fit(training_features, training_labels, validation_data = (testing_features, testing_labels), epochs=100, batch_size=1000, verbose = 1)
scores=model.evaluate(testing_features, testing_labels, verbose=0)
print('baseline error: ', 100-scores[1]*100)

def check(wave_file, digit):
 number = read_wave.read_wavefile(wave_file)
 number_mfcc = dsp.mfcc(number, fs=16000)
 number_mfcc = number_mfcc.reshape((1, ) + number_mfcc.shape + (1, ))
 p = model.predict(number_mfcc)
 p = p[0]
 if max(p) == p[digit]:
  print(digit, 'passed, p = ', (p*100).astype(int))
  return 0
 else:
  print(digit, 'failed, p = ', (p*100).astype(int))
  return 1

errors = 0
errors += check('../testdata/zero.wav',  0)
errors += check('../testdata/one.wav',   1)
errors += check('../testdata/two.wav',   2)
errors += check('../testdata/three.wav', 3)
errors += check('../testdata/four.wav',  4)
errors += check('../testdata/five.wav',  5)
errors += check('../testdata/six.wav',   6)
errors += check('../testdata/seven.wav', 7)
errors += check('../testdata/eight.wav', 8)
errors += check('../testdata/nine.wav',  9)

if errors>0:
  print('you lose: ', errors, ' wrong, do more training ')
else: 
  print('you win!!!')



