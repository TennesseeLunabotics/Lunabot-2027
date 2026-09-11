from arduino_client import ArduinoClient

client = ArduinoClient(port="/dev/ttyUSB0")
client.reset()
feedback = client.read_analog()
feedback = client.read_analog()
print(feedback)
print(feedback[0] == 'A')
print(feedback[1])
print(feedback[2])
print(feedback[3])
exit()
while(True):
    feedback = client.read_analog()
    compensation_value = 0
    
    if (len(feedback) >= 3) and feedback[0] == 'A':
        arm_diff = int(feedback[1]) - int(feedback[3])
        compensation_value = abs(arm_diff) * constants.ARM_P + constants.ARM_DUTY_CYCLE
    #print(compensation_value)
