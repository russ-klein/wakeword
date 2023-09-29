import file_access
import read_wave

import numpy as np
import matplotlib.pyplot as plot
from scipy.signal import get_window 
from scipy.fftpack import fft

def display_pcm_waveform(w):
    plot.figure()
    plot.plot(w)
    plot.show()

def normalize_audio(audio):
    ''' increases the amplitude of the waveform, essentially  
        turn the volume up as much as possible without distortion '''
    audio = audio / np.max(np.abs(audio))
    return audio


def frame_audio(audio, FFT_size=320, hop_size=10, sample_rate=16000):
    ''' chops up the audio into frames, frame needs to hold a full wave (2pi) so I picked 20 ms, with 50% overlap '''
    # hop_size in ms
    
    audio = np.pad(audio, int(FFT_size / 2), mode='reflect')
    print('audio shape: ', len(audio))
    frame_len = np.round(sample_rate * hop_size / 1000).astype(int)
    print('frame length: ', frame_len)
    frame_num = int((len(audio) - FFT_size) / frame_len) + 1
    print('frame_num: ', frame_num)
    frames = np.zeros((frame_num,FFT_size))
    
    for n in range(frame_num):
        frames[n] = audio[n*frame_len:n*frame_len+FFT_size]
    
    return frames

def freq_to_mel(freq):
    return 2595.0 * np.log10(1.0 + freq / 700.0)

def met_to_freq(mels):
    return 700.0 * (10.0**(mels / 2595.0) - 1.0)

def get_filter_points(fmin, fmax, mel_filter_num, FFT_size, sample_rate):
    fmin_mel = freq_to_mel(fmin)
    fmax_mel = freq_to_mel(fmax)
    
    print("MEL min: {0}".format(fmin_mel))
    print("MEL max: {0}".format(fmax_mel))
    
    mels = np.linspace(fmin_mel, fmax_mel, num=mel_filter_num+2)
    freqs = met_to_freq(mels)
    
    return np.floor((FFT_size + 1) / sample_rate * freqs).astype(int), freqs

def get_filters(filter_points, FFT_size):
    filters = np.zeros((len(filter_points)-2,int(FFT_size/2+1)))
    
    for n in range(len(filter_points)-2):
        filters[n, filter_points[n] : filter_points[n + 1]] = np.linspace(0, 1, filter_points[n + 1] - filter_points[n])
        filters[n, filter_points[n + 1] : filter_points[n + 2]] = np.linspace(1, 0, filter_points[n + 2] - filter_points[n + 1])
    
    return filters

def dct(dct_filter_num, filter_len):
    basis = np.empty((dct_filter_num,filter_len))
    basis[0, :] = 1.0 / np.sqrt(filter_len)
    
    samples = np.arange(1, 2 * filter_len, 2) * np.pi / (2.0 * filter_len)

    for i in range(1, dct_filter_num):
        basis[i, :] = np.cos(i * samples) * np.sqrt(2.0 / filter_len)
        
    return basis

def main():
    # parameters for the algorithm
  
    sample_rate = 16000
    hop_size = 20 #ms
    fmin = 20
    fmax = int(sample_rate / 2)
    num_mel_filters = 30
    num_dct_filters = 29
    overlap = 2
    fft_size = int((sample_rate / 1000) * hop_size * overlap)

    print('sample_rate:     ', sample_rate)
    print('hop_size:        ', hop_size)
    print('fmin:            ', fmin)
    print('fmax:            ', fmax)
    print('num_mel_filters: ', num_mel_filters)
    print('num_dct_filters: ', num_dct_filters)
    print('overlap:         ', overlap)
    print('fft_size:        ', fft_size)
    print(' ')

    pcm_audio = read_wave.read_wavefile('../testdata/dog.wav')

    loud_pcm_audio = normalize_audio(pcm_audio)
    frames = frame_audio(loud_pcm_audio, fft_size, )
    
    window = get_window("hann", fft_size, fftbins=True)

    attenuated_frames = frames * window

    attenuated_frames_transposed = np.transpose(attenuated_frames)

    audio_fft = np.empty((int(1 + fft_size // 2), attenuated_frames_transposed.shape[1]), dtype=np.complex64, order='F')

    for n in range(audio_fft.shape[1]):
       audio_fft[:, n] = fft(attenuated_frames_transposed[:, n], axis=0)[:audio_fft.shape[0]]

    audio_fft = np.transpose(audio_fft)

    audio_power = np.square(np.abs(audio_fft))


    filter_points, mel_frequencies = get_filter_points(fmin, fmax, num_mel_filters, fft_size, sample_rate)

    filters = get_filters(filter_points, fft_size)

    enorm = 2.0 / (mel_frequencies[2:num_mel_filters+2] - mel_frequencies[:num_mel_filters])
    filters = filters * enorm[:, np.newaxis]

    audio_filtered = np.dot(filters, np.transpose(audio_power))
    audio_log = 10.0 * np.log10(audio_filtered)

    dct_filters = dct(num_dct_filters, num_mel_filters)

    cepstral_coefficents = np.dot(dct_filters, audio_log)

    return cepstral_coefficents


