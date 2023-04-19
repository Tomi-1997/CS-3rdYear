import time

IMG1 = 'fr1.jpg'
IMG2 = 'fr2.jpg'

W = 600
H = W

import os
import cv2
import copy
import math
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.image as image
from pillow_heif import register_heif_opener

register_heif_opener()


def pil_open(path: str):
    img = image.imread(path)
    # img = cv2.resize(img, (W, H))
    return img


def cv2_open(path: str):
    if path.endswith('.HEIC'):
        pil_image = pil_open(path)
        open_cv_image = np.array(pil_image)
        # Convert RGB to BGR
        # open_cv_image = open_cv_image[:, :, ::-1].copy()
        return open_cv_image

    else:
        return pil_open(path)


def open(filename, grey=True):
    """Returns an image after resizing it."""
    img = image.imread(filename)
    img = img[:, :, ::-1].copy()
    if grey:
        img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    # img = cv2.resize(img, (W, H))
    return img


def show(img, size=None):
    """Plots image by size, if given- else by original size"""
    if size is not None:
        img = cv2.resize(img, size)

    cv2.imshow('name', img)
    cv2.waitKey(0)

    # closing all open windows
    cv2.destroyAllWindows()


def blur_image(img):
    """Uses gaussian blur to make simillar pixel even more simillar."""
    sig = 3
    smoothed = cv2.GaussianBlur(img, (0, 0), sigmaX=sig, sigmaY=sig, borderType=cv2.BORDER_DEFAULT)
    # smoothed = cv2.bilateralFilter(image,9,sig,sig)
    return smoothed


# Apply adaptive threshold on a gray image.
# An adaptive threhsold has different thresholds for each pixel, based on the region around it.
def adaptive_threshold(img):
    # https://docs.opencv.org/4.x/d7/d4d/tutorial_py_thresholding.html
    max_val = 150
    block = 3  # must be odd
    thresh = cv2.adaptiveThreshold(img, max_val, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY, block, 0)
    return thresh


def detect_star_keypoints(img):
    max_val = 150
    # Set up the SimpleBlobdetector with default parameters.
    params = cv2.SimpleBlobDetector_Params()

    # Change thresholds
    params.minThreshold = 0
    params.maxThreshold = max_val

    # Filter by Area.
    params.filterByArea = True
    params.minArea = 1  # filter out pixels less than here
    params.maxArea = 10000  # filter out pixels more than here

    # Filter by Color (black=0/white=255)
    params.filterByColor = True
    params.blobColor = 255

    # Filter by Circularity
    params.filterByCircularity = False

    # Filter by Convexity = defined as the (Area of the Blob / Area of it’s convex hull).
    params.filterByConvexity = False

    # Distance Between Blobs
    params.minDistBetweenBlobs = 1

    # Do detecting
    detector = cv2.SimpleBlobDetector_create(params)

    # Get keypoints
    keypoints = detector.detect(img)

    return keypoints


# Get keypoint locations and radius
def get_coords(img, keypoints):
    coordinates = []
    for keypoint in keypoints:
        x = int(keypoint.pt[0])
        y = int(keypoint.pt[1])
        s = keypoint.size
        r = int(math.floor(s))
        b = math.floor((img[y][x]))
        c = (x, y, r, b)
        coordinates.append(c)
    return coordinates


# Draw blobs
def draw_blobs(img, coords):
    """Shows an image with each star highlighted by index and a circle"""
    blobs = img.copy()
    color = (255, 0, 0)
    for c in coords:
        loc = (c[0], c[1])
        r = c[2]
        cv2.circle(blobs, loc, r, color, 2)
        cv2.putText(blobs, str(coords.index(c)), loc, cv2.FONT_HERSHEY_SIMPLEX, 1, color, 2, cv2.LINE_AA)

    size = (600, 600)
    show(blobs, size)


def star_doc(img_name):
    """Returns a list of tuples :
       for each elem in list, elem is - (x, y, r, brt)
       x, y : 2D location
       r : radius
       brt : brightness"""
    img = open(img_name, grey=True)
    img = blur_image(img)
    img = adaptive_threshold(img)
    keypoints = detect_star_keypoints(img)

    img = open(img_name, grey=True)
    cords = get_coords(img, keypoints)
    return cords


