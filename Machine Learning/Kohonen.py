DIM = 2


def main():
    combinations = [(20, 'uniform'), (20, 'proportional'),
                    (200, 'uniform'), (200, 'proportional')]

    for neurons, sample_method in combinations:
        algo = Kohonen(neurons_num=neurons, dimension=DIM, sample_by=sample_method)
        data = generate_data(DIM)
        algo.load(data)
        algo.fit()
        algo.show()


def generate_data(dimensions):
    return []


class Kohonen:
    def __init__(self, neurons_num=20, dimension=2, sample_by='uniform'):
        """
        :param neurons_num: Number of neurons to be fitted to the input
        :param dimension: Feature number of each input sample
        :param sample_by: Pick a random sample while training by-
                          'uniform'      - Each sample has the same chance to be picked.
                          'proportional' - Samples are more likely to be picked based on their first feature.
                           (For two dimensional points, samples with a higher x value are more likely to be picked)
        """
        self.sample_by = sample_by
        self.dimension = dimension
        self.neurons_num = neurons_num
        self.data_ = None

    def load(self, data):
        self.data_ = data

    def fit(self):
        if self.data_ is None:
            return

    def show(self):
        pass


if __name__ == '__main__':
    main()
