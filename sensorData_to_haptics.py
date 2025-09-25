import pandas as pd
import serial
import time

COM_PORT = 'COM6'   #Adjust if needed
BAUD_RATE = 115200
CSV_FILE = "sync_test2509.csv"
VIDEO_FILE = "sync_test2509.mp4"


def mag_to_strength(mag):
    """Map magnitude to Low/Med/High vibration strength."""
    if mag >= 3.0:
        return "High"
    elif mag >= 2.0:
        return "Med"
    else:
        return "Low"


try:
    with serial.Serial(COM_PORT, BAUD_RATE, timeout=1) as ser:
        print(f"Connected to {COM_PORT}")
        print("Type 'exit' or 'quit' to end")
        while True:
            user_input = input("Enter command (e.g., 500,Low): ")
            if user_input.lower() in ["exit", "quit"]:
                break
            start = time.perf_counter()
            ser.write((user_input + '\n').encode())

except serial.SerialException as e:
    print(f"Serial error: {e}")
except Exception as e:
    print(f"Unexpected error: {e}")