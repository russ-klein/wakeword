
import random
import os

import numpy as np
import tensorflow as tf
import tensorflow_io as tfio

import read_wave
import dsp

path = '../datasets'

words = [
         'backward',
         'bed',
         'bird',
         'cat',
         'dog',
         'down',
         'eight',
         'five',
         'follow',
         'forward',
         'four',
         'go',
         'happy',
         'house',
         'learn',
         'left',
         'marvin',
         'nine',
         'no',
         'off',
         'on',
         'one',
         'right',
         'seven',
         'sheila',
         'six',
         'stop',
         'three',
         'tree',
         'two',
         'up',
         'visual',
         'wow',
         'yes',
         'zero'
        ]

def random_word():
  return words[random.randint(0, len(words)-1)]

def random_file_name(directory_string):
  file_list = os.listdir(directory_string)
  idx = random.randint(0, len(file_list)-1)
  print('index: ', idx, '/', len(file_list))
  file_name = file_list[random.randint(0, len(file_list)-1)]
  print('filename: ', file_name)
  return directory_string + '/' + file_name

def random_file(word=None):
  ''' returns a random wave filename, if word is given, it will be from the 'word' directory '''
  if word==None:
    word = random_word();

  file_name = random_file_name(path + '/' + word)

  return file_name

def get_all_filenames(word_list = []):
  ''' returns a list of files (to be used as feature maps) and labels '''
  if len(word_list) == 0:
    word_list = words

  big_list = []
  labels = []
  for word in word_list:
    #print("Current word: ", word)
    list_of_files = os.listdir(path + '/' + word)
    for file in list_of_files:
      if file[-4:] == '.wav':  # only select files with the suffix ".wav"
        big_list.append(path + '/' + word + '/' + file)
        labels.append(word)

  return np.array(big_list), np.array(labels)


def create_feature_maps(file_list):
  ''' returns a list of feature maps corresponding to files in the list ''' 

  features = []
  for wave_file in file_list:
    print('wavefile: ', wave_file)
    audio_waveform = read_wave.read_wavefile(wave_file)
    mfcc = dsp.mfcc(audio_waveform, fs=16000)
    '''
    spectrogram = tfio.audio.spectrogram(audio_waveform, nfft=512, window=512, stride=256)
    spectrogram = tf.cast(spectrogram, tf.float32)
    mel_spectrogram = tfio.audio.melscale(spectrogram, rate=16000, mels=128, fmin=0, fmax=8000)
    print('size of spectrogram: ', np.shape(spectrogram))
    print('size of mel_spectrogram: ', np.shape(mel_spectrogram))
    features.append(mel_spectrogram)
    '''
    features.append(mfcc)

  return np.array(features)

def create_features_and_labels(word_list=[], training_size=10000, testing_size=2000):
  
  if len(word_list) == 0:
    word_list = words

  filename_list, label_list = get_all_filenames(word_list)
  print('length of all files: ', len(filename_list))

  select_idx = random.sample(range(len(filename_list)), training_size + testing_size)

  filename_list = filename_list[select_idx];
  label_list    = label_list[select_idx];
  
  print('new length of all files: ', len(filename_list))

  training_features = create_feature_maps(filename_list[0:training_size])
  training_labels   = label_list[0:training_size]

  testing_features  = create_feature_maps(filename_list[training_size:training_size+testing_size])
  testing_labels    = label_list[training_size:training_size+testing_size]

  return training_features, training_labels, testing_features, testing_labels

def digits_to_categorical(digit_labels):
  cats = []
  for digit in digit_labels:
    this_cat = 10 * [0.0]
    if digit == 'zero':
      this_cat[0] = 1.0
    if digit == 'one':
      this_cat[1] = 1.0
    if digit == 'two':
      this_cat[2] = 1.0
    if digit == 'three':
      this_cat[3] = 1.0
    if digit == 'four':
      this_cat[4] = 1.0
    if digit == 'five':
      this_cat[5] = 1.0
    if digit == 'six':
      this_cat[6] = 1.0
    if digit == 'seven':
      this_cat[7] = 1.0
    if digit == 'eight':
      this_cat[8] = 1.0
    if digit == 'nine':
      this_cat[9] = 1.0
    cats.append(this_cat)
  return cats
