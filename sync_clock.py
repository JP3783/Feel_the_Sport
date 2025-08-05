import serial
import time
import datetime

ser = serial.Serial("COM3", 115200)

while True:
    now = datetime.datetime.now()
    timestr = now.strftime("%Y-%m-%d %H:%M:%S")
    ser.write(timestr.encode())
    print("Sent:", timestr)
    time.sleep(60) #sync every minute
# import serial
# print("pyserial works!")
