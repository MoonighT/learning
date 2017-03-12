from record import loadTest
from record import loadDataSentence
from record import loadTestSentence
import numpy as np
from keras.models import Sequential
from keras.layers.core import Flatten, Dense, Dropout, Lambda
from keras.layers.convolutional import Convolution2D, MaxPooling2D, ZeroPadding2D
from keras.optimizers import SGD, Adam
from keras.preprocessing import image
from keras.layers.normalization import BatchNormalization
import classify


def buildlanguegemodel():
    dictionary = {}
    sentences = loadDataSentence()
    for s in sentences:
        word = ""
        for n in s:
            word += chr(ord('a') + n)
        dictionary[word] = True
    return dictionary 

def gettestsep():
    sep = loadTestSentence()
    sepmap = {}
    for i in sep:
        sepmap[i] = True
    return sepmap

if __name__ == "__main__":
    mydict = buildlanguegemodel()
    sep = gettestsep()
    #print sep
    (x_test, y_test, i_test) = loadTest('./data/test.csv')
    x_test= np.expand_dims(x_test, 3)
    #x_test = x_test[0:100]
    #print x_test.shape
    modelPath = "./model4/"
    lm = classify.cnn_model()
    lm.load_weights(modelPath + "16.pkl")
    out = lm.predict(x_test)
    res = np.argmax(out, axis=1)

    indice = out.argsort()
    indice = np.array([ n[::-1][:2] for n in indice])
    words = []
    word = []
    for i, o in enumerate(out):
        pro = out[i][indice[i]]    
        if pro[0] / pro[1] > 100:
            pro[1] = pro[0]
            indice[i][1] = indice[i][0]
        word.append(indice[i])
        if i in sep:
            words.append(word)
            word = []
    #print words
    results = []
    for w in words:
        keyword = "" 
        for c in w:
            keyword += chr(ord('a') + c[0])
        if keyword in mydict:
            results.append(keyword)
        else:
            #print "not in dict", keyword
            wordcollection = []
            wordcollection.append(keyword)
            stop = False
            for i, c in enumerate(w):
                if c[1] != c[0]:
                    newcollection = []
                    for target in wordcollection:
                        newkeyword = target[0:i] + chr(ord('a') + c[1]) + target[i+1:]
                        if newkeyword in mydict:
                            keyword = newkeyword
                            #print "find keyword", keyword
                            stop = True
                            break;
                        else:
                            newcollection.append(newkeyword)
                            newcollection.append(target)
                    wordcollection = newcollection
                    if stop == True:
                        break
            #print "change to ", keyword
            results.append(keyword)
    
    charresult = []
    for r in results:
        for c in r:
            charresult.append(c)
    
    if len(i_test) != len(charresult):
        print "error"
    #print results
    print "Id,Prediction"
    for i in range(len(i_test)):
        print i_test[i],"," + charresult[i]
