# Data is a list of students' answers for 31 questions about:
# - Learning habits
# - Family and personal questions
# https://archive.ics.uci.edu/ml/datasets/Higher+Education+Students+Performance+Evaluation+Dataset

# Algorithms used
# - Svm, Percpetron, Naive bayes, K-Neighbours, Adaboost, Decision Trees.
# https://scikit-learn.org/stable/user_guide.html
import sklearn.ensemble
from sklearn.neighbors import KNeighborsClassifier
from sklearn import svm, tree
from sklearn.linear_model import Perceptron
from sklearn.naive_bayes import GaussianNB
from sklearn.ensemble import AdaBoostClassifier
from sklearn.metrics import confusion_matrix, ConfusionMatrixDisplay

import matplotlib.pyplot as plt
import math, random, time, copy, graphviz
"""
A Single sample of data is:
+ List of features. (Integers, floats)
+ Label.
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

PATH = "C:\\Users\\tomto\\Desktop\\ML_PROJECT\\"
FILENAME = "data.txt"
SEPARATOR = ";"

LABELS = [0, 1, 2, 3, 4, 5, 6, 7]
LABEL_TYPE = int
LABEL_DESC = ["Fail", "DD", "DC", "CC", "CB", "BB", "BA","AA"]

FEATURES_NUM = 31
DATA_TYPE = int
FEATURES_DESC = [] # Will be read from text file.

"""Read from text file description for each feature."""
with open(PATH + "feature_desc.txt", "r") as fp:
    for line in fp.readlines():
        FEATURES_DESC.append(line.removesuffix("\n"))

"""Percent of overall samples to train on, before testing on the other group."""
TRAIN_PER = 0.5

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

def split_test_train(data, train_percent):
    """Returns two independent lists from a given list."""
    train = random.sample(data, int(len(data) * train_percent))
    test = [i for i in data if i not in train]

    return train, test

def k_neighbours_get_best_k(samples):
    """Attempts to extract the best number of neighbours to look when deciding which label a new point will be given,
    by running 1000 times k-neighbours each time with k = 1, 3, 5, 7, ..., up to a quarter of the data size"""
    start = 1
    end = int(len(samples)/4)
    jump = 2
    runs = 1000
    k_error = {k : 0 for k in [i for i in range(start, end, jump)]}

    for k in range(start, end, jump):
        res, _ = run_clf(KNeighborsClassifier(n_neighbors=k), samples, runs)
        k_error[k] += res

    min_err = min(k_error, key=k_error.get)
    return min_err, k_error[min_err]

def plot_tree(tr):
    """Given a decision tree classifier, plots the tree."""
    tree.plot_tree(tr, max_depth=3, class_names= LABEL_DESC,
                   feature_names=FEATURES_DESC, fontsize=7,
                   filled=True, rounded=True, label='root')
    plt.show()

def confusion_matrix_clf(clf, samples):
    """Plots the confusion matrix a classifier predicted on untrained data"""
    train, test = split_test_train(samples, TRAIN_PER)
    X = [i.features for i in train]
    y = [i.label for i in train]
    clf.fit(X, y)

    predictions = clf.predict([i.features for i in test])
    real = [i.label for i in test]

    cm = confusion_matrix(real, predictions)
    disp = ConfusionMatrixDisplay(confusion_matrix=cm, display_labels=clf.classes_)
    disp.plot()
    plt.title(clf.__repr__())
    plt.show()

def run_clf(clf, samples, runs):
    """Runs the given classifier on the data and returns the average error."""
    error = 0
    acc = 0
    for _ in range(runs):
        TRAIN, TEST = split_test_train(samples, train_percent = TRAIN_PER)

        X = [samp.features for samp in TRAIN]
        y = [samp.label for samp in TRAIN]

        clf.fit(X, y) # Train the classifier on one group and count error on the other.

        predictions = clf.predict([elem.features for elem in TEST])

        error_forgive = 1
        acc += sum([1 for guess, real in zip(predictions, [elem.label for elem in TEST])
                    if math.fabs(guess - real) < error_forgive])
        for pred, actual in zip(predictions, TEST):
            error += ((actual.label - pred) * (actual.label - pred)) / runs ## Squared
            # error += math.fabs(actual.label - pred) / runs              ## 1 dim distance

    return error, int(acc / runs)

def find_important_rules(samples, runs):
    """Plots an error graph for each feature of the data set, while highlighting the minimal and maximal error."""
    removed_feat_avg_error = []
    for i in range(FEATURES_NUM):

        ## Deep copy samples while removing only one feature each time
        temp_samples = copy.deepcopy(samples)

        for elem in temp_samples:
            elem.features[i] = -1

        avg_error = 0
        # Perform each classifier in the modified data
        clfs =  [KNeighborsClassifier(n_neighbors=7),
                AdaBoostClassifier(),
                tree.DecisionTreeClassifier()]

        for clf in clfs:
            res_error, _ = run_clf(clf, samples=temp_samples, runs=runs)
            avg_error += res_error / len(clfs)
        removed_feat_avg_error.append(avg_error)

    plt.plot([i for i in range(FEATURES_NUM)], removed_feat_avg_error, 'bs',
             [i for i in range(FEATURES_NUM)], removed_feat_avg_error, 'k')
    plt.axis([0, FEATURES_NUM, min(removed_feat_avg_error) / 2, max(removed_feat_avg_error) * 2])
    plt.title("Error after removing a the x feature (Performing tests with 30 features).")


    # Highlight min error
    min_val = min(removed_feat_avg_error)
    min_ind = removed_feat_avg_error.index(min_val)
    plt.annotate(FEATURES_DESC[min_ind], xy=(min_ind, min_val), xytext=(1, min_val * 0.6),
                 arrowprops=dict(facecolor='black', shrink=0.05),)

    # Highlight max error
    max_val = max(removed_feat_avg_error)
    max_ind = removed_feat_avg_error.index(max_val)
    plt.annotate(FEATURES_DESC[max_ind], xy=(max_ind, max_val), xytext=(1, max_val * 1.2),
                 arrowprops=dict(facecolor='black', shrink=0.05),)

    err_desc = [(val, FEATURES_DESC[i]) for i, val in enumerate(removed_feat_avg_error)]
    err_desc.sort(key=lambda k : k[0])

    plt.ylabel("Error")
    plt.show()

def predict_unkown(samples):
    """Given a new sample, with no course id (30 features), predicts outcome."""
    runs = 100

    ## Sample based on my actual habits.
    me = [2, 2, 2, 4, 2, 1, 1, 1, 1, 3, 4, 4, 2, 2, 4, 3, 3, 3, 1, 1, 1, 2, 2, 1, 1, 3, 3, 3, 3, 3]
    predicts = [0 for i in range(len(LABELS))]

    for i in range(runs):

        X = [samp.features[:-1] for samp in samples] # Excluding course id.
        y = [samp.label for samp in samples]

        for clf in [AdaBoostClassifier(), tree.DecisionTreeClassifier()]:
            clf.fit(X, y)

            predict = clf.predict([me])[0]
            predicts[predict] += 1

    result = predicts.index(max(predicts))
    print(f'Most gussed grade is {result}')

def v2_binarify_data(samples, divide_point):
    """Turns data labels into 0 and 1. Based on a given input, each sample with label above that will turn to 1"""
    for smp in samples:
        lbl = smp.label
        smp.label = 1 if lbl > divide_point else 0

    LABELS = [0, 1]
    LABEL_DESC = ["BAD", "GOOD"]
    print(f'Binarified data.')

def error_accuracy_of(samples, classifiers, runs):
    for clf in classifiers:
        clf_name = clf.__repr__()
        print(f'{clf_name[:clf_name.index("(")]}')
        error, acc = run_clf(clf, samples=samples, runs=runs)
        print(f'error- {error}, accuracy- {acc} / {int(len(samples) * (1 - TRAIN_PER) + 0.5)}')

if __name__ == '__main__':
    decor = '---###---' # Organise printing
    samples = get_data(x_per_line=FEATURES_NUM, filepath=PATH + "data_proccessed.txt", separator=SEPARATOR)
    rnd_smp = random.sample(samples, 1)[0]
    print(f'{decor}\nExample feature of a sample:\n{rnd_smp.features}')
    print(f'Examlpe label of a sample:\n{rnd_smp.label}\n{decor}')

    runs = 100

    ada = AdaBoostClassifier()
    tr = tree.DecisionTreeClassifier()
    sv = svm.NuSVC(nu=0.01)
    classifiers = [GaussianNB(),
                   Perceptron(),
                   KNeighborsClassifier(n_neighbors=7),
                   ada,
                   tr,
                   ## -- New additions -- ##
                   sv,
                   sklearn.ensemble.VotingClassifier
                   (estimators=[('Adaboost', ada), ('Tree', tr), ('Svm', sv)], voting='hard')]

    """Get average error and accuracy for each classifier"""
    # error_accuracy_of(samples, classifiers, runs)

    """Get confusion matrix for each classifier"""
    # for clf in classifiers:
    #     confusion_matrix_clf(clf, samples = samples)

    """Find best neighbour size"""
    # print(k_neighbours_get_best_k(samples))

    """Analyse features"""
    # find_important_rules(samples=samples, runs=runs)

    """Binairifed data"""
    ### ------------------------------ New additions since presentation ------------------------------ ###
    # v2_binarify_data(samples=samples, divide_point=3)
    # error_accuracy_of(samples, classifiers, runs)
    #
    # for clf in classifiers:
    #     confusion_matrix_clf(clf, samples = samples)


