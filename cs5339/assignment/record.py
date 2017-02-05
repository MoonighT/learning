import numpy as np
import matplotlib.pyplot as plt
import time

class Record:
    col = 8
    row = 16
    def __init__(self, line):
        parts = line.split(',')
        self._id = int(parts[0])
        self.predict = parts[1]
        self.nexid = int(parts[2])
        self.pos = int(parts[3])
        datas = np.array([ int(x) for x in parts[4:]])
        self.img = np.reshape(datas, (-1, self.col))

    # image is rank 1 np array
    def show(self):
        plt.imshow(self.img, extent=(0, self.col, self.row, 0), interpolation='nearest', cmap='gray_r')
        plt.show()

    def label(self):
        lab = ord(self.predict[0]) - ord('a')
        return lab

    def data(self):
        return self.img

    def id(self):
        return self._id


def constructOneRecord(line):
    r = Record(line)
    return r


def readData(path="./data/train.csv"):
    #ignore first line
    with open(path) as f:
        contents = f.readlines()
    contents = contents[1:]
    for content in contents:
        constructOneRecord(content)


def makeData(contents):
    x = []
    y = []
    i = []
    for content in contents:
        r = constructOneRecord(content)
#        r.show()
        x.append(r.data())
        y.append(r.label())
        i.append(r.id())
    return np.array(x), np.array(y), np.array(i)

def loadData(path="./data/train.csv"):
    with open(path) as f:
        contents = f.readlines()
    contents = contents[1:]
    train = contents[0: int(len(contents) * 0.9) ]
    test = contents[int(len(contents) * 0.9) :]
    return makeData(train), makeData(test)

def loadTest(path="./data/test2.csv"):
    with open(path) as f:
        contents = f.readlines()
    contents = contents[1:]
    return makeData(contents)


if __name__ == "__main__":
    (x_test, y_test, i_test) = loadTest()

