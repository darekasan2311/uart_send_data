import serial
import time

# Open serial port
ser = serial.Serial('/dev/ttyACM0', 460800, timeout=1)
time.sleep(2)  # Wait for connection to establish

print("Sending messages to ESP32...")
while True:
    # if ser.in_waiting > 0:
    #     line = ser.readline().decode('utf-8').rstrip()
    #     print(f"Received: {line}")
    #     time.sleep(1)
    ser.write(b'Hello from Linux\n')
    time.sleep(0.1)
ser.close()
