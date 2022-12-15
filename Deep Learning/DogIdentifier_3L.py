import winsound # for beep
from array import array

import time
import pandas as pd # open csv
import numpy as np
import matplotlib.pyplot as plt
import os, random # get random dog batch
import cv2

import matplotlib.image as img
from PIL import Image, ImageOps

import tensorflow.compat.v1.keras.preprocessing.image as tf_img
import tensorflow.compat.v1 as tf
tf.disable_v2_behavior()

directory = 'C:\\Users\\tomto\\PycharmProjects\\RandomStuff\\dog-breed-identification\\'
labels = pd.read_csv(directory + '\\labels.csv')
breeds = pd.unique(labels['breed'])
breeds_num = len(breeds)
train_dir = directory + '\\train\\'

DATA_NAMES = [train_dir + fname + '.jpg' for fname in labels['id']]   # All filenames
VALID_NAMES = random.sample(DATA_NAMES, int(len(DATA_NAMES) * 0.2))   # 20% of file names
TRAIN_NAMES = [fname for fname in DATA_NAMES if fname not in VALID_NAMES] # rest

# Check to see sample size is valid.
assert(len(VALID_NAMES) + len(TRAIN_NAMES) == len(DATA_NAMES)) # Validate uniform scattering of data

img_height = 200
img_width = 200

batch_size = 20
iters = 100  # Iterations before checking accuracy
epochs = 100

train_accuracy_tracker = []
train_loss_tracker = []
train_accuracy_avg_tracker = []
train_loss_avg_tracker = []

accuracy_tracker = []
loss_tracker = []
accuracy_avg_tracker = []
loss_avg_tracker = []

def init_label_vector(i:int):
    """Constructs and returns a vector of ones and zeros corresponding to the dog's breed."""
    ans = [0 for _ in range(breeds_num)]
    ans[i] = 1
    return ans

# For each breed, attach a [0,0,...,1,...] vector.
breeds_dic = {breed: init_label_vector(i) for i, breed in enumerate(breeds)}

def resize_all(w, h):
    """Resizes all pictures in the training folder. (Overwrites)"""
    print(f'Starting to resize all images to {w}, {h}')
    ids = labels.get(key="id")

    for id in ids:
        image = Image.open(train_dir + id + '.jpg')
        new_image = image.resize((w, h))
        new_image.save(train_dir + id + '.jpg')

def find_index(breed_name):
    """Finds the index of the breed in the unique breed list."""
    for i, breed in enumerate(breeds):
        if breed == breed_name:
            return i
    return -1

def get_breed(id):
    """Returns a dog's breed based on id."""
    new_id = id.replace(".jpg","")
    dog_file = pd.read_csv(directory + 'labels.csv')
    for i, dog in enumerate(dog_file.values):
        if dog[0] == new_id:
            return dog[1]

def zero_one_vec(breed:str):
    return [i for i in breeds_dic[breed]]

def trim(id):
    ans = id[id.rfind('\\') + 1:-4] # returns an image id without file path and without .jpg
    return ans

# For each image, attach to it's id (random string) a [0,...1,..] vector.
# This line may take 30 to 60 seconds at launch.
DATA_ANS_VEC = {trim(i) : zero_one_vec(breed = get_breed(trim(i))) for i in DATA_NAMES}

def get_answer_vec(id):
    """Returns a vector of ones and zeros corresponding to the dog's breed."""
    return DATA_ANS_VEC[id]

def get_dog(train):
    """Returns a tuple of ( [Grayscale image values], [0,0... 1, 0, 0] 1 for the index of the breed)"""

    # Get random image from train or validation sample.
    id = random.choice(TRAIN_NAMES) if train is True else random.choice(VALID_NAMES)

    # Get only file name out of filepath
    filename = id.replace(train_dir,"").replace(".jpg","")
    ans = []

    # load image data as grayscale, flip coin to mirror image or to keep original.
    im = Image.open(id).convert('L') if random.randint(0,1) == 1 else ImageOps.mirror(Image.open(id).convert('L'))
    px = im.load()

    # dump data between 0,1
    for i in range(img_height):
        for j in range(img_width):
            ans.append(px[i, j] / 255.0)

    return ans, get_answer_vec(filename)

def get_dogs(num, train):
    """Returns a list of many dogs and their vector of breed."""
    dogs = []
    ans = []
    for i in range(num):
        d, a = get_dog(train)
        dogs.append(d)
        ans.append(a)

    return dogs, ans

def plot(epochs, acc, loss, train_acc, train_loss):
    x_acc = [i for i in range(epochs)]
    y_acc = acc

    x_loss = [i for i in range(epochs)]
    y_loss = loss

    fig = plt.figure()
    ax1 = fig.add_subplot(1, 2, 1)
    ax2 = fig.add_subplot(1, 2, 2)

    ax1.plot(x_acc, y_acc, label='validation')
    ax1.plot(x_acc, train_acc, label='train')

    ax2.plot(x_loss, y_loss, label='validation')
    ax2.plot(x_loss, train_loss, label='train')

    ax1.set_xlabel('epochs')
    ax1.set_title('accuracy')
    ax1.legend()
    ax2.set_xlabel('epochs')
    ax2.set_title('loss')
    ax2.legend()

    plt.show()
    plt.show()

