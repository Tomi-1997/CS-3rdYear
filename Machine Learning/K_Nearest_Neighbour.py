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


## Try to generalise as much as possible,
# by changing variables below this could work on the float datatype given in the previous ex.
SEPARATOR = ","

LABELS = [1, 2]
LABEL_TYPE = int

FEATURES_NUM = 3
DATA_TYPE = int

def d(p1:Data, p2:Data, Lp):
    """Returns distance between two points, based on a given Lp. (1, 2 or inf)"""
    ans = 0
    if Lp == 1: ## Sum of X1 - Y1, X2 - Y2, ... , Xn - Yn.
        for feat1, feat2 in zip(p1.features, p2.features):
           ans += math.fabs(feat1 - feat2)
    if Lp == 2: ## Square root of the following sum (X1 - Y1) squared, (X2 - Y2) squared, ... , (Xn - Yn) squared.
        for feat1, feat2 in zip(p1.features, p2.features):
            ans += (feat1 - feat2) * (feat1 - feat2)
        ans = math.sqrt(ans)
    if Lp == math.inf: ## Take the co-ordinate with the largest difference between p1 and p2.
                       ## d( [1 , 4 , 7], [1, 1, 6] ) = max(1-1,4-1,7-6) = max(0,3,1) = 3
        for feat1, feat2 in zip(p1.features, p2.features):
            dist = math.fabs(feat1 - feat2)
            if (dist > ans):
                ans = dist

    return ans

def nearest_neigh(s1:Data, T:list, curr_neigh:list, Lp):
    """Returns the nearest neighbours based on Lp as a distance function"""
    ans = None
    dist = math.inf

    # Get a new neighbour with minimal distance based on Lp (1,2 or inf).
    for s2 in T:
        current_d = d(s1, s2, Lp)
        if (current_d < dist and s2 not in curr_neigh and s2 != s1):
            dist = current_d
            ans = s2

    return ans

def get_guess(samp:Data, k:int, T:list, Lp):
    """For input of a data sample do:
        1- Construct list with k-nearest neighbours and their label
        2- Return label with most repeats"""
    samp_neigh = []
    label_counter = {i : 0 for i in LABELS}

    while len(samp_neigh) < k:
        nearest = nearest_neigh(samp, T, samp_neigh, Lp)
        samp_neigh.append(nearest)
        label_counter[nearest.label] = label_counter.get(nearest.label) + 1

    return max(label_counter, key=label_counter.get)

def nearest_neighbours(k, Lp, samples):
    empirical_error = 0
    true_error = 0

    # Sample a random group for training, and then test on the other group.
    T = random.sample(samples, int(len(samples) / 2))
    S = [samp for samp in samples if samp not in T]
    assert len(T) + len(S) == len(samples)
    assert not (T in S or S in T)

    for samp in T:
        guess = get_guess(samp, k, T, Lp)
        if (guess != samp.label): empirical_error += 1
    for samp in S:
        guess = get_guess(samp, k, T, Lp)
        if (guess != samp.label): true_error += 1

    return empirical_error, true_error

def get_data(x_per_line:int, filepath:str, separator:str):
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

if __name__ == '__main__':
    # Prepare data
    filepath = "C:\\Users\\tomto\\Desktop\\tomi\\3rdyear\\Winter\\EX\\ML\\4\\haberman.data"
    samples = get_data(x_per_line=FEATURES_NUM, filepath=filepath, separator=SEPARATOR)
    runs = 100

    # Run algo
    start = time.time()

    k_ = [1, 3, 5, 7, 9]
    Lp_ = [1, 2, math.inf]

    all_errors = [0 for i in range(len(k_) * len(Lp_) * 3)] # Times 3 for empiric error, true error and their difference.
    all_errors_detail = [] # Filled below

    # init lists for storing error data
    for k in k_:
        for Lp in Lp_:
            space = "  " if Lp != math.inf else ""
            all_errors_detail.append("\nk=" + str(k) + ", lp=" + str(Lp) + space + ", E=")
            all_errors_detail.append("k=" + str(k) + ", lp=" + str(Lp) + space + ", T=")
            all_errors_detail.append("k=" + str(k) + ", lp=" + str(Lp) + space + ", D=")

    # start running k-nearest algo with varying k and distance function
    for _ in range(runs):
        stage = 0
        for k in k_:
            for Lp in Lp_:
                empiric_err, true_err = nearest_neighbours(k, Lp, samples)
                ## Add training error dividied by how many runs there are
                all_errors[stage] += empiric_err / runs
                stage += 1
                ## Add true error divided by how many runs there are
                all_errors[stage] += true_err / runs
                stage += 1
                ## Take latest errors and mark the difference.
                all_errors[stage] = math.fabs((all_errors[stage - 1] - all_errors[stage - 2]))
                stage += 1

    for detail, value in zip(all_errors_detail,all_errors):
        print(f'{detail}{value}')

    end = time.time()
    print(f'Runtime of {int((end - start) / 60)} minutes.')

    #  Testing distance function
    # a = Data(features=[3,1],label=1)
    # b = Data(features=[6,3], label=1)
    # print(d(a,b, 2))
    # for i in range(3):
    #     a = random.randint(0, len(samples))
    #     b = random.randint(0, len(samples))
    #     print(samples[a])
    #     print(samples[b])
    #     print(d(samples[a], samples[b], 1))
    #     print(d(samples[a], samples[b], 2))
    #     print(d(samples[a], samples[b], math.inf))
