import pandas as pd
import serial
import time


COM_PORT = 'COM7'   #Adjust if needed
BAUD_RATE = 115200
CSV_FILE = "tennis_hits.csv"

#Duration of buzz per event (ms)
BUZZ_DURATION_MS = 200  

def mag_to_strength(mag):
    """Map magnitude to Low/High vibration strength."""
    if mag >= 3:
        return "High"
    else:
        return "Low"

try:
    #Load CSV
    df = pd.read_csv(CSV_FILE)

    #Keep only events with magnitude >= 2
    events = df[df["Mag"] >= 2]

    print(f"Found {len(events)} events with Mag >= 2")

    #Open serial connection
    with serial.Serial(COM_PORT, BAUD_RATE, timeout=1) as ser:
        print(f"Connected to {COM_PORT}")

        for _, row in events.iterrows():
            strength = mag_to_strength(row["Mag"])
            command = f"{BUZZ_DURATION_MS},{strength}"

            start = time.perf_counter()
            ser.write((command + "\n").encode())

            #wait for ACK
            ack = ser.readline().decode().strip()
            end = time.perf_counter()

            if ack == "ACK":
                latency_ms = (end - start) * 1000
                print(f"Sent: {command} | ACK in {latency_ms:.2f} ms")
            else:
                print(f"Sent: {command} | No ACK (got '{ack}')")

            time.sleep(0.1)  #small pause between commands

except serial.SerialException as e:
    print(f"Serial error: {e}")
except Exception as e:
    print(f"Unexpected error: {e}")
