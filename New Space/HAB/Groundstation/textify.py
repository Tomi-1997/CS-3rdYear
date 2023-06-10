import time
import serial


def monitor():
    ser = serial.Serial(COMPORT, BAUDRATE)

    while True:
        line = ser.readline()
        if len(line) > 0:
            text = line.decode("utf-8")
            text_file = open("GS.log", "a")
            text_file.write(text)
            text_file.close()

            print(text, end="")
        else:
            time.sleep(1)


COMPORT = 'COM5'
BAUDRATE = 115200

if __name__ == '__main__':
    monitor()
