import math
import os
import random
import time
import numpy as np
import matplotlib.pyplot as plt

"""
A Single sample of data is:
♫ A list \ vector of features. (Integers, floats)
♫ A label. (Integer, usually)
"""
class Data:
    def __init__(self, label: int, features:list):
        self.features = features
        self.x = np.array(features)
        self.label = label

    def __repr__(self):
        repr = "[L:( " + str(self.label) + " ) X's:( "
        for f in self.features:
            repr += str(f) + " "
        return "\n" + repr + ")]"


## Constants
SEPARATOR = ","

bet = 1
mem = 2
lamed = 3
LABELS = [bet, mem, lamed]
LABELS_NAME = ["-", "bet", "lamed", "mem"]
LABEL_TYPE = int

FEATURES_NUM = 100
DATA_TYPE = int

DATA_PATH = "C:\\Users\\tomto\\Desktop\\NC\\data\\"

## From text to list of data
CHAR_IGNORE = ['(', ')', '[', ']', '\n']
def load_data(x_per_line : int, filepath : str, separator : str, samples : list):

    for fname in os.listdir(filepath):                     ## For each file in directory

        with open(filepath + fname, "r") as fp:            ## For each sample in file

            for line in fp.readlines():                    ## Each line is a sample
                features = []

                for bad_char in CHAR_IGNORE:
                    line = line.replace(bad_char, "")

                line = line.split(separator)

                if len(line) != FEATURES_NUM + 1: continue ## Ignore damaged \ bad lines

                for i, val in enumerate(line):

                    if (i == 0):
                        label = LABEL_TYPE(val)            ## Assuming the first character is the label
                    else:
                        features.append(DATA_TYPE(val))
                samples.append( Data(features= features, label= label) )

    return samples


def sigmoid(x):
    z = 500
    cp = np.clip( x, -z, z )  ## Prevent sigmoid overflow
    return 1 / (1 + np.exp( -cp ))


def backprop(sample, w, b, lr):

    dz = sigmoid(sample.x) * (1 - sigmoid(sample.x))
    dw = sample.x * dz
    db = dz

    w = w - lr * dw
    b = b - lr * db


def pred(sample, w, b):
    f = sample.x @ w + b
    g = sigmoid(f)
    y = A if g.sum() > 0.5 else B  ## Predict A or B
    return y


def fit(train, test, w):
    """Runs adaline on a given train and return number of correct predictions on given test set."""

    lr = 0.5
    eps = 0.1
    converged = False
    prev_loss = 0
    iters = 0
    b = np.zeros_like(w)

    while not converged:

        loss = 0
        iters += 1
        for t in train:
            y = pred(t, w, b)
            w = w + lr * (t.label - y) * t.x    ## Update weights
            loss += (t.label - y) ** 2          ## If the prediction is correct,
                                                ## the loss won't increase and W remains the same.

        backprop(t, w, b, lr)


        if abs(loss - prev_loss) < eps:  ## If our loss hasn't changed by an epsilon, we assume we have converged.
            converged = True

        prev_loss = loss


    predictions = []
    for s in test:
        y = pred(s, w, b)

        pred_correct = 1 if y == s.label else 0
        predictions.append(pred_correct)

    return sum(predictions) / len(predictions), iters


def avg(l : list):
    return sum(l) / len(l)


def run(curr_samples):
    w = np.random.rand((FEATURES_NUM))
    runs = 5
    acc = []
    conv = []


    size = len(curr_samples)
    for i in range(0, size, size // runs): ## Each time take a fifth, or a quarter of the data to be the test.

        test = curr_samples[i: i + size // runs]
        train = [i for i in curr_samples if i not in test]

        acc_, conv_ = fit(train, test, w)  ## Document average test accuracy and iterations until convergance.
        acc.append(acc_)
        conv.append(conv_)


    avg_acc = avg(acc)
    acc_rate = avg_acc * 100
    acc_rate = round(acc_rate, 3)

    avg_conv = avg(conv)
    avg_conv = round(avg_conv, 3)
    print(f'Average test success rate - {acc_rate}%, Average iterations until convergance - {avg_conv}')


def setup(samples):
    w = 10
    h = 10
    fig = plt.figure(figsize=(8, 8))
    columns = 4
    rows = 5

    ax = []
    for i in range(1, columns * rows + 1):

        index = random.randint(0, len(samples))

        samp = samples[index]
        img = samp.x
        lab = LABELS_NAME[samp.label]
        img = img.reshape(w, h)
        ax.append( fig.add_subplot(rows, columns, i) )
        ax[-1].set_title(lab)
        plt.imshow(img)

    plt.show()


if __name__ == '__main__':

    # Prepare data
    original_datapath = DATA_PATH + "regular\\"
    samples = []
    samples = load_data(x_per_line = FEATURES_NUM, filepath = original_datapath, separator = SEPARATOR, samples = samples)

    decor = '▬' * 10
    setup(samples)
    # exit(0)

    print(f'\n{decor} Adaline - regular data only {decor}\n')
    for s in samples:
        assert len(s.x) == FEATURES_NUM
        assert s.label in LABELS

    ### 1st, Beit vs Mem. Run adaline with a copy of only relavent samples.
    A = mem
    B = bet
    print(f'Running mem versus beit.')
    curr_samples = [i for i in samples if i.label == A or i.label == B]
    run(curr_samples)


    ## 2nd, Lamed vs brit.
    A = lamed
    B = bet
    print(f'Running lamed versus beit.')
    curr_samples = [i for i in samples if i.label == A or i.label == B]
    run(curr_samples)


    ## 3rd, Lamed vs mem.
    A = lamed
    B = mem
    print(f'Running lamed versus mem.')
    curr_samples = [i for i in samples if i.label == A or i.label == B]
    run(curr_samples)

    print(f'\n{decor} Adaline - regular and rotated data {decor}\n')
    rotated_datapath = DATA_PATH + "rotated\\"
    samples = load_data(x_per_line=FEATURES_NUM, filepath=rotated_datapath, separator=SEPARATOR, samples=samples)

    ### 1st, Beit vs Mem. Run adaline with a copy of only relavent samples.
    A = mem
    B = bet
    print(f'Running mem versus beit.')
    curr_samples = [i for i in samples if i.label == A or i.label == B]
    run(curr_samples)

    ## 2nd, Lamed vs brit.
    A = lamed
    B = bet
    print(f'Running lamed versus beit.')
    curr_samples = [i for i in samples if i.label == A or i.label == B]
    run(curr_samples)

    ## 3rd, Lamed vs mem.
    A = lamed
    B = mem
    print(f'Running lamed versus mem.')
    curr_samples = [i for i in samples if i.label == A or i.label == B]
    run(curr_samples)