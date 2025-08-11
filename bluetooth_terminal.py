import serial
import csv
import time

COM_PORT = 'COM7'     #Bluetooth serial port - adjust if needed
                      #How? Upload arduino program and pair watch with laptop. See Device Manager for COM Ports
BAUD_RATE = 115200

CSV_FILENAME = "tennis_hits.csv"
DURATION_SECONDS = 60


try:
    with serial.Serial(COM_PORT, BAUD_RATE, timeout=1) as ser:
        print(f"Listening on {COM_PORT} at {BAUD_RATE} baud...")

        #Set up csv file
        with open(CSV_FILENAME, mode="w", newline="") as file:
            writer = csv.writer(file)
            writer.writerow(["Time", "X", "Y", "Z", "Mag", "Notes"])

            start_time = time.time()

            #Read and save for one minute
            while time.time() - start_time < DURATION_SECONDS:
                if ser.in_waiting:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        print(f"[WATCH] {line}")

                        #try to parse accelerometer data if they exist
                        try:
                            parts = line.replace("X:", "").replace("Y:", "").replace("Z:", "").replace("Mag:", "").split()
                            x, y, z, mag = map(float, parts[:4])
                            notes = "HIT" if "HIT" in line else ""
                            writer.writerow([time.strftime("%H:%M:%S"), x, y, z, mag, notes])
                        except:
                            #skip lines that don't match expected format
                            pass
        print(f"Data saved to {CSV_FILENAME}")


except serial.SerialException as e:
    print(f"Serial error: {e}")
except Exception as e:
    print(f"Unexpected error: {e}")
