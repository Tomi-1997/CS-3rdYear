import time
import serial

print_prefixes = ['alt:', 'lat:', 'lon:']
def monitor():
    ser = serial.Serial(COMPORT, BAUDRATE)

    while True:
        line = ser.readline()
        if len(line) > 0:
            try:
                text = line.decode("utf-8")
            except Exception:
                continue

            text_file = open("GS.log", "a")
            text_file.write(text)
            text_file.close()

            for prefix in print_prefixes:
                if prefix in text:
                    end_line = "\n" if prefix == 'lon:' else " "  # Assuming lon is the last value
                    text = text.strip()                         # Remove \n
                    print(text.split(':')[1], end=end_line)    # Print text after ':' symbol
        else:
            time.sleep(1)


COMPORT = 'COM5'
BAUDRATE = 115200

if __name__ == '__main__':
    monitor()
