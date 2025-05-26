# import serial

# COM_PORT = 'COM7'  # Adjust if needed
# BAUD_RATE = 115200

# try:
#     with serial.Serial(COM_PORT, BAUD_RATE, timeout=1) as ser:
#         print(f"Connected to {COM_PORT}")
#         print("Type 'exit' or 'quit' to end")
#         while True:
#             user_input = input("Enter command (e.g., 500,Low): ")
#             if user_input.lower() in ["exit", "quit"]:
#                 break
#             ser.write((user_input + '\n').encode())
# except serial.SerialException as e:
#     print(f"Serial error: {e}")
# except Exception as e:
#     print(f"Unexpected error: {e}")

import serial
import time
from datetime import datetime

MICROBIT_PORT = 'COM8'
WATCH_PORT = 'COM7'
BAUD_RATE = 115200

try:
    microbit = serial.Serial(MICROBIT_PORT, BAUD_RATE, timeout=1)
    watch = serial.Serial(WATCH_PORT, BAUD_RATE, timeout=1)
    print(f"Listening for crash signal on {MICROBIT_PORT}")
    print(f"Sending vibration to watch via {WATCH_PORT}")
    
    while True:
        line = microbit.readline().decode().strip()
        if line:
            now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            print(f"[{now}] Received from micro:bit: {line}")
            if "CRASH" in line.upper():
                print(f"[{now}] 🚨 Crash detected! Sending vibration to T-Watch.")
                watch.write(b"500,High\n")  # 500ms High intensity buzz
        time.sleep(0.1)

except serial.SerialException as e:
    print(f"Serial error: {e}")
except Exception as e:
    print(f"Unexpected error: {e}")
finally:
    try:
        microbit.close()
        watch.close()
    except:
        pass