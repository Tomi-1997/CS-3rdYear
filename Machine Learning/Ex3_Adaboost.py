import math
import random
import time

import matplotlib.pyplot as pl

# CENTER = 0.5 for plotting
class Data:
    def __init__(self, x:float, y:float, lab:int):
        self.features = [x, y]
        self.label = lab

    def __repr__(self):
        return "\nx:{}, y:{}, label:{}".format(self.x, self.y, self.label)

class Line:
    def __init__(self, point1:Data, point2:Data):
        self.x1 = point1.features[0]
        self.y1 = point1.features[1]
        self.x2 = point2.features[0]
        self.y2 = point2.features[1]

    def guess(self, point:Data):
        xA = point.features[0]
        yA = point.features[1]
        v1x, v1y = self.x2 - self.x1, self.y2 - self.y1  # Vector 1
        v2x,v2y = xA - self.x1, yA - self.y1  # Vector 2
        cross_product = v1x * v2y - v1y * v2x
        return 1 if cross_product > 0 else 0

    def calc_error(self, sample_list):
        ## For each wrong guess on a point X, it adds the weight of X into a list L, and return the sum of L.
        return sum([point.weight for point in sample_list if self.guess(point) != point.label])

    def __repr__(self):
        return f'({self.x1},{self.y1} ----> {self.x2},{self.y2})'

def adaboost(samples, iters, feat_num):
    S = random.sample(samples, int(len(samples) / 2))  ## Train
    T = [s for s in samples if s not in S]  ## Test

    assert len(S) + len(T) == len(samples)

    for point in samples:
        point.weight = 1 / len(samples)

    ## Rules (lines between each pair of points in S)
    rules = []
    for i in range(len(S)):
        for j in range(i + 1, len(S)):
            rules.append(Line(point1=S[i], point2=S[j]))

    important_rules = []
    empirical_err = []
    true_err = []
    for i in range(iters):

        ## For each rule, calculate error on S
        rules_error = [(rule.calc_error(S), i) for i, rule in enumerate(rules)]

        ## Get rule with minimal error
        min_error_pair = min(rules_error)
        min_rule = rules[min_error_pair[1]]
        min_error = min_error_pair[0]

        ## Add to important rules list
        important_rules.append(min_rule)
        alpha_error = 0.5 * math.log((1 - min_error) / min_error)

        ## Update weights [Normalised]
        for point in samples:
            if point.label == min_rule.guess(point):
                point.weight = point.weight / (2 * (1 - min_error))
            else:
                point.weight = point.weight / (2 * min_error)


        k_emp_err = 0
        for point in S:
            if Hk(point, important_rules) != point.label:
                k_emp_err += (1.0 / len(samples))

        k_true_err = 0
        for point in T:
            if Hk(point, important_rules) != point.label:
                k_true_err += (1.0 / len(samples))

        empirical_err.append(k_emp_err)
        true_err.append(k_true_err)

    return empirical_err, true_err


def plot_rules(rules):
    pl.plot([rule.x1 for rule in rules], [rule.y1 for rule in rules],
                           [rule.x2 for rule in rules], [rule.y2 for rule in rules])
    pl.show()

def Hk(x:Data, rules:list):
    sum_mistakes = []
    for rule in rules:
        res = 1 if rule.guess(x) == x.label else -1
        sum_mistakes.append(x.weight * res)
    return sign(sum(sum_mistakes))

## Should also return -1 but there is only 0 and 1 for all Yi labels.
def sign(x):
    if x > 0:
        return 1
    return 0

def get_data(filepath:str):
    samples = []
    with open(filepath, "r") as fp:

        for line in fp.readlines():
            x = line[0 : line.find(" ") + 1]
            line = line.replace(x, "")
            y = line[0 : line.find(" ") + 1]
            line = line.replace(y, "")
            label = line
            samples.append( Data(float(x), float(y), int(label)) )

    return samples
if __name__ == '__main__':
    start = time.time()
    # Prepare data
    filepath = "C:\\Users\\tomto\\Desktop\\ToDo\\EX\\ML\\3\\squares.txt"
    samples = get_data(filepath=filepath)

    iter_num = 8
    emp_errors = [0 for _ in range(iter_num)]
    test_errors = [0 for _ in range(iter_num)]

    runs = 50
    for i in range(runs):
        print(i)
        result = adaboost(samples, iters=iter_num, feat_num=2)
        test_error = result[0]
        true_error = result[1]
        j = 0
        for e, t in zip(test_error, true_error):
            emp_errors[j] += e / runs
            test_errors[j] += t / runs
            j+= 1


    print(f'Empiric')
    for i, val in enumerate(emp_errors):
        print(f'Rules:{i+1}, Cost:{val}')

    print(f'True')
    for i, val in enumerate(test_errors):
        print(f'Rules:{i+1}, Cost:{val}')

    end = time.time()
    print(f'Time took: {end - start}.')
