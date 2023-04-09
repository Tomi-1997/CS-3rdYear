import copy
import os
import cv2
import math

import matplotlib.image as image
import matplotlib.pyplot as plt

# Heic files
from pillow_heif import register_heif_opener
register_heif_opener()

W = 800
H = 800

R = 15   # pixel blob size filter
EPS = 20 # error margin for brightness difference between two pixels.

def match(img1, img2):
    pass


def unsafe(i, j):
    return i < 0 or i >= W or j < 0 or j >= H


def group_size(img, i, j, brt, traveled, max_depth):

    # too deep
    if max_depth <= 0:
        return 0

    # out of bounds
    if unsafe(i, j):
        return 0

    # been here
    if traveled[i][j]:
        return 0

    traveled[i][j] = True

    # bright enough to be considered at the same blob, continue exploring
    curr_brt = brightness(img[i][j])
    if math.fabs(curr_brt - brt) > EPS:
        return 0

    return 1 + group_size(img, i + 1, j, brt, traveled, max_depth - 1) \
             + group_size(img, i - 1, j, brt, traveled, max_depth - 1) \
             + group_size(img, i, j + 1, brt, traveled, max_depth - 1) \
             + group_size(img, i, j - 1, brt, traveled, max_depth - 1)


def avg_brightness(img):
    size = 0
    brt = 0
    for i, row in enumerate(img):
        for j, col in enumerate(row):
            brt += brightness(col)
            size += 1

    return brt / size


def brightness(pixel):
    return 0.2126 * pixel[0] + 0.7152 * pixel[1] + 0.0722 + pixel[2]


def draw_rect(img, cl, pos, size):
    x = pos[0]
    y = pos[1]

    for i in range(x - size, x + size):
        if not unsafe(i, y + size):
            img[i][y + size] = cl
        if not unsafe(i, y - size):
            img[i][y - size] = cl

    for j in range(y - size, y + size):
        if not unsafe(x + size, j):
            img[x + size][j] = cl
        if not unsafe(x - size, j):
            img[x - size][j] = cl


def image_to_star_doc(img):
    im = copy.deepcopy(img)
    traveled = []
    threshold = avg_brightness(img) * 2

    # mark explored pixels, avoid checking pixels twice
    for i, row in enumerate(im):
        traveled.append([False for _ in row])

    cords = []

    for i, row in enumerate(im):
        for j, pix in enumerate(row):

            if traveled[i][j] : continue

            # search for a bright star
            brt = brightness(pix)
            if (brt > threshold):

                # find group which share around the same brightness around it
                r = group_size(img, i, j, brt, traveled, 20)

                # if it's too big, it might be dirt or clouds
                if r < R:
                    cords.append((i, j))


    highlight_stars(im, cords)

    print(len(cords))
    show(im)
    return cords


def highlight_stars(im, doc):
    for tup in doc:
        draw_rect(img=im, cl=[255, 0, 0], pos=(tup[0], tup[1]), size=5)


def open_local(filename):
    """Returns an image after resizing it."""
    img = image.imread(filename)
    img = cv2.resize(img, (W, H))
    return img


def show(img):
    """Plots image"""
    plt.imshow(img)
    plt.show()


if __name__ == '__main__':
    image = open_local('IMG_3060.HEIC')
    star_doc = image_to_star_doc(image)
    # highlight_stars(image, star_doc)
