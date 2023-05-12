import random
import pygame
from datetime import timedelta
from ambiance import Atmosphere


MAX_HEIGHT = 40000
MIN_HEIGHT = 0

## Altitude to maintain
LOWER = 10000
UPPER = 30000

## -------------    30,000  -------------
## ------------- ............  -------------
## ------------- 30,000 - buffer -------------
## ------------- ...............  -------------
## ------------- Balloon should be here --------
## ------------- ...............  -------------
## ------------- 10,000 + buffer -------------
## ------------- ............  -------------
## -------------    10,000  -------------

BUFFER = 5000

DT = 100 ## 1 DT = 1 Second
GRAMS = 1
KG = 1000 * GRAMS


BALOON_WEIGHT = 600 * GRAMS
PAYLOAD_WEIGHT = 300 * GRAMS
ANTI_FREEZE_AMOUNT = 0.1 * KG


# Pygame constants
X_SIZE = 600
Y_SIZE = 300
size = [X_SIZE, Y_SIZE]
white = [255, 255, 255]
red = [255, 0, 0]
grey = [70, 70, 70]


def main():

    pygame.init()
    seconds = 0
    iters = 0

    screen = pygame.display.set_mode(size)
    clock = pygame.time.Clock()
    run = True
    ascent = True                           ## Log time took to reach 10 km
    pop = False                             ## Too high \ liquid ran out
    b = Baloon()

    while run:

        if iters % 100 == 0: b.log(seconds)
        if b.height >= 10000 and ascent:
            b.log(seconds)
            ascent = False

        iters += 1
        seconds += DT

        if b.height > MAX_HEIGHT or 0 > b.height:  ## Check limits
            pop = True
            run = False

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                run = False

        draw(screen, b)                            ## Draw limit, buffer zone and balloon
        update(b, seconds)
        pygame.display.flip()
        clock.tick(60)


    if pop:
        print(f'Popped at {b.height} after {sec_to_time(seconds)}.')
    pygame.quit()
    exit(0)


class Baloon():
    def __init__(self):
        self.x = random.randint(20, 200)
        self.height = 0
        self.anti_freeze_left = ANTI_FREEZE_AMOUNT
        self.weight = self.calc_weight()
        self.helium_left = self.weight * GRAMS + 2 # Enough to lift at start
        self.lift = 0;


    def calc_weight(self):
        res = BALOON_WEIGHT + PAYLOAD_WEIGHT
        res += self.anti_freeze_left
        return res


    def helium_leak(self):
        self.helium_left = max((self.helium_left - 0.00004 * DT), 0)


    def vent(self):
        self.helium_left = max(self.helium_left - 0.02 * DT, 0)


    def drip(self):
        self.anti_freeze_left = max(self.anti_freeze_left - 0.05 * DT, 0)


    def update(self, time):
        self.x += random.random() * 0.001 * DT
        self.helium_leak()
        self.weight = self.calc_weight()
        prev_height = self.height

        # current_hour = time // 3600
        # current_hour = current_hour % 24

        lift = (self.helium_left - self.weight)
        self.height += DT * lift

        # atmo = Atmosphere(self.height)
        # temp = atmo.temperature_in_celsius
        # pressure = atmo.pressure[0]

        rising = self.height - prev_height >= 0.01 * DT
        dropping = not rising

        if rising and self.height > UPPER - BUFFER:
            self.vent()

        if dropping and self.height < LOWER + BUFFER:
            self.drip()


    def log(self, sec = 0):
        print(f'Weight - {round(self.weight, 3)}, '
              f'Alt - {round(self.height, 3)}, '
              f'Anti-freeze - {round(self.anti_freeze_left, 3)},'
              f' Helium - {round(self.helium_left, 3)},'
              f' Time - {sec_to_time(sec)}')


def draw(screen, b):
    ## BLACK SCREEN -> RED ZONE -> 10 TO 30 KM LINES -> BALOON
    screen.fill([0, 0, 0])

    ## RED ZONE - MAX HEIGHT
    start = translate(0, MAX_HEIGHT)
    end = translate(X_SIZE, MAX_HEIGHT)
    pygame.draw.line(surface=screen, color=red, start_pos=start, end_pos=end)

    ## LOWER LIM
    start = translate(0, LOWER)
    end = translate(X_SIZE, LOWER)
    pygame.draw.line(surface=screen, color=white, start_pos=start, end_pos=end)

    ## LOWER BUFFER
    start = translate(0, LOWER + BUFFER)
    end = translate(X_SIZE, LOWER + BUFFER)
    pygame.draw.line(surface=screen, color=grey, start_pos=start, end_pos=end)

    ## UPPER LIM
    start = translate(0, UPPER)
    end = translate(X_SIZE, UPPER)
    pygame.draw.line(surface=screen, color=white, start_pos=start, end_pos=end)

    ## UPPER LIM BUFFER
    start = translate(0, UPPER - BUFFER)
    end = translate(X_SIZE, UPPER - BUFFER)
    pygame.draw.line(surface=screen, color=grey, start_pos=start, end_pos=end)

    ## OUR BALOON
    x, y = translate(b.x, b.height)
    pygame.draw.circle(surface=screen, color=white, center=[x, y], radius=10, width=1)
    pygame.draw.circle(surface=screen, color=grey, center=[x, y], radius=1, width=1)


def update(b, time):
    b.update(time)


def sec_to_time(seconds):
    td = timedelta(seconds=seconds)
    return td


def translate(x, y):
    y = y * Y_SIZE / (MAX_HEIGHT + 2000) ## Y -> [0, Y_SIZE]
    x = x

    return x, Y_SIZE - y


if __name__ == '__main__':
    main()