def predict(session, prediction, x_data, filepath):
    guess_data = []
    with Image.open(filepath).convert('L') as im:
        px = im.load()

    for i in range(img_height):
        for j in range(img_width):
            guess_data.append(px[i, j] / 255.0)

    output = session.run(prediction, {x_data: [guess_data]})
    output = np.argmax(output)
    print("your prediction result is:", breeds[output])

def train_model(learning_rate):
    # Unmark this only if you have the original backup folder of pictures.
    # resize_all(img_width, img_height)

    start = time.time()
    hidden_layer1_size = breeds_num * 5

    # Defining features (pixels) and labels(dog breed).
    pixel_num = img_width * img_height  # Not multiplied by three as images are greyscaled
    pixels = tf.placeholder(tf.float32, [None, pixel_num], name="pixels")
    answers = tf.placeholder(tf.float32, [None, breeds_num], name="answers")

    # Defining layers
    weights1 = tf.Variable(tf.truncated_normal([pixel_num, hidden_layer1_size], stddev=0.1), name="weights1")
    bias1 = tf.Variable(tf.constant(0.1, shape=[hidden_layer1_size]), name="bias1")
    prediction1 = tf.nn.relu(tf.matmul(pixels, weights1) + bias1, name="prediction1")

    weights2 = tf.Variable(tf.truncated_normal([hidden_layer1_size, breeds_num], stddev=0.1), name="weights2")
    bias2 = tf.Variable(tf.constant(0.1, shape=[breeds_num]), name="bias2")

    keep_prob = tf.placeholder(tf.float32) # for dropout
    drop_out = tf.nn.dropout(prediction1, keep_prob)

    prediction2 = tf.nn.relu(tf.matmul(drop_out, weights2) + bias2, name="prediction2")

    # Loss function
    loss = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(logits=prediction2, labels=answers), name="loss")
    train_step = tf.train.GradientDescentOptimizer(learning_rate).minimize(loss)

    correct_prediction = tf.equal(tf.argmax(prediction2, 1), tf.argmax(answers, 1))
    accuracy = tf.reduce_mean(tf.cast(correct_prediction, tf.float32))

    saver = tf.train.Saver()
    init = tf.global_variables_initializer()

    session = tf.Session()
    session.run(init)
    max_acc = 0

    print(f'Starting.')
    for i in range(epochs):

        for _ in range(iters):  # Train
            batch_xs, batch_ys = get_dogs(batch_size, train=True)
            train_acc, train_ls, _ = session.run([accuracy, loss, train_step],
                                                 feed_dict={pixels: batch_xs, answers: batch_ys, keep_prob : 0.5})

            train_accuracy_tracker.append(float(train_acc))
            train_loss_tracker.append(train_ls)

        train_avg_acc = sum(train_accuracy_tracker) / len(train_accuracy_tracker)
        train_avg_loss = sum(train_loss_tracker) / len(train_loss_tracker)

        train_accuracy_avg_tracker.append(float(train_avg_acc))
        train_loss_avg_tracker.append(train_avg_loss)

        batch_xs, batch_ys = get_dogs(batch_size, train=False)  # Validate
        acc, ls = session.run([accuracy, loss], feed_dict={pixels: batch_xs, answers: batch_ys, keep_prob : 1.0})

        # Add loss and accuracy to plot later
        accuracy_tracker.append(float(acc))
        loss_tracker.append(ls)

        avg_acc = sum(accuracy_tracker) / len(accuracy_tracker)
        avg_loss = sum(loss_tracker) / len(loss_tracker)
        print(f'({i}), After {int(time.time() - start)}s, accuracy avg {avg_acc:.0%}, loss avg {avg_loss}')

        loss_avg_tracker.append(avg_loss)
        accuracy_avg_tracker.append(avg_acc)

        # print(session.run(weights[-1], feed_dict={pixels: batch_xs, answers: batch_ys}))
        # print(f'After {int(time.time() - start)}s, accuracy is {acc:.0%}, loss is {ls}')
        # if (acc > max_acc):
        #     max_acc = acc
        #     # saver.save(session, 'my_test_model')
        #     print(f'After {int(time.time() - start)}s model saved with accuracy {acc:.0%}')

    winsound.Beep(440, 500)

    try:
        predict(session, prediction2, pixels, directory + "\\bonito\\1.jpg")
        predict(session, prediction2, pixels, directory + "\\bonito\\2.jpg")
        predict(session, prediction2, pixels, directory + "\\bonito\\3.jpg")
    except:
        pass

    plot(epochs, accuracy_avg_tracker, loss_avg_tracker, train_accuracy_avg_tracker, train_loss_avg_tracker)
    return prediction2


if __name__ == '__main__':
    model = train_model(learning_rate=0.01)
