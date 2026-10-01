/**
 * PROGRAM NAME: arduino_test.cpp
 * PROGRAM DESCRIPTION: Program will read in script used for testing if
 * ardiuno_client.cpp is communicating correctly with esp32/arduino
 * DATE:  
 * SOURCES:
 * LAST CONTRIBUTOR: 
 * LAST CONTRIBUTOR NOTES:
 */

#include "arduino_client.hpp"
#include <string>
#include <unistd.h>

using std::string;
using std::vector;

int main() {
    boost::asio::io_context io;
    vector <unsigned int> digitalPins;
    vector <unsigned int> digitalVals;
    vector <unsigned int> pwmPins;
    vector <unsigned int> pwmVals;

    ArduinoClient arduino = ArduinoClient(io, "/dev/ttyUSB0", 9600);
    arduino.reset();

    digitalPins = {16, 17, 14, 23};
    digitalVals.assign(4, 1);
    pwmPins     = {26, 27, 13, 25};
    pwmVals.assign(4, 255);

    arduino.digital_write(digitalPins, digitalVals);
    arduino.pwm_write(pwmPins, pwmVals);

    sleep(1);
    
    digitalVals.assign(4, 0);
    pwmVals.assign(4, 0);

    arduino.digital_write(digitalPins, digitalVals);
    arduino.pwm_write(pwmPins, pwmVals);
    

    return 0;
}
