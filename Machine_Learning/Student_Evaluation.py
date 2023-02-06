# Full name : Higher Education Students Performance Evaluation Dataset Data Set.
# https://archive.ics.uci.edu/ml/datasets/Higher+Education+Students+Performance+Evaluation+Dataset

from sklearn.neighbors import KNeighborsClassifier
from sklearn import svm

import math
import random
import time
"""
A Single sample of data is:
+ List of features. (Integers, floats)
+ Label. (0 or 1, usually)
"""
class Data:
    def __init__(self, label: int, features:list):
        self.features = features
        self.label = label

    def __repr__(self):
        repr = "[L:( " + str(self.label) + " ) X's:( "
        for f in self.features:
            repr += str(f) + " "
        return "\n" + repr + ")]"


LABELS = [0, 1, 2, 3, 4, 5, 6, 7]
LABEL_TYPE = int

FEATURES_NUM = 32
DATA_TYPE = int

PATH = "C:\\Users\\tomto\\Desktop\\ML_PROJECT\\"
FILENAME = "data.txt"
SEPARATOR = ";"

def get_data(x_per_line:int, filepath:str, separator:str):
    """Returns a list of data elements from a given filepath."""
    samples = []
    with open(filepath, "r") as fp:
        for line in fp.readlines():
            features = []
            line = line.replace("\n", "").split(separator)
            for i, val in enumerate(line):
                if (i == len(line) - 1): ## Last element is label, else is a feature
                    label = LABEL_TYPE(val)
                else:
                    features.append(DATA_TYPE(val))
            samples.append( Data(features= features, label= label) )
    return samples

def split(data, train_percent):
    """Returns two independent lists from a given list."""
    train = random.sample(samples, int(len(samples) * train_percent))
    test = [i for i in samples if i not in train]

    return train, test

def k_neighbours(num_neighbours, samples, runs):
    error = 0
    for _ in range(runs):
        TRAIN, TEST = split(samples, train_percent = 0.7)

        X = [samp.features for samp in TRAIN]
        y = [samp.label for samp in TRAIN]
        neigh = KNeighborsClassifier(n_neighbors=num_neighbours)
        neigh.fit(X, y)

        predictions = neigh.predict([elem.features for elem in TEST])
        for pred, actual in zip(predictions, TEST):
            error += math.fabs(actual.label - pred) / runs

    return error

def k_neighbours_get_best_k(samples):
    """Attempts to extract the best number of neighbours to look when deciding which label a new point will be given."""
    start = 1
    end = int(len(samples)/4)
    jump = 2
    runs = 1000
    k_error = {k : 0 for k in [i for i in range(start, end, jump)]}

    for _ in range(runs):
        for k in range(start, end, jump):
            res = k_neighbours(num_neighbours=k, samples=samples)
            k_error[k] += res / runs

    min_err = min(k_error, key=k_error.get)
    return min_err, k_error[min_err]

def run_svm(samples, runs):
    error = 0
    for _ in range(runs):
        TRAIN, TEST = split(samples, train_percent = 0.7)

        X = [samp.features for samp in TRAIN]
        y = [samp.label for samp in TRAIN]
        clf = svm.LinearSVC(max_iter=16000)
        clf.fit(X, y)

        predictions = clf.predict([elem.features for elem in TEST])
        for pred, actual in zip(predictions, TEST):
            error += math.fabs(actual.label - pred) / runs

    return error

def compare_res(pred, actual):
    for p, a in zip(pred, actual):
        print(f'predict:{p},real:{a.label}')

if __name__ == '__main__':
    decor = '---###---' # Organise printing
    samples = get_data(x_per_line=FEATURES_NUM, filepath=PATH + "data_proccessed.txt", separator=SEPARATOR)
    rnd_smp = random.sample(samples, 1)[0]
    print(f'{decor}\nExample feature of a sample:\n{rnd_smp.features}')
    print(f'Examlpe label of a sample:\n{rnd_smp.label}\n{decor}')

    # print(k_neighbours_get_best_k(samples=samples)) # After 1000 runs, k = 7, error is around 75.
    runs = 1
    print(f'Starting SVM.')
    print(run_svm(samples, runs=runs))
    print(f'Starting KNeighbours.')
    print(k_neighbours(num_neighbours=7, samples=samples, runs=runs))

