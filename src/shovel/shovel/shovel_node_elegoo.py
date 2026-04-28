import serial
import time

class ArduinoClient:
    def __init__(self, port, baudrate=9600):
        self.ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2)  # wait for Arduino reset

    def send_command(self, cmd):
        self.ser.write((cmd + "\n").encode())

    def digital_write(self, pin, value):
        # value: 0 or 1
        self.send_command(f"D,{pin},{value}")

    def pwm_write(self, pin, value):
        # value: 0–255
        self.send_command(f"P,{pin},{value}")

    def read_analog(self):
        self.send_command("R")
        time.sleep(0.1)

        lines = []
        while self.ser.in_waiting:
            line = self.ser.readline().decode().strip()
            lines.append(line)

        return lines


# Example usage
if __name__ == "__main__":
    arduino = ArduinoClient(port="/dev/ttyACM0")  # change to your port (e.g., "/dev/ttyUSB0")

    # Turn pin 4 ON
    arduino.digital_write(4, 1)

    # Set PWM on pin 5
    arduino.pwm_write(5, 128)

    # Read analog values
    for i in range(200):
        data = arduino.read_analog()
        print("Raw response:", data)
        time.sleep(.01)
    arduino.pwm_write(5,0)


