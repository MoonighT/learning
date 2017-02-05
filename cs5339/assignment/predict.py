from record import loadTest
import numpy as np
from keras.models import Sequential
from keras.layers.core import Flatten, Dense, Dropout, Lambda
from keras.layers.convolutional import Convolution2D, MaxPooling2D, ZeroPadding2D
from keras.optimizers import SGD, Adam
from keras.preprocessing import image
from keras.layers.normalization import BatchNormalization
import classify

if __name__ == "__main__":
    (x_test, y_test, i_test) = loadTest()
    x_test= np.expand_dims(x_test, 3)
    print x_test.shape
    modelPath = "./model/"
    lm = classify.cnn_model()
    lm.load_weights(modelPath + "8.pkl")
    out = lm.predict(x_test)
    res = np.argmax(out, axis=1)

    print "Id,Prediction"
    for i in range(len(i_test)):
        print i_test[i],"," + chr(ord('a') + res[i])
