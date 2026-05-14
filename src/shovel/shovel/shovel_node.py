import RPi.GPIO as GPIO
import rclpy
from rclpy.node import Node
from . import constants, arduino_client, calculations
from std_msgs.msg import String, Float32


class shovel_node(Node):
    def __init__(self):
        super().__init__('shovel_node')

        self.arduino = arduino_client.ArduinoClient(port="/dev/ttyUSB0")

        self.arduino.reset()

        self.arm_publisher_ = self.create_publisher(Float32, 'arm_angle', 10)
        self.scoop_publisher_ = self.create_publisher(Float32, 'scoop_angle', 10)
        self.arm_subscription = self.create_subscription(
                String,
                '/shovel/arm_cmd',
                self.arm_callback,
                10)
        self.scoop_subscription = self.create_subscription(
                String,
                '/shovel/scoop_cmd',
                self.scoop_callback,
                10)
        self.bucket_subscription = self.create_subscription(
                String,
                '/shovel/bucket_cmd',
                self.bucket_callback,
                10)

        self.bucket_subscription  # prevent unused variable warning
        self.arm_subscription  # prevent unused variable warning

    def arm_callback(self, msg):
        feedback = self.arduino.read_analog();
        if (len(feedback) >= 3) and feedback[0] == 'A':
            arm_diff = int(feedback[1]) - int(feedback[3])
            compensation_value = max(-abs(arm_diff) * constants.ARM_P + constants.ARM_DUTY_CYCLE, 50)
            arm_angle = calculations.angle_from_extension((int(feedback[1]) + int(feedback[3])) / 2 * constants.ARM_INCHES_PER_BIT)
            self.arm_publisher_.publish(Float32(data=(arm_angle)))
        else:
            arm_diff = 0
            compensation_value = constants.ARM_DUTY_CYCLE
        if(bool(arm_diff > 0) ^ bool(msg.data == constants.ARM_BACKWARD)):
            pwm_values = [compensation_value, constants.ARM_DUTY_CYCLE]
        else:
            pwm_values = [constants.ARM_DUTY_CYCLE, compensation_value]
        
        if(msg.data == constants.ARM_FORWARD):
            self.arduino.digital_write(constants.ARM_DIR, [1, 1]);
        elif(msg.data == constants.ARM_BACKWARD):
            self.arduino.digital_write(constants.ARM_DIR, [0, 0]);
        else:
            pwm_values = [0, 0];
        self.arduino.pwm_write(constants.ARM_PWM, pwm_values);

    def bucket_callback(self, msg):
        if(msg.data == constants.BUCKET_FORWARD):
            self.arduino.pwm_write(constants.BUCKET_PWM, constants.BUCKET_DUTY_CYCLE);
            self.arduino.digital_write(constants.BUCKET_DIR, 1);
        elif(msg.data == constants.BUCKET_BACKWARD):
            self.arduino.pwm_write(constants.BUCKET_PWM, constants.BUCKET_DUTY_CYCLE);
            self.arduino.digital_write(constants.BUCKET_DIR, 0);
        else:
            self.arduino.pwm_write(constants.BUCKET_PWM, 0)

    def scoop_callback(self, msg):
        if(msg.data == constants.SCOOP_FORWARD):
            self.arduino.pwm_write(constants.SCOOP_PWM, constants.SCOOP_DUTY_CYCLE);
            self.arduino.digital_write(constants.SCOOP_DIR, 0);
        elif(msg.data == constants.SCOOP_BACKWARD):
            self.arduino.pwm_write(constants.SCOOP_PWM, constants.SCOOP_DUTY_CYCLE);
            self.arduino.digital_write(constants.SCOOP_DIR, 1);
        else:
            self.arduino.pwm_write(constants.SCOOP_PWM, 0);

def main(args=None):
    rclpy.init(args=args)

    shovelNode = shovel_node()

    rclpy.spin(shovelNode)

    # Destroy the node explicitly
    # (optional - otherwise it will be done automatically
    # when the garbage collector destroys the node object)
    shovelNode.destroy_node()


if __name__ == '__main__':
    main()
