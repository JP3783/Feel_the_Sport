import pandas as pd
import serial
import time
from datetime import datetime
import os

# ====== CONFIG ======
COM_PORT = 'COM6'   # Adjust if needed
BAUD_RATE = 115200
CSV_FILE = "fc2_3min.csv"
VIDEO_FILE = "C:/Users/jpout/Feel_the_Sport_2025/FC2_3min.mp4"
# ====================


def get_strength(mag):
    """Convert magnitude to vibration strength."""
    if mag >= 2.5:
        return "High"
    elif mag >= 2:
        return "Low"
    else:
        return None


#load csv
df = pd.read_csv(CSV_FILE)

#parse timestamps into datetime objects
df["Timestamp"] = pd.to_datetime(df["Timestamp"])

#use the first timestamp as the reference
start_time = df["Timestamp"].iloc[0]

try:
    with serial.Serial(COM_PORT, BAUD_RATE, timeout=1) as ser:
        print(f"[INFO] Connected to {COM_PORT}")

        # # -----------MC1-------------
        # #countdown
        # for sec in range(5, 0, -1):
        #     print(f"Starting in {sec}...")
        #     time.sleep(1)
        # #start video
        # os.startfile(VIDEO_FILE)
        # time.sleep(2.3)
        # start_perf = time.perf_counter()
        # print("[INFO] Video started, sending vibration events...")


        # # -----------MC2-------------
        # #countdown
        # for sec in range(9, 0, -1):
        #     print(f"Starting in {sec}...")
        #     if sec == 6:
        #         os.startfile(VIDEO_FILE)
        #     time.sleep(1)
        # #start video
        # # os.startfile(VIDEO_FILE)
        # # time.sleep(2.3)
        # start_perf = time.perf_counter()
        # print("[INFO] Video started, sending vibration events...")

        # # -----------FC1-------------
        # #countdown
        # for sec in range(5, 0, -1):
        #     print(f"Starting in {sec}...")
        #     time.sleep(1)
        # #start video
        # os.startfile(VIDEO_FILE)
        # time.sleep(1.5)
        # start_perf = time.perf_counter()
        # print("[INFO] Video started, sending vibration events...")

        # -----------FC2-------------
        #countdown
        for sec in range(5, 0, -1):
            print(f"Starting in {sec}...")
            time.sleep(1)
        #start video
        os.startfile(VIDEO_FILE)
        time.sleep(4.5)
        start_perf = time.perf_counter()
        print("[INFO] Video started, sending vibration events...")

        

        #send vibrations
        for _, row in df.iterrows():
            delta = (row["Timestamp"] - start_time).total_seconds()

            #wait until the event time
            while (time.perf_counter() - start_perf) < delta:
                time.sleep(0.001)

            strength = get_strength(row["AvgMag"])
            if strength:
                message = f"{int(row['Duration'] * 1000)},{strength}"
                ser.write((message + "\n").encode())
                print(f"[{row['Timestamp'].time()}] Sent: {message}")

        print("[INFO] Finished sending all vibration events.")

except serial.SerialException as e:
    print(f"[ERROR] Serial connection failed: {e}")
except Exception as e:
    print(f"[ERROR] Unexpected error: {e}")
