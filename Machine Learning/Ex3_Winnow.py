class Data:
    def __init__(self, feat:list, lab:int):
        self.features = feat
        self.label = lab

    def sum_weights(self):
        return sum([feature * weight for feature, weight in zip(self.features, weights)])

def update_weights(sample, guess:int):
    for i, val in enumerate(sample.features):

        # Guess 1, but 0 was true ---> set all weights of active features to 0
        if val == 1 and guess == 1:
            weights[i] = 0

        # Guess 0, but 1 was true ---> multiply all weights of active features.
        if val == 1 and guess == 0:
            weights[i] *= 2

def get_data(x_per_line:int, filepath:str):
    samples = []
    with open(filepath, "r") as fp:

        for line in fp.readlines():
            line = line.replace(' ', '')
            features = [int(i) for i in line[:x_per_line]]
            label = int(line[x_per_line : x_per_line + 1])
            samples.append( Data(features, label) )

    return samples

if __name__ == '__main__':

    # Prepare data
    filepath = "C:\\Users\\tomto\\Desktop\\ToDo\\EX\\ML\\3\\winnow_vectors.txt"
    features_num = 20
    samples = get_data(x_per_line = features_num, filepath=filepath)
    weights = [1 for i in range(features_num)]
    print(weights)

    # Winnow
    mistakes = 0
    gussed_all_correctly = False
    while not gussed_all_correctly:

        gussed_all_correctly = True
        for sample in samples:

            guess = 1 if sample.sum_weights() >= features_num else 0
            correct = (guess == sample.label)

            if not correct:
                mistakes += 1
                gussed_all_correctly = False
                update_weights(sample, guess)
                break


    print(weights)
    print(f'Mistakes made : {mistakes}')

