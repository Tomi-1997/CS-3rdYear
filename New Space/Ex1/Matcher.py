import copy
import os
import cv2
import math
import random

import matplotlib.image as image
import matplotlib.pyplot as plt

# Heic files
from pillow_heif import register_heif_opener
register_heif_opener()

W = 800
H = 800

R = (W + H) / 100   # pixel blob size filter
EPS = 50            # error margin for brightness difference between two pixels, to be considered same blob
MIN_BRT = 270       # min brightness or consider a pixel to be a star


def match(img1, img2):
    pass


def unsafe(i, j):
    return i < 0 or i >= W or j < 0 or j >= H


def group_size(img, i, j, brt, traveled, max_depth):

    ans = 0

    # too deep
    if max_depth <= 0:
        return ans

    # out of bounds
    if unsafe(i, j):
        return ans

    # been here
    if traveled[i][j]:
        return ans

    traveled[i][j] = True
    curr_brt = brightness(img[i][j])

    # bright enough to be considered at the same blob, continue exploring
    if curr_brt < MIN_BRT and abs(curr_brt - brt) > EPS:
        return ans

    ans += 1
    for l in range(i - 1, i + 2):
        for k in range(j - 1, j + 2):
            if l == i and j == k: continue
            ans += group_size(img, l, k, brt, traveled, max_depth - 1)

    return ans


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


def brighter_than_area(img, i, j, brt):
    sample_size = 6
    sample_count = 30
    avg = 0
    index = 0

    while index < sample_count:
        ii = random.randint(i - sample_size, i + sample_size)
        jj = random.randint(j - sample_size, j + sample_size)

        if unsafe(ii, jj) or (ii == i and jj == j):
            continue

        index += 1
        avg += brightness(img[ii][jj]) / sample_count

    return brt > max(avg * 1.5, MIN_BRT)


def image_to_star_doc(img):
    im = copy.deepcopy(img)
    traveled = []
    avgbrt = avg_brightness(img)
    THRESH = max(avgbrt * 1.5, MIN_BRT)

    # mark explored pixels, avoid checking pixels twice
    for i, row in enumerate(im):
        traveled.append([False for _ in row])

    cords = []

    for i, row in enumerate(im):
        for j, pix in enumerate(row):

            if traveled[i][j] : continue

            # search for a bright star
            brt = brightness(pix)
            if (brt > THRESH and brighter_than_area(img, i, j, brt)):

                # find group which share around the same brightness around it
                r = group_size(img, i, j, brt, traveled, 10)
                print(f'i-{i},j-{j},r-{r}')

                # if it's too big, it might be dirt or clouds
                if r < R:
                    cords.append((i, j, r, brt))

            traveled[i][j] = True


    highlight_stars(im, cords)

    print(len(cords))
    print(cords)
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
    image = open_local('IMG_3051.HEIC')
    star_doc = image_to_star_doc(image)
    # highlight_stars(image, star_doc)
