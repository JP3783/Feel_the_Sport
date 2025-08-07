import serial

COM_PORT = 'COM7'     #Bluetooth serial port - adjust if needed
                      #How? Upload arduino program and pair watch with laptop. See Device Manager for COM Ports
BAUD_RATE = 115200

try:
    with serial.Serial(COM_PORT, BAUD_RATE, timeout=1) as ser:
        print(f"Listening on {COM_PORT} at {BAUD_RATE} baud...")
        while True:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(f"[WATCH] {line}")
except serial.SerialException as e:
    print(f"Serial error: {e}")
except Exception as e:
    print(f"Unexpected error: {e}")
