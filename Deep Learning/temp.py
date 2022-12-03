
# To save image data as binary (faster than builtin write)
import pickle
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

batch_size = 20
img_height = 300
img_width = 300

directory = 'C:\\Users\\tomto\\PycharmProjects\\RandomStuff\\dog-breed-identification\\'
labels = pd.read_csv(directory + '\\labels.csv')
breeds = pd.unique(labels['breed'])
breeds_num = len(breeds)
train_dir = directory + '\\train\\'
train_dir_normalised = directory + '\\train_normalised\\'
test_dir = directory + '\\test\\'

def save(id):
    """"Opens an image by id from the training folder and saves the data as a normalised list."""
    dir = train_dir + id
    image_to_list = []
    my_img = img.imread(dir + '.jpg', img_height * img_width)
    for i in my_img:
        for j in i:
            for k in j:
                image_to_list.append(k)

    list_to_bin = bytearray(image_to_list)
    with open(train_dir_normalised + id, 'wb') as f:
        pickle.dump(list_to_bin, f)

def save_all():
    ids = labels.get(key="id")
    for id in ids:
        save(id)

def resize_all(w, h):
    """Resizes all pictures in the training folder. (Overwrites)"""
    ids = labels.get(key="id")

    for id in ids:
        image = Image.open(train_dir + id + '.jpg')
        new_image = image.resize((w, h))
        new_image.save(train_dir + id + '.jpg')

def get_dog():
    """Returns a tuple of ( [Image RGB values divided by 255], [0,0... 1, 0, 0] 1 for the index of the breed)"""
    id = random.choice(os.listdir(train_dir))
    dir = train_dir_normalised + id.replace(".jpg","")

    img_ans = pickle.load(open(dir, "rb"))

    return img_ans, get_ans(id)

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

def get_ans(id):
    """Returns a vector of ones and zeros corresponding to the dog's breed."""
    new_id = id.replace(".jpg","")
    breed = [0 for i in range(breeds_num)]
    dog_file = pd.read_csv(directory + 'labels.csv')

    for dog in dog_file.values:
        if dog[0] == new_id:
            breed[find_index(dog[1])] = 1

    return breed

if __name__ == '__main__':

    # save_all()
    
    # Unmark this only if you have the original backup folder of pictures.
    # resize_all(img_width, img_height)
    start = time.time()

    pixel_num = img_width * img_height * 3
    pixels = tf.placeholder(tf.float32,[batch_size, pixel_num])
    answers = tf.placeholder(tf.float32, [batch_size, breeds_num])
    weights = tf.Variable(tf.zeros([pixel_num, breeds_num]))
    bias = tf.Variable(tf.zeros([breeds_num]))

    predicate = tf.nn.softmax(tf.matmul(pixels, weights) + bias)
    cross_entropy = tf.reduce_mean(-tf.reduce_sum(answers * tf.log(predicate), reduction_indices = [1]))
    train_step = tf.train.GradientDescentOptimizer(0.001).minimize(cross_entropy)

    saver = tf.train.Saver()
    init = tf.global_variables_initializer()

    session = tf.Session()
    session.run(init)
    max_acc = 0
    iters = 10 # Iterations before checking accuracy
    time_limit = 60

    print(f'Starting, running for {time_limit} seconds.')
    while (time.time() - start) < time_limit:
        for i in range(iters):
            batch_xs, batch_ys = get_dogs(batch_size)
            session.run(train_step, feed_dict={pixels: batch_xs, answers: batch_ys})
            correct_prediction = tf.equal(tf.argmax(predicate,1), tf.argmax(answers,1))
            accuracy = tf.reduce_mean(tf.cast(correct_prediction, tf.float32))

        batch, labels = get_dogs(batch_size)
        acc = session.run(accuracy, feed_dict={pixels: batch, answers: labels})
        # print(f'Finished batch with {acc} accuracy.')
        if (acc > max_acc):
            max_acc = acc
            saver.save(session, 'my_test_model')
            print(f'Model saved with accuracy {acc}')
