import serial
import time
import json

class ArduinoClient:
    def __init__(self, port, baudrate=9600):
        self.ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2)  # wait for Arduino reset

    def send_command(self, cmd:dict):
        msg = json.dumps(cmd)
        self.ser.write((msg + "\n").encode())

    def _validate_digital_value(self, v):
        if v not in (0, 1):
            raise ValueError(f"digital_write value must be 0 or 1, got {v}")


    def _validate_pwm_value(self, v):
        if not (0 <= v <= 255):
            raise ValueError(f"pwm_write value must be an integer 0–255, got {v}")

    def digital_write(self, pin, value):
        if isinstance(pin, (list, tuple)):
            if not isinstance(value, (list, tuple)):
                raise ValueError("Value must be a list/tuple when pin is multiple")

            if len(pin) != len(value):
                raise ValueError("Pins and values must have same length")

            for p, v in zip(pin, value):
                self._validate_digital_value(v)
                cmd = {"type": "D", "pin": p, "value": v}
                self.send_command(cmd)
        else:
            self._validate_digital_value(value)
            cmd = {"type": "D", "pin": pin, "value": value}
            self.send_command(cmd)


    def pwm_write(self, pin, value):
        if isinstance(pin, (list, tuple)):
            if not isinstance(value, (list, tuple)):
                raise ValueError("Value must be a list/tuple when pin is multiple")

            if len(pin) != len(value):
                raise ValueError("Pins and values must have same length")

            for p, v in zip(pin, value):
                self._validate_pwm_value(v)
                cmd = {"type": "D", "pin": p, "value": v}
                self.send_command(cmd)
        else:
            self._validate_pwm_value(value)
            cmd = {"type": "D", "pin": pin, "value": value}
            self.send_command(cmd)

    def read_analog(self):
        cmd = {"type": "R"}
        self.send_command(cmd)
        time.sleep(0.02)

        lines = []
        while self.ser.in_waiting:
            line = self.ser.readline().decode().strip()
            lines.append(line)
        if(len(lines) > 0):
            lines = lines[0].split(",")
        return lines
    
    def shutdown(self, wait_ack=False, timeout=2):
        cmd = {"type": "S"}
        self.send_command(cmd)
        if wait_ack:
            start = time.time()
            while time.time() - start < timeout:
                if self.ser.in_waiting:
                    line = self.ser.readline().decode().strip()
                    if line in ("OK", "SHUTDOWN"):
                        return True
        return False
    
    def reset(self):
        cmd = {"type": "RESET"}
        self.send_command(cmd)


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
