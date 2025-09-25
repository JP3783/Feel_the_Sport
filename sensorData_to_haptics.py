import pandas as pd
import serial
import time
import cv2
import os


COM_PORT = 'COM7'   #Adjust if needed
BAUD_RATE = 115200
CSV_FILE = "tennis_hits_519_video.csv"
VIDEO_FILE = "tennis_hits_519_video_TRIM.mp4"
# video_path = os.path.abspath(VIDEO_FILE)
check = False

# print(f"[DEBUG] Full video path: {video_path}")

# print(cv2.getBuildInformation())


#Duration of buzz per event (ms)
BUZZ_DURATION_MS = 200  

def mag_to_strength(mag):
    """Map magnitude to Low/Med/High vibration strength."""
    if mag >= 3.0:
        return "High"
    elif mag >= 2.0:
        return "Med"
    else:
        return "Low"

try:
    #Load CSV
    df = pd.read_csv(CSV_FILE)
    # df = df.sort_values("Time").reset_index(drop=True)

    #Keep only events with magnitude >= 2
    # df = df[df["Mag"] >= 2]

    #open serial port
    try:
        ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
        print(f"Connected to {COM_PORT}")
    except serial.SerialException as exception:
        print(f"Serial error: {exception}")
        ser = None

    # print(f"ser: {ser}")

    #open video
    cap = cv2.VideoCapture(VIDEO_FILE)
    # print("[DEBUG] Current working directory:", os.getcwd())
    # if not cap.isOpened():
    #     print(f"[ERROR] Failed to open video file: {VIDEO_FILE}")
    #     print("       Double-check the path and file format.")
    #     exit(1)
    # else:
    #     print(f"[INFO] Successfully opened video: {VIDEO_FILE}")
    
    if not cap.isOpened():
        print(f"[ERROR] Failed to open video: {VIDEO_FILE}")
        print("Double-check the path and file format.")
        exit(1)
    else:
        print("[INFO] Video opened successfully.")


    #start playback timer
    start_time = time.perf_counter()

    #track events
    event_index = 0
    total_events = len(df)
    print(f"Playing video and listening for {total_events} vibration events...")

    # if cap.isOpened():
    #     check = True
    # elif not cap.isOpened():
    #     check = False

    # print(f"check: {check}")

    # print(f"cap.isOpened: {cap.isOpened}")
    # print(f"cap.isOpened(): {cap.isOpened()}")

    while cap.isOpened():
        # print(f"it reaches here bao")
        ret, frame = cap.read()
        # print(f"ret: {ret}")
        # print(f"frame: {frame}")
        # print(f"cap.read(): {cap.read()}")
        if not ret:
            break
        cv2.imshow("Tennis Session", frame)

        #calculate elapsed time
        current_time = time.perf_counter() - start_time
        print(f"current time: {current_time}")

        #send vibration IF it's time
        while event_index < total_events and df.loc[event_index, "Time"] <= current_time:
            mag = df.loc[event_index, "Mag"]
            strength = mag_to_strength(mag)
            command = f"{BUZZ_DURATION_MS},{strength}"
            
            if ser:
                ser.write((command + "\n").encode())
                ack = ser.readline().decode().strip()
                print(f"[{current_time:.2f}s] Sent: {command} | ACK: {ack}")
            else:
                print(f"[{current_time:.2f}s] Simulated: {command} (no serial connection)")
            
            event_index += 1

        #exit if q is pressed
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    #do a clean up
    cap.release()
    cv2.destroyAllWindows()
    if ser:
        ser.close()
        print(f"Serial connection closed.")
except serial.SerialException as e:
    print(f"Serial error: {e}")
except Exception as e:
    print(f"Unexpected error: {e}")
