import pandas as pd
import serial
import time
from datetime import datetime
import os


COM_PORT = 'COM6'   #Adjust if needed
BAUD_RATE = 115200
CSV_FILE = "sync_test2.csv"
VIDEO_FILE = "sync_test2.mp4"
DURATION_MS = 10

OUTPUT_FILE = "processed_hits.csv"

#define strengths of vibrations
def get_strength(mag):
    if mag >= 3:
        return "High"
    elif mag >= 2.5:
        return "Med"
    elif mag >= 2:
        return "Low"
    else:
        return None
    
#read csv file into Pandas
df = pd.read_csv(CSV_FILE)
#parse timestamps into datetime objects
df["Time"] = pd.to_datetime(df["Time"], format="%H:%M:%S.%f")
#gets the first 'Time' value
start_time = df["Time"].iloc[0]
#calculate difference between times
df["Delta"] = (df["Time"] - start_time).dt.total_seconds()


#Process HIT events into groups
processed_rows = []
i=0
while i < len(df):
    if df.loc[i, "Notes"] == "HIT":
        first_hit_time = df.loc[i, "Time"]
        first_index = i
        latest_hit_time = first_hit_time
        hit_mags = [df.loc[i, "Mag"]]

        skip = 0
        j = i + 1
        while j < len(df) and skip <= 5:
            if df.loc[j, "Notes"] == "HIT":
                latest_hit_time = df.loc[j, "Time"]
                hit_mags.append(df.loc[j, "Mag"])
                skip = 0
            else:
                skip += 1
            j += 1
        
        #once skip exceeded 5, finalize this group
        duration = (latest_hit_time - first_hit_time).total_seconds()
        avg_mag = sum(hit_mags) / len(hit_mags)
        timestamp = first_hit_time

        processed_rows.append({"Timestamp": timestamp, "Duration": duration, "AvgMag": avg_mag})

        #move i after processed the whole group
        i = j
    else:
        i += 1

#Save to new CSV file
out_df = pd.DataFrame(processed_rows)
out_df.to_csv(OUTPUT_FILE, index=False)
print(f"Processed HIT events saved to {OUTPUT_FILE}")

try:
    with serial.Serial(COM_PORT, BAUD_RATE, timeout=1) as ser:
        print(f"Connected to {COM_PORT}")

        #Countdown before starting playback
        for sec in range(5, 0, -1):
            print(f"Starting in {sec}...")
            # if sec == 1:
                # os.startfile(VIDEO_FILE)
            time.sleep(1)
        # print("Go!")
        os.startfile(VIDEO_FILE)
        time.sleep(0.5)
        start_perf = time.perf_counter()

        #Send events from processed CSV
        for _, row in out_df.iterrows():
            delta = (row["Timestamp"] - start_time).total_seconds()

            #Wait until event time
            while (time.perf_counter() - start_perf) < delta:
                time.sleep(0.001)

            strength = get_strength(row["AvgMag"])
            if strength:
                message = f"{int(row['Duration']*1000)},{strength}"
                ser.write((message + "\n").encode())
                print(f"[{row['Timestamp'].time()}] Sent: {message}")

        print("Finished sending events.")

except serial.SerialException as e:
    print(f"Serial error: {e}")
except Exception as e:
    print(f"Unexpected error: {e}")