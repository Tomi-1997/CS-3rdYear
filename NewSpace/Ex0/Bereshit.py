import math, Moon
import random, matplotlib.pyplot as plt
from PID import PID

WEIGHT_EMP = 165

MAIN_BURN = 0.15
MAIN_ENG_F = 430

SECOND_ENG_F = 25
SECOND_BURN = 0.009

ALL_BURN = MAIN_BURN + 8*SECOND_BURN


VS_ALL = []
DVS_ALL = []
HS_ALL = []
ALT_ALL = []
NN_ALL = []

def paint_ship(alt, ang, NN):
    pass

def plot(y, xargs):
    fig = plt.figure()
    ax1 = fig.add_subplot(1, 2, 1)
    ax1.plot(xargs[0], label = 'vs')
    ax1.plot(y, label = 'dvs')
    ax1.legend()

    ax2 = fig.add_subplot(1, 2, 2)
    ax2.plot(xargs[1])
    ax2.set_title('Alt')

    plt.show()

def minorplus(number, percent):
    z = random.random()
    return number * percent * z if random.random() > 0.5 else - number * percent * z

def accMax(weight):
    return acc(weight, True, 8);

def acc(weight, main, seconds):
    t = 0;
    if main: t += MAIN_ENG_F
    t += seconds * SECOND_ENG_F
    ans = t / weight
    return ans

def get_dvs(alt):
    # Between 1.5 and 30.
    return max(min(30, math.log(alt, 2) * 2) - 1, 1.5)

def simulate():

    pid = PID(P = 0.04, I = 0.0003, D = 0.2)

    print("Simulating Bereshit's Landing:")
    vs = random.uniform(20, 30)
    # vs = 30
    hs = 932
    hs = hs + minorplus(hs, 0.1) ## Hs +- 10%
    dist = 181 * 1000
    ang = random.uniform(50, 70)
    alt = random.randint(12000, 15000) # old val 13748
    time = 0
    dt = 1
    acc = 0
    fuel = 121

    weight = WEIGHT_EMP + fuel
    info = "time, vs, hs, dist, alt, ang, weight, acc"
    print(info)
    NN = 0.7

    LAND_MODE = 500
    while alt > 0:

        ## Lose of orientation
        # if alt < 2000:
        #     ang = random.randint(0, 360)

        if alt < LAND_MODE and ang > 0:
            ang -= 1
        if alt < LAND_MODE:
            dt = 0.1

        if time % 10 == 0 or alt < 100:
            # print(f'{time}, {vs}, {hs}, {dist}, {alt}, {ang}, {weight}, {acc}')
            print(f"time:{time}, vs:{vs}, hs:{hs}, alt:{alt}, fuel:{fuel}")

        dvs = get_dvs(alt)
        NN += pid.calc_error(vs, dvs)
        if NN > 1: NN = 1
        if NN < 0: NN = 0
        #
        # if alt < 500:
        #     if ang > 3: ang-= 3
        #     else: ang = 0

        ang_rad = math.radians(ang);
        h_acc = math.sin(ang_rad) * acc;
        v_acc = math.cos(ang_rad) * acc;
        vacc = Moon.getAcc(hs);
        time += dt;
        dw = dt * ALL_BURN * NN;

        if (fuel > 0):
            fuel -= dw
            weight = WEIGHT_EMP + fuel
            acc = NN * accMax(weight)

        else:
            print(f'no fuel!')
            acc=0

        v_acc -= vacc;
        if hs > 0: hs -= h_acc * dt
        else: hs = 0
        dist -= hs * dt;
        vs -= v_acc * dt;
        alt -= dt * vs;

        VS_ALL.append(vs)
        DVS_ALL.append(dvs)
        HS_ALL.append(hs)
        ALT_ALL.append(alt)
        NN_ALL.append(NN)

    print(f"time:{time}, vs:{vs}, hs:{hs}, dist:{dist}, alt:{alt}, ang:{ang}, weight:{weight}, acc:{acc}")

    safe_speed = 2.5
    if vs < safe_speed and hs < safe_speed:
        print('landed!')
    else:
        print(f'crashed at the speed of {vs}')

    plot(DVS_ALL, [VS_ALL, ALT_ALL])

if __name__ == '__main__':
    simulate()