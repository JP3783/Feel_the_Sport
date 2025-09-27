import pandas as pd
import serial
import time
from datetime import datetime


COM_PORT = 'COM6'   #Adjust if needed
BAUD_RATE = 115200
CSV_FILE = "sync_test.csv"
VIDEO_FILE = "sync_test.mp4"
DURATION_MS = 10

#define strengths of vibrations
def get_strength(mag):
    if mag >= 2:
        return "High"
    # elif mag >= 2:
    #     return "Med"
    # elif mag >= 1.5:
    #     return "Low"
    else:
        return None
    
df = pd.read_csv(CSV_FILE)
#parse timestamps into datetime objects
df["Time"] = pd.to_datetime(df["Time"], format="%H:%M:%S.%f")
start_time = df["Time"].iloc[0]
df["Delta"] = (df["Time"] - start_time).dt.total_seconds()


try:
    with serial.Serial(COM_PORT, BAUD_RATE, timeout=1) as ser:
        print(f"Connected to {COM_PORT}")

        #Countdown before starting playback
        for i in range(5, 0, -1):
            print(f"Starting in {i}...")
            time.sleep(1)

        print("Go!")

        start_perf = time.perf_counter()
        for _, row in df.iterrows():
            mag = row["Mag"]
            delta = row["Delta"]

            #Wait until we reach this event's timestamp
            while (time.perf_counter() - start_perf) < delta:
                time.sleep(0.001)  #avoid busy waiting

            strength = get_strength(mag)
            if strength:  #Only send if valid
                message = f"{DURATION_MS},{strength}"
                ser.write((message + "\n").encode())
                print(f"[{row['Time'].time()}] Sent: {message}")

        print("Finished sending events.")
        # while True:
        #     user_input = input("Enter command (e.g., 500,Low): ")
        #     if user_input.lower() in ["exit", "quit"]:
        #         break
        #     start = time.perf_counter()
        #     ser.write((user_input + '\n').encode())
except serial.SerialException as e:
    print(f"Serial error: {e}")
except Exception as e:
    print(f"Unexpected error: {e}")