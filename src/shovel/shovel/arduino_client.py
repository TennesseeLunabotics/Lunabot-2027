import serial
import time

class ArduinoClient:
    def __init__(self, port, baudrate=9600):
        self.ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2)  # wait for Arduino reset

    def send_command(self, cmd):
        self.ser.write((cmd + "\n").encode())

    def _validate_digital_value(self, v):
        if v not in (0, 1):
            raise ValueError(f"digital_write value must be 0 or 1, got {v}")


    def _validate_pwm_value(self, v):
        if not isinstance(v, int) or not (0 <= v <= 255):
            raise ValueError(f"pwm_write value must be an integer 0–255, got {v}")

    def digital_write(self, pin, value):
        if isinstance(pin, (list, tuple)):
            if not isinstance(value, (list, tuple)):
                raise ValueError("Value must be a list/tuple when pin is multiple")

            if len(pin) != len(value):
                raise ValueError("Pins and values must have same length")

            for p, v in zip(pin, value):
                self._validate_digital_value(v)
                self.send_command(f"D,{p},{v}")
        else:
            self._validate_digital_value(value)
            self.send_command(f"D,{pin},{value}")


    def pwm_write(self, pin, value):
        if isinstance(pin, (list, tuple)):
            if not isinstance(value, (list, tuple)):
                raise ValueError("Value must be a list/tuple when pin is multiple")

            if len(pin) != len(value):
                raise ValueError("Pins and values must have same length")

            for p, v in zip(pin, value):
                self._validate_pwm_value(v)
                self.send_command(f"P,{p},{v}")
        else:
            self._validate_pwm_value(value)
            self.send_command(f"P,{pin},{value}")

    def read_analog(self):
        self.send_command("R")
        # time.sleep(0.1)

        lines = []
        while self.ser.in_waiting:
            line = self.ser.readline().decode().strip()
            lines.append(line)
        return lines
    
    def shutdown(self, wait_ack=False, timeout=2):
        self.send_command("S")
        if wait_ack:
            start = time.time()
            while time.time() - start < timeout:
                if self.ser.in_waiting:
                    line = self.ser.readline().decode().strip()
                    if line in ("OK", "SHUTDOWN"):
                        return True
        return False
    
    def reset(self):
        self.send_command("RESET")


if __name__ == "__main__":
    arduino = ArduinoClient(port="/dev/ttyUSB0")
    arduino.reset()

    digitalPins = [16, 17, 14, 23]
    pwmPins = [26, 27, 13, 25]

    while True:
        for i in range(4):
            arduino.digital_write(digitalPins[i], 1)
            arduino.pwm_write(pwmPins[i], 255)
            time.sleep(0.5)
            arduino.digital_write(digitalPins[i], 0)
            arduino.pwm_write(pwmPins[i], 0)