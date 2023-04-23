"""
Star Tracker, and Star Matcher.

*** *** User guide: *** ***

** ****** ****** ****** ****** ****** ****** ****** ****** ****** ******
** Star Tracker **                                                     *
** To find stars in an image, define the file name and call star_doc() *
**                                                                     *
** myimage = "nightsky.png"                                            *
** result = star_doc(myimage)                                          *
**                                                                     *
** To display them, call draw_blobs with an open iamge and the result. *
**                                                                     *
** myimage_open = pil_open(myimage)                                    *
** draw_blobs(myimage_open, result)                                    *
** ****** ****** ****** ****** ****** ****** ****** ****** ****** ******


** ****** ****** ****** ****** ****** ****** ****** ****** ****** ******
** Matcher **                                                          *
** Given two simillar images, returns two lists of elements,           *
** such that star from A[i] is equal to star from B[i]                 *
**                                                                     *
** define two filenames, and call match (fname1, fname2)               *
** res = match(fname1, fname2)                                         *
**                                                                     *
** To display the proccess- Show the three stars from A that were      *
** used to find a transformation, and two images side by side:         *
** call match with show = True                                         *
** res = match(fname1, fname2, show = True)                            *
** ****** ****** ****** ****** ****** ****** ****** ****** ****** ******

"""
import time
import os
import cv2
import copy
import math
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.image as image
from pillow_heif import register_heif_opener

register_heif_opener()
# yellow in bgr
color = [0, 255, 255]


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
    # img = image.imread(filename)
    img = cv2.imread(filename)
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
    for c in coords:
        loc = (c[0], c[1])
        r = c[2]
        cv2.circle(blobs, loc, r, color, 1)
        cv2.putText(blobs, str(coords.index(c)), loc, cv2.FONT_HERSHEY_SIMPLEX, 1, color, 2, cv2.LINE_AA)

    size = (900, 900)
    show(blobs, size)


