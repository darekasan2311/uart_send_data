import serial
import time
import argparse

def read_args():
    parser = argparse.ArgumentParser(
        description='Send commands to ESP32 via Serial')

    parser.add_argument('-m',
                        '--mode',
                        default='ON',
                        help='LED mode (ON/OFF)',
                        choices=['ON', 'OFF']
                        )

    parser.add_argument('-f',
                        '--freq',
                        type=int,
                        default=0,
                        help='LED blink frequency (default: 0)'
                        )

    args = parser.parse_args()
    return args

def main():
    port = '/dev/ttyACM0'
    baud_rate = 460800
    ser = None

    args = read_args()

    try:
        # Open serial port
        ser = serial.Serial(port, baud_rate, timeout=1)
        # time.sleep(1)  # Wait for connection to establish

        print("Sending message to ESP32...")

        if args.mode == 'OFF':
            args.freq = 0
        else:
            args.freq = abs(args.freq)

        message = f"{args.mode},{args.freq}\n"

        ser.write(message.encode('utf-8'))
        print(f"Sent: {message.strip()}")

    except serial.SerialException as e:
        print(f"Serial port error: {e}")
        print("Check if:")
        print("  - Device is connected")
        print("  - Port /dev/ttyACM0 exists")
        print("  - You have permissions (try: sudo usermod -a -G dialout $USER)")
        
    except PermissionError:
        print("Permission denied accessing /dev/ttyACM0")
        print("Run: sudo chmod 666 /dev/ttyACM0")
        print("Or add user to dialout group: sudo usermod -a -G dialout $USER")

    except KeyboardInterrupt:
        print("\nExiting.")

    finally:
        if ser is not None and ser.is_open:
            print("Close serial port.")
            ser.close()


if __name__ == "__main__":
    main()
