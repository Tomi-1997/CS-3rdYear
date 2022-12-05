# Results so far [on training data, not validation] on learning rate of 0.1, 5 iterations before testing accuracy and loss
# After 62516s, accuracy is 97%, loss is 0.31866762042045593
# After 46119s, accuracy is 90%, loss is 0.5829817652702332
# After 27661s, accuracy is 74%, loss is 1.3114298582077026
# After 12161s, accuracy is 50%, loss is 2.4888787269592285
# After 4962s, accuracy is 23%, loss is 3.5788869857788086
# After 1100s, accuracy is 11%, loss is 4.441249370574951


import winsound # for beep
from array import array

import time
import pandas as pd # open csv
import numpy as np
import matplotlib.pyplot as plt
import os, random # get random dog batch
import cv2

import matplotlib.image as img
from PIL import Image

import tensorflow.compat.v1.keras.preprocessing.image as tf_img
import tensorflow.compat.v1 as tf
tf.disable_v2_behavior()

directory = 'C:\\Users\\tomto\\PycharmProjects\\RandomStuff\\dog-breed-identification\\'
labels = pd.read_csv(directory + '\\labels.csv')
breeds = pd.unique(labels['breed'])
breeds_num = len(breeds)
train_dir = directory + '\\train\\'
train_dir_organised = directory + '\\train_organised\\'
test_dir = directory + '\\test\\'


batch_size = breeds_num + 1
learning_rate = 0.1
img_height = 200
img_width = 200

def init_label_vector(i:int):
    """Constructs and returns a vector of ones and zeros corresponding to the dog's breed."""
    ans = [0 for _ in range(breeds_num)]
    ans[i] = 1
    return ans

breeds_dic = {breed: init_label_vector(i) for i, breed in enumerate(breeds)}

def resize_all(w, h):
    """Resizes all pictures in the training folder. (Overwrites)"""
    print(f'Starting to resize all images to {w}, {h}')
    ids = labels.get(key="id")

    for id in ids:
        image = Image.open(train_dir + id + '.jpg')
        new_image = image.resize((w, h))
        new_image.save(train_dir + id + '.jpg')

def get_dog(id = None):
    """Returns a tuple of ( [Grayscale image values], [0,0... 1, 0, 0] 1 for the index of the breed)"""
    if id is None:
        id = random.choice(os.listdir(train_dir))
        dir = train_dir_organised + id.replace(".jpg","")
    else:
        dir = train_dir_organised + id

    # img_ans = pickle.load(open(dir, "rb"))

    ans = []
    with Image.open(train_dir + id).convert('L') as im:
        px = im.load()

    for i in range(img_height):
        for j in range(img_width):
            ans.append(px[i, j] / 255.0)

    return ans, get_answer_vec(id)

def get_answer_vec(id):
    """Returns a vector of ones and zeros corresponding to the dog's breed."""
    breed = get_breed(id)
    return breeds_dic[breed]

def get_dogs(num):
    """Returns a list of many dogs and their vector of breed."""
    dogs = []
    ans = []
    for i in range(num):
        d, a = get_dog()
        dogs.append(d)
        ans.append(a)

    return dogs, ans

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

if __name__ == '__main__':


    # Unmark this only if you have the original backup folder of pictures.
    # resize_all(img_width, img_height)

    start = time.time()
    hidden_layer1_size = breeds_num * 5

    # Defining features (pixels) and labels(dog breed).
    pixel_num = img_width * img_height # Not multiplied by three as images are greyscaled
    pixels = tf.placeholder(tf.float32,[None, pixel_num])
    answers = tf.placeholder(tf.float32, [None, breeds_num])

    # Defining layers
    weights1 = tf.Variable(tf.truncated_normal([pixel_num, hidden_layer1_size], stddev= 0.1))
    bias1 = tf.Variable(tf.constant(0.1, shape = [hidden_layer1_size]))
    prediction1 = tf.nn.relu(tf.matmul(pixels, weights1) + bias1)

    weights2 = tf.Variable(tf.truncated_normal([hidden_layer1_size, breeds_num], stddev= 0.1))
    bias2 = tf.Variable(tf.constant(0.1, shape = [breeds_num]))
    prediction2 = tf.nn.relu(tf.matmul(prediction1, weights2) + bias2)

    # Loss function
    loss = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(logits=prediction2, labels=answers))
    train_step = tf.train.GradientDescentOptimizer(learning_rate).minimize(loss)

    correct_prediction = tf.equal(tf.argmax(prediction2, 1), tf.argmax(answers, 1))
    accuracy = tf.reduce_mean(tf.cast(correct_prediction, tf.float32))

    saver = tf.train.Saver()
    init = tf.global_variables_initializer()

    session = tf.Session()
    session.run(init)
    max_acc = 0
    iters = 5 # Iterations before checking accuracy

    print(f'Starting.')
    while True:

        for i in range(iters):
            batch_xs, batch_ys = get_dogs(batch_size)
            session.run(train_step, feed_dict={pixels: batch_xs, answers: batch_ys})
            debug = True

        acc, ls = session.run([accuracy, loss], feed_dict={pixels: batch_xs, answers: batch_ys})

        # print(session.run(weights[-1], feed_dict={pixels: batch_xs, answers: batch_ys}))
        print(f'After {int(time.time() - start)}s, accuracy is {acc:.0%}, loss is {ls}')
        if (acc > max_acc):
            max_acc = acc
            saver.save(session, 'my_test_model')
            print(f'After {int(time.time() - start)}s model saved with accuracy {acc:.0%}')

    winsound.Beep(440, 500)
    print(f'Max accuracy is {max_acc:.0%}')