def binarify(img, cordinates):
    """Input: Image and star cordinates. Returns a copy of the image where all pixels are blackened except for the stars."""
    img = cv2_open(img)
    img = copy.deepcopy(img)
    img = np.zeros_like(img)

    for cords in cordinates:
        x, y, rad, brt = cords
        rad = rad + int(brt / 255 * 5)  # Paint a pixel's area based on radius and brightness.
        for i in range(x - rad, x + rad + 1):
            for j in range(y - rad, y + rad + 1):
                img[j][i] = [255, 255, 255]

    return img


def draw_side_by_side(img1, img2, brightest1, brightest2):
    """Draws two pictures side by side, highlighting the stars that were used to find a rotation between the images"""
    color = [255, 0, 0]
    for b1, b2 in zip(brightest1, brightest2):
        loc = (b1[0], b1[1])
        r = 40
        cv2.circle(img1, loc, r, color, 2)

        loc = (b2[0], b2[1])
        cv2.circle(img2, loc, r, color, 2)

    size = (1600, 900)
    # concatenate image Horizontally
    Hori = np.concatenate((img1, img2), axis=1)
    Hori = cv2.resize(Hori, size)

    cv2.imshow('HORIZONTAL', Hori)

    cv2.waitKey(0)
    cv2.destroyAllWindows()


def brt_and_rad(cord):
    """Returns a value based on a star cordinates' brightness and radius."""
    x, y, r, brt = cord
    return r * brt


def get_3_biggest(cords, img):
    n = 3
    temp_cords = copy.deepcopy(cords)
    h = img.shape[0]
    w = img.shape[1]
    ans = []
    cen_diff = 0.25

    # Extract three brightest that aren't at the edge of the photo
    for _ in temp_cords:

        if len(ans) == n:
            break

        best = max(temp_cords, key=lambda x: brt_and_rad(x))  # x[2] = radius
        x, y = best[0], best[1]
        if w * cen_diff < x < w * (1 - cen_diff) and h * cen_diff < y < h * (1 - cen_diff):
            ans.append([x, y])

        temp_cords.remove(best)

    return ans


def dist(u, v):
    """Returns a geometrical distance between two points"""
    x1, y1 = u[0], u[1]
    x2, y2 = v[0], v[1]
    return math.sqrt((x1 - x2) ** 2 + (y1 - y2) ** 2)


def find_x_in_img(x, target):
    """Finds the pair of stars X in target image, returns a list of positions: [ [x1, y1], [x2, y2] ]"""
    if (len(x) != 3): return None
    # Errors between target two stars to consider when searching for the same pair in another image
    d = [dist(x[0], x[1]), dist(x[1], x[2]), dist(x[2], x[0])]

    EPS = 100  ## Error to forgive
    best_err = EPS
    best_pair = None
    for star_x in target:
        for star_y in target:
            for star_z in target:

                if not star_x != star_y != star_z:
                    continue

                d1 = abs(dist(star_x, star_y) - d[0])
                d2 = abs(dist(star_y, star_z) - d[1])
                d3 = abs(dist(star_z, star_x) - d[2])

                if d1 + d2 + d3 < best_err:
                    print('found')
                    # Found a good result, don't return yet- could be a better one
                    best_err = d1 + d2 + d3
                    best_pair = [[star_x[0], star_x[1]], [star_y[0], star_y[1]], [star_z[0], star_z[1]]]

    print(best_err)
    return best_pair


def match(fname1, fname2):
    img1_open = open(fname1)
    img2_open = open(fname2)

    cords1 = star_doc(fname1)
    cords2 = star_doc(fname2)

    b1 = binarify(fname1, cords1)
    b2 = binarify(fname2, cords2)

    a = get_3_biggest(cords1, img1_open)
    b = find_x_in_img(x=a, target=cords2)

    draw_side_by_side(img1_open, img2_open, a, b)
    exit(0)
    # a = np.float32(a)
    # b = np.float32(b)
    #
    # ret = cv2.getAffineTransform(a, b)
    # ret = ret.astype(int)
    # a = np.float32( [[i[0], i[1]] for i in cords1] )
    # b = np.float32( [[i[0], i[1]] for i in cords2] )

    # print("A")
    # print(a)
    # print("Transform")
    # print(ret)
    #
    # dst = a @ ret #cv2.warpAffine(a, ret, (a.shape[1], a.shape[0]))
    # dst = dst.astype(int)
    # print("A @ Transform")
    # print(dst)
    # print("B")
    # print(b)


if __name__ == '__main__':
    match(IMG1, IMG2)
