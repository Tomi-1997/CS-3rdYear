import math
import random
import matplotlib.pyplot as plt

DIM = 2
DATA_SIZE = 500
LR = 0.1
ITERS = 1000
PLOT = True
ANIMATE = True ## True- Shows each plot for half a second and continues, False- Shows plot until exitted

HAND_PATH_3 = r"C:\Users\user\file\hand_pic_path"
HAND_PATH_4 = r"C:\Users\user\file\hand_pic_path"


def main():
    data_groups = {'square': (generate_data_sqr    (DIM, DATA_SIZE,  0, 1),  0, 1),
                  'circle' : (generate_data_circle (DIM, DATA_SIZE, -4, 4), -4, 4),
                  '4fhand' : (generate_4fin_hand   (     DATA_SIZE,  0, 20), 0, 20),
                  '3fhand' : (generate_3fin_hand   (     DATA_SIZE,  0, 20), 0, 20)}


    for data, min, max in data_groups.values():
        run(data, min, max)


def run(data, min, max):
    combinations = [(20, 'uniform'), (20, 'proportional'),
                    (200, 'uniform'), (200, 'proportional')]

    for neurons, sample_method in combinations:
        algo = Kohonen(neurons_num=neurons, dimension=DIM,
                       sample_by=sample_method, iters=ITERS,
                       lr=LR, min=min, max=max, plot = True)

        algo.load(data)
        algo.fit()


def generate_data_sqr(dimensions, size, min, max):
    ans = [data(dimensions, min, max) for _ in range(size)]

    ## Add some outliers
    for _ in range(size // 20):
        ans.append(data(dimensions, min - max / 5, max + max / 5))

    return ans


def generate_data_circle(dimensions, size, min, max):

    ans = []
    while len(ans) < size:
        data_point = data(dimensions, -20, 20)

        if min <= sum([i * i for i in data_point.x]) ** 0.5 <= max:
            ans.append(data_point)

    return ans


def generate_4fin_hand(size, min, max):
    im = plt.imread(HAND_PATH_4)
    ans = []
    w, h, c = im.shape

    for i in range(h):
        for j in range(w):
            if sum(im[i, j]) == 0:
                x, y = max - i + min, j ## Weird subtracting as
                ans.append(data(DIM, min, max, y=None, x= [y, x]))

    return ans


def generate_3fin_hand(size, min, max):
    im = plt.imread(HAND_PATH_3)
    ans = []
    w, h, c = im.shape

    for i in range(h):
        for j in range(w):
            if sum(im[i, j]) == 0:
                x, y = max - i + min, j ## Weird subtracting as
                ans.append(data(DIM, min, max, y=None, x= [y, x]))

    return ans


class Kohonen:
    def __init__(self, neurons_num=20, dimension=2, sample_by='uniform',
                 iters = 100, lr = 0.1, min = 0, max = 1, plot = False):
        """
        :param neurons_num: Number of neurons to be fitted to the input
        :param dimension: Feature number of each input sample
        :param sample_by: Pick a random sample while training by-
                          'uniform'      - Each sample has the same chance to be picked.
                          'proportional' - Samples are more likely to be picked based on their first feature.
                           (For two dimensional points, samples with a higher x value are more likely to be picked)
        """
        self.sample_by_ = sample_by
        self.dimension_ = dimension
        self.neurons_num_ = neurons_num
        self.data_ = None
        self.iters_ = iters
        self.lr_ = lr

        self.min_ = min
        self.max_ = max
        self.plot_ = plot

        self.neurons_ = generate_data_sqr(DIM, neurons_num, min, max)
        for i, n in enumerate(self.neurons_):
            ## Make the neurons be in a straight line at first
            pos = (i / neurons_num ) * (max - min)
            pos += min
            n.x = [pos for _ in range(dimension)]


    def load(self, data):
        self.data_ = data


    def fit(self):
        if self.data_ is None:
            print('No data loaded')
            return

        self.plot(0)
        for i in range(1, self.iters_ + 1):

            if i % (self.iters_ // 3) == 0:
                self.plot(i)
                self.lr_ = self.lr_ * 0.99

            data_point = self.select_randomly()
            closest_neuron= self.get_closest_n(target = data_point)
            self.shift_neighbors(neuron = closest_neuron, source = data_point)


    def select_randomly(self):

        weights = None ## If uniform, all elements have the same weight
                       ## Else- give weight to the first feature

        if self.sample_by_ == 'proprtional':
            weights = [i.x[0] for i in self.data_] ## [0.1 , 0.1, 0.8, 9]
            s = sum(weights)                       ## Sum all weights and divide
            weights = [i / s for i in weights]     ## [0.01, 0.01, 0.08, 0.9] -> 9 has the best chance

        ans = random.choices(self.data_, weights = weights, k = 1)
        return ans[0]


    def dist(self, a, b):
        ans = 0
        for ax, bx in zip(a.x, b.x):
            ans += (bx - ax) ** 2
        return ans ** 0.5


    def get_closest_n(self, target):
        best_ans = None
        best_diff = math.inf

        for n in self.neurons_:
            dt = self.dist(target, n)

            if dt < best_diff:
                best_diff = dt
                best_ans = n

        return best_ans


    def neigh_func(self, n1, n2):
        dt = self.dist(n1, n2)
        if dt > (self.max_ - self.min_) * 0.05: return 0
        return math.exp(-(dt * dt))


    def shift_neighbors(self, neuron, source):
        for n in self.neurons_:
            for i in range(self.dimension_):
                n.x[i] += self.lr_ * (source.x[i] - n.x[i]) * (self.neigh_func(n, neuron))


    def plot(self, iters = -1):
        if not self.plot_:       return
        if self.dimension_ != 2: return

        plt.clf()
        ## Data points in blue
        x = [i.x[0] for i in self.data_]
        y = [i.x[1] for i in self.data_]
        plt.plot(x, y, 'bo')

        ## Neurons in red
        x = [i.x[0] for i in self.neurons_]
        y = [i.x[1] for i in self.neurons_]
        plt.plot(x, y, 'ro-')

        plt.title(f"Neurons - {self.neurons_num_}, Sample by - {self.sample_by_}, Iteration - {iters}")

        if ANIMATE:
            plt.draw()
            plt.pause(0.4)

        else:
            plt.show()


class data:
    def __init__(self, dimensions, min, max, y = None, x = None):

        if x is None:
            self.x = [round(random.uniform(min, max), 3) for _ in range(dimensions)]
        else:
            self.x = x

        self.y = y

    def __repr__(self):
        return f'\nlabel-{self.y}, features-{self.x}'


if __name__ == '__main__':
    main()
