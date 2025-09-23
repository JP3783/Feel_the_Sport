import serial
import time

COM_PORT = 'COM7'  # Adjust if needed
BAUD_RATE = 115200

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

            # #wait for ACK
            # ack = ser.readline().decode().strip()
            # end = time.perf_counter()

            # if ack == "ACK":
            #     latency_ms = (end - start) *1000
            #     print(f"ACK received in {latency_ms:.2f} ms")
            # else:
            #     print("No ACK or invalid response:", ack)
except serial.SerialException as e:
    print(f"Serial error: {e}")
except Exception as e:
    print(f"Unexpected error: {e}")