from record import loadData
import numpy as np
from keras.models import Sequential
from keras.layers.core import Flatten, Dense, Dropout, Lambda
from keras.layers.convolutional import Convolution2D, MaxPooling2D, ZeroPadding2D
from keras.optimizers import SGD, Adam
from keras.preprocessing import image
from keras.layers.normalization import BatchNormalization

def onehot(value, dim=26):
    r = []
    for v in value:
        row = np.zeros(dim)
        row[v] = 1
        r.append(row)
    return np.array(r)

def norm_input(x):
    return x
    #mean_x = x.mean().astype(np.float32)
    #std_x = x.std().astype(np.float32)
    #mean_x = x.mean()
    #std_x = x.std()
    #return (x-mean_x) / std_x

def lin_model():
    model = Sequential([
        Lambda(norm_input, input_shape=(16,8,1)),
        Flatten(),
        Dense(26, activation='softmax')
        ])
    model.compile(Adam(), loss='categorical_crossentropy', metrics=['accuracy'])
    return model

def ConvBlock(model, layers, filters):
    for i in range(layers):
        model.add(ZeroPadding2D((1,1)))
        model.add(Convolution2D(filters, 3, 3, activation='relu'))
        #BatchNormalization()
    model.add(MaxPooling2D((2,2), strides=(1,1)))

def FCBlock(model):
    #model.add(Dense(32, activation='relu'))
    #model.add(Dense(32, activation='relu'))
    pass

def cnn_model():
    model = Sequential([Lambda(norm_input, input_shape=(16,8,1))])
    ConvBlock(model,2,16)
    ConvBlock(model,2,16)
    #ConvBlock(model,2,8)
    #ConvBlock(model,2,5)
    #ConvBlock(model,2,3)
    model.add(Flatten())
    FCBlock(model)
#    model.add(Dropout(0.3))
    #BatchNormalization()
    model.add(Dense(26, activation='softmax'))
    model.compile(Adam(), loss='categorical_crossentropy', metrics=['accuracy'])
    return model


if __name__ == "__main__":
    (x_train, y_train, i_train), (x_test, y_test, i_test) = loadData()
    x_train = np.expand_dims(x_train, 3)
    x_test = np.expand_dims(x_test, 3)
    y_train = onehot(y_train)
    y_test = onehot(y_test)
    modelPath = "./model4/"
    gen = image.ImageDataGenerator()
    #gen = image.ImageDataGenerator(rotation_range=10)
    batches = gen.flow(x_train, y_train, batch_size=16)
    test_batches = gen.flow(x_test, y_test, batch_size=16)
    lm = cnn_model()
    lm.fit_generator(batches, batches.N, nb_epoch=2,validation_data=test_batches, nb_val_samples=test_batches.N)
    lm.save_weights(modelPath + "2.pkl")
    lm.optimizer.lr = 0.001
    lm.fit_generator(batches, batches.N, nb_epoch=2,validation_data=test_batches, nb_val_samples=test_batches.N)
    lm.save_weights(modelPath + "4.pkl")
    lm.fit_generator(batches, batches.N, nb_epoch=2,validation_data=test_batches, nb_val_samples=test_batches.N)
    lm.save_weights(modelPath + "6.pkl")
    lm.fit_generator(batches, batches.N, nb_epoch=2,validation_data=test_batches, nb_val_samples=test_batches.N)
    lm.optimizer.lr = 0.0001
    lm.save_weights(modelPath + "8.pkl")
    lm.fit_generator(batches, batches.N, nb_epoch=1,validation_data=test_batches, nb_val_samples=test_batches.N)
    lm.save_weights(modelPath + "9.pkl")
    lm.fit_generator(batches, batches.N, nb_epoch=1,validation_data=test_batches, nb_val_samples=test_batches.N)
    lm.save_weights(modelPath + "10.pkl")
    lm.optimizer.lr = 0.00001
    lm.fit_generator(batches, batches.N, nb_epoch=1,validation_data=test_batches, nb_val_samples=test_batches.N)
    lm.save_weights(modelPath + "11.pkl")
    lm.fit_generator(batches, batches.N, nb_epoch=1,validation_data=test_batches, nb_val_samples=test_batches.N)
    lm.save_weights(modelPath + "12.pkl")
    lm.fit_generator(batches, batches.N, nb_epoch=1,validation_data=test_batches, nb_val_samples=test_batches.N)
    lm.save_weights(modelPath + "13.pkl")
    lm.fit_generator(batches, batches.N, nb_epoch=1,validation_data=test_batches, nb_val_samples=test_batches.N)
    lm.save_weights(modelPath + "14.pkl")
    lm.fit_generator(batches, batches.N, nb_epoch=1,validation_data=test_batches, nb_val_samples=test_batches.N)
    lm.save_weights(modelPath + "15.pkl")
    lm.fit_generator(batches, batches.N, nb_epoch=1,validation_data=test_batches, nb_val_samples=test_batches.N)
    lm.save_weights(modelPath + "16.pkl")
    lm.fit_generator(batches, batches.N, nb_epoch=1,validation_data=test_batches, nb_val_samples=test_batches.N)
    lm.save_weights(modelPath + "17.pkl")
    lm.fit_generator(batches, batches.N, nb_epoch=1,validation_data=test_batches, nb_val_samples=test_batches.N)
    lm.save_weights(modelPath + "18.pkl")
    lm.fit_generator(batches, batches.N, nb_epoch=1,validation_data=test_batches, nb_val_samples=test_batches.N)
    lm.save_weights(modelPath + "19.pkl")
    lm.fit_generator(batches, batches.N, nb_epoch=1,validation_data=test_batches, nb_val_samples=test_batches.N)
    lm.save_weights(modelPath + "20.pkl")


