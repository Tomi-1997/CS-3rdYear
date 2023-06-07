import sys, os, time
import serial


def monitor():
    ser = serial.Serial(COMPORT, BAUDRATE)

    while (1):
        line = ser.readline()
        if len(line) > 0:
            text_file = open("GS.log", "a")
            text = line.decode("utf-8")
            text_file.write(text)
            text_file.close()
        else:
            time.sleep(1)


COMPORT = 'COM3'
BAUDRATE = 115200

if __name__ == '__main__':
    monitor()
