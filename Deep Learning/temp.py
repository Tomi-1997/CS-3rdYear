import numpy
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.image as img
import os, random
import tensorflow.compat.v1 as tf
import tensorflow.compat.v1.keras.preprocessing.image as tf_img
import cv2
tf.disable_v2_behavior()

batch_size = 100
img_height = 500
img_width = 500

directory = 'C:\\Users\\tomto\\PycharmProjects\\RandomStuff\\dog-breed-identification\\'
labels = pd.read_csv(directory + '\\labels.csv')
breeds = pd.unique(labels['breed'])
breeds_num = len(breeds)
train_dir = directory + '\\train\\'
test_dir = directory + '\\test\\'

# Returns a tuple of (image, label)
def get_dog():
    id = random.choice(os.listdir(train_dir))
    dir = train_dir + id

    img_ans = []
    my_img = img.imread(dir, img_height * img_width)
    z = 0
    for i in my_img:
        if z == img_width * img_height:
            break
        for j in i:
            if z == img_width * img_height:
                break
            for k in j:
                if z == img_width * img_height:
                    break
                img_ans.append(k)
                z += 1

    my_len = len(img_ans)
    while my_len < img_height * img_width:
        img_ans.append(0)
        my_len += 1

    return img_ans, get_ans(id)

def get_dogs(num):
    dogs = []
    ans = []
    for i in range(num):
        d, a = get_dog()
        dogs.append(d)
        ans.append(a)
    return dogs, ans

def find_index(breed_name):
    for i, breed in enumerate(breeds):
        if breed == breed_name:
            return i
    return -1

def get_ans(id):
    new_id = id.replace(".jpg","")
    breed = [0 for i in range(breeds_num)]
    dog_file = pd.read_csv(directory + 'labels.csv')

    for dog in dog_file.values:
        if dog[0] == new_id:
            breed[find_index(dog[1])] = 1

    return breed

if __name__ == '__main__':
    pixel_num = img_width * img_height
    pixels = tf.placeholder(tf.float32,[None, pixel_num])
    answers = tf.placeholder(tf.float32, [None, breeds_num])
    weights = tf.Variable(tf.zeros([pixel_num, breeds_num]))
    bias = tf.Variable(tf.zeros([breeds_num]))

    predicate = tf.nn.softmax(tf.matmul(pixels, weights) + bias)
    cross_entropy = tf.reduce_mean(-tf.reduce_sum(answers * tf.log(predicate), reduction_indices = [1]))
    train_step = tf.train.GradientDescentOptimizer(0.5).minimize(cross_entropy)
    init = tf.global_variables_initializer()

    session = tf.Session()
    session.run(init)

    for i in range(1000):
        batch_xs, batch_ys = get_dogs(batch_size)
        session.run(train_step, feed_dict={pixels: batch_xs, answers: batch_ys})

    correct_prediction = tf.equal(tf.argmax(predicate,1), tf.argmax(answers,1))
    accuracy = tf.reduce_mean(tf.cast(correct_prediction, tf.float32))

    batch, labels = get_dogs(batch_size)
    print(session.run(accuracy, feed_dict={pixels: batch, answers: labels}))
