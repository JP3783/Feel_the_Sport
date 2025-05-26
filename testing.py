import serial
import time

ser = serial.Serial('COM7', 115200) 
time.sleep(2)  #Wait for connection

while True:
    #Step 1: Get current time
    laptop_send_time = time.time()
    message = f"TIMESTAMP:{laptop_send_time}\n"
    
    #Step 2: Send timestamp to watch
    ser.write(message.encode())

    #Step 3: Wait for response
    response = ser.readline().decode().strip()
    laptop_receive_time = time.time()

    print(f"Raw response: {response}")

    if response.startswith("LAPTOP:"):
        try:
            parts = response.split(',')
            sent_time = float(parts[0].split(':')[1])
            watch_time = int(parts[1].split(':')[1])

            round_trip = laptop_receive_time - sent_time
            print(f"\n📡 Sent: {sent_time}")
            print(f"⌚ Watch time (millis): {watch_time}")
            print(f"📥 Received: {laptop_receive_time}")
            print(f"⏱️ Round-trip delay: {round_trip:.6f} seconds\n")

        except Exception as e:
            print("Error parsing response:", e)

    time.sleep(2)