def star_doc(img_name):
    """ Input - filename of a star picture
        Returns a list of tuples :
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


def show2(img1, img2):
    """Shows two images side by side"""
    s = 900
    img1 = cv2.resize(img1, (s,s))
    img2 = cv2.resize(img2, (s,s))
    size = (s * 2, s)
    # concatenate image Horizontally
    Hori = np.concatenate((img1, img2), axis=1)
    Hori = cv2.resize(Hori, size)

    cv2.imshow('HORIZONTAL', Hori)

    cv2.waitKey(0)
    cv2.destroyAllWindows()


def draw_side_by_side(img1, img2, brightest1, brightest2):
    """Draws two pictures side by side, highlighting the stars that were used to find a rotation between the images"""
    color = [255, 0, 0]
    for b1, b2 in zip(brightest1, brightest2):
        loc = (b1[0], b1[1])
        r = 40
        cv2.circle(img1, loc, r, color, 2)

        loc = (b2[0], b2[1])
        cv2.circle(img2, loc, r, color, 2)

    s = 900
    img1 = cv2.resize(img1, (s,s))
    img2 = cv2.resize(img2, (s,s))
    size = (s * 2, s)

    # concatenate image Horizontally
    Hori = np.concatenate((img1, img2), axis=1)
    x, y = int(Hori.shape[0] * 0.5), int(Hori.shape[1] * 0.1)
    cv2.putText(Hori, "Star group to find transformation", (x, y), cv2.FONT_HERSHEY_SIMPLEX, 1, color, 2, cv2.LINE_AA)
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
            ans.append(best)

        temp_cords.remove(best)

    if len(ans) < 2: return None
    return ans


def dist(u, v):
    """Returns a geometrical distance between two points"""
    x1, y1 = u[0], u[1]
    x2, y2 = v[0], v[1]
    return math.sqrt((x1 - x2) ** 2 + (y1 - y2) ** 2)


def similar_rbrt(l1, l2):
    epsr = 2
    epsbrt = 20
    for x, y in zip(l1, l2):
        if abs(x[2] - y[2]) > epsr or abs(x[3] - y[3]) > epsbrt:
            return False
    return True


def find_x_in_img(x, target):
    """Finds the pair of stars X in target image, returns a list of positions: [ [x1, y1], [x2, y2] ]"""
    if (len(x) != 3): return None
    # Errors between target two stars to consider when searching for the same pair in another image
    d = [dist(x[0], x[1]), dist(x[1], x[2]), dist(x[2], x[0])]

    eps = 200  ## Error to forgive
    best_err = eps
    best_res = None

    runs = 0
    n = len(target)
    for i in range(n):
        for j in range(n):
            for k in range(n):

                sx = target[i]
                sy = target[j]
                sz = target[k]

                if not sx != sy != sz:
                    continue

                d1 = abs(dist(sx, sy) - d[0]) # x -> y
                d2 = abs(dist(sy, sz) - d[1]) # y -> z
                d3 = abs(dist(sz, sx) - d[2]) # z -> x

                if d1 + d2 + d3 < best_err and similar_rbrt( [sx, sy, sz], x):
                    # Found a good result, don't return yet- could be a better one
                    best_err = d1 + d2 + d3
                    best_res = [sx, sy, sz]
    return best_res


def match(fname1, fname2, show = False):
    """input: img A, img B, show = bool
    Returns two lists of [xpos, ypos] between corresponding stars in pictures A and B.
    if show is true it plots the two pictures side by side with indexes of each star."""
    not_found = 'Match not found.'

    cords1 = star_doc(fname1)
    cords2 = star_doc(fname2)

    if cords1 is None or cords2 is None:
        print(not_found)
        return 0

    img1 = open(fname1)
    img2 = open(fname2)
    a = get_3_biggest(cords1, img1)

    if a is None:
        print(not_found)
        return 0

    b = find_x_in_img(x=a, target=cords2)
    if b is None:
        print(not_found)
        return 0

    if show:
        draw_side_by_side(img1, img2, a, b)

    # Source - https://stackoverflow.com/questions/20546182/how-to-perform-coordinates-affine-transformation-using-python-part-2?answertab=votes#tab-top
    # Pad the data with ones, so that our transformation can do translations too
    a = np.array([ [elem[0], elem[1]] for elem in a ])
    b = np.array([ [elem[0], elem[1]] for elem in b ])

    n = a.shape[0]
    pad = lambda x: np.hstack([x, np.ones((x.shape[0], 1))])
    unpad = lambda x: x[:, :-1]
    X = pad(a)
    Y = pad(b)

    # Solve the least squares problem X * A = Y
    # to find our transformation matrix A
    A, res, rank, s = np.linalg.lstsq(X, Y, rcond=None)

    transform = lambda x: unpad(np.dot(pad(x), A))

    # From list, take [x, y] -> np array
    cords1 = np.array( [ [elem[0], elem[1]] for elem in cords1 ] )
    cords2 = np.array( [ [elem[0], elem[1]] for elem in cords2 ] )

    cords1_transformed = transform(cords1)

    # Lists of matching stars return
    cords1_cp = []
    cords1_trans_cp = []

    # Count only star in second's image boundary
    h, w = img2.shape[0], img2.shape[1]
    index = 0
    for x, y in cords1_transformed:

        in_bounds = w > x > 0 and h > y > 0
        if in_bounds:
            x1, y1 = cords1[index]
            cords1_cp.append( [int(x1), int(y1)] )
            x2, y2 = cords1_transformed[index]
            cords1_trans_cp.append( [int(x2), int(y2)] )
        index += 1


    msg = f"Found {len(cords1_trans_cp)} matching stars, returning two lists." if len(cords1_trans_cp) > 5 else not_found
    print(msg)

    if show:
        img1_open = pil_open(fname1) # Open with colours
        img2_open = pil_open(fname2)
        index = 1
        loc_eps = 10

        fsize = 1 if h < 2000 or w < 2000 else 3
        for c1, c2 in zip(cords1_cp, cords1_trans_cp):
            loc = (int(c1[0] + loc_eps), int(c1[1] + loc_eps))
            cv2.putText(img1_open, str(index), loc, cv2.FONT_HERSHEY_SIMPLEX, fsize, color, 2, cv2.LINE_AA)

            loc = (int(c2[0] + loc_eps), int(c2[1] + loc_eps))
            cv2.putText(img2_open, str(index), loc, cv2.FONT_HERSHEY_SIMPLEX, fsize, color, 2, cv2.LINE_AA)

            index += 1

        show2(img1_open, img2_open)


    return cords1_cp, cords1_trans_cp


if __name__ == '__main__':

    myimage = "ST_db1.png"
    result = star_doc(myimage)
    myimage_open = pil_open(myimage)
    draw_blobs(myimage_open, result)

    fname1 = 'fr1.jpg'
    fname2 = 'fr2.jpg'
    res = match(fname1, fname2, show = True)
    print(res)
