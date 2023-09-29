import file_access

from keras.models import Sequential
from keras.layers import Dense, Conv2D, Flatten
import numpy as np


#load training data

dataset = file_access.create_features_and_labels(['zero', 'one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine'], training_size = 1000, testing_size=250)
print('data set retrieved')
training_features = dataset[0].reshape(dataset[0].shape + (1, ))
training_labels   = np.array(file_access.digits_to_categorical(dataset[1]))
testing_features  = dataset[2].reshape(dataset[2].shape + (1, ))
testing_labels    = np.array(file_access.digits_to_categorical(dataset[3]))

num_classes = 10

print('feature shape: ', training_features.shape)
print('testing shape: ', testing_features.shape)

# define baseline model
def keyword_model():
        # create model
        model = Sequential()
        model.add(Conv2D(186, (7,7), use_bias=True, padding="same", activation="relu", input_shape=(101,20,1)))
        model.add(Flatten())
        model.add(Dense(128, use_bias=True, kernel_initializer='normal', activation=None))
        model.add(Dense(128, use_bias=True, kernel_initializer='normal', activation=None))
        model.add(Dense(10,  use_bias=True, kernel_initializer='normal', activation='softmax'))
        # Compile model
        model.compile(loss='categorical_crossentropy', optimizer='adam', metrics=['accuracy'])
        return model

model = keyword_model()
model.fit(training_features, training_labels, validation_data = (testing_features, testing_labels), epochs=100, batch_size=500, verbose = 1)
scores=model.evaluate(testing_features, testing_labels, verbose=0)
print('baseline error: ', 100-scores[1]*100)
