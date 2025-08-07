import serial
import time

ser = serial.Serial("COM3", 115200, timeout=1)

print("Listening for impacts...")
while True:
    line = ser.readline().decode().strip()
    if line:
        print("Received:", line)
