from keras.models import Sequential
from keras.layers import Dense, Conv2D, Flatten

# define basic model
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

def keyword_nopad_model():
        # create model
        model = Sequential()
        model.add(Conv2D(186, (7, 20), use_bias=True, padding="VALID", activation="relu", input_shape=(101,20,1)))
        model.add(Flatten())
        model.add(Dense(128, use_bias=True, kernel_initializer='normal', activation=None))
        model.add(Dense(128, use_bias=True, kernel_initializer='normal', activation=None))
        model.add(Dense(10,  use_bias=True, kernel_initializer='normal', activation='softmax'))
        # Compile model
        model.compile(loss='categorical_crossentropy', optimizer='adam', metrics=['accuracy'])
        return model

