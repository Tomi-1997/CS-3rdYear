import copy
import os

import matplotlib.image as image
import matplotlib.pyplot as plt

def match(img1, img2):
    pass


def darken_image(mat):
    """Zeroes pixels below a certain threshold, and whites-out others."""
    black = [0, 0, 0]
    white = [255, 255, 255]
    kinda_dark = 100

    for i, row in enumerate(mat):
        for j, col in enumerate(row):

            ## https://stackoverflow.com/questions/6442118/python-measuring-pixel-brightness
            brightness = sum(col) / len(col)
            is_dark = brightness < kinda_dark

            mat[i][j] = black if is_dark else white


def open_local(filename):
    """Opens image at the same directory as the code"""
    directory_path = os.path.dirname(__file__)
    file_path = os.path.join(directory_path, 'IMG_3063.jpg')
    img = image.imread(file_path)
    edited = copy.deepcopy(img)
    return edited


def show(img):
    """Plots image"""
    plt.imshow(img)
    plt.show()


if __name__ == '__main__':
    edited = open_local('IMG_3063.jpg')
    darken_image(edited)
    show(edited)
