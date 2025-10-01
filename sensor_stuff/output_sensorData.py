import serial
import csv
import time
from datetime import datetime

COM_PORT = 'COM6'     #Bluetooth serial port - adjust if needed            COM6 for watch 1                 COM6 for watch 2
                      #How? Upload arduino program and pair watch with laptop. See Device Manager for COM Ports
BAUD_RATE = 115200

CSV_FILENAME = "tennisHits21025.csv"
DURATION_SECONDS = 10 #300
DELAY_SECONDS = 5 #delay before start writing to csv

try:
    # stopwatch_start = time.time()

    with serial.Serial(COM_PORT, BAUD_RATE, timeout=1) as ser:
        print(f"Listening on {COM_PORT} at {BAUD_RATE} baud...")
        print(f"Waiting {DELAY_SECONDS} seconds before logging starts...")

        #show countdown
        for i in range(DELAY_SECONDS, 0, -1):
            print(f"Starting in {i}...")
            time.sleep(1)

        stopwatch_start = time.time()

        #Set up csv file
        with open(CSV_FILENAME, mode="w", newline="") as file:
            writer = csv.writer(file)
            writer.writerow(["Time", "X", "Y", "Z", "Mag", "Notes"])

            logging_start_time  = time.time()

            #Read and save for DURATION_SECONDS
            while time.time() - logging_start_time  < DURATION_SECONDS:
                if ser.in_waiting:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        print(f"{datetime.now().strftime("%H:%M:%S.%f")[:-3]} {line}")

                        #try to parse accelerometer data if they exist
                        try:
                            parts = line.replace("X:", "").replace("Y:", "").replace("Z:", "").replace("Mag:", "").split()
                            x, y, z, mag = map(float, parts[:4])
                            notes = "HIT" if mag >= 2 else ""
                            writer.writerow([datetime.now().strftime("%H:%M:%S.%f")[:-3], x, y, z, mag, notes])
                        except:
                            #skip lines that don't match expected format
                            pass
        print(f"Data saved to {CSV_FILENAME}")
        
        stopwatch_end = time.time()
        elapsed_time = stopwatch_end - stopwatch_start
        print(f"Elapsed time: {elapsed_time:.2f} seconds")
except serial.SerialException as e:
    print(f"Serial error: {e}")
except Exception as e:
    print(f"Unexpected error: {e}")
