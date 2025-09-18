import serial

COM_PORT = 'COM8'  # Adjust if needed
BAUD_RATE = 115200

try:
    with serial.Serial(COM_PORT, BAUD_RATE, timeout=1) as ser:
        print(f"Connected to {COM_PORT}")
        print("Type 'exit' or 'quit' to end")
        while True:
            user_input = input("Enter command (e.g., 500,Low): ")
            if user_input.lower() in ["exit", "quit"]:
                break
            ser.write((user_input + '\n').encode())
except serial.SerialException as e:
    print(f"Serial error: {e}")
except Exception as e:
    print(f"Unexpected error: {e}")