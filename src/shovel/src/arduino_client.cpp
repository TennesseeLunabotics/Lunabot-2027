#include "arduino_client.hpp"

ArduinoClient::ArduinoClient(int port, int baudrate)  {
    (void) port;
    (void) baudrate;
}

bool ArduinoClient::send_command(json message) {
    (void) message;
    return false;
}

bool ArduinoClient::validate_digital_value(int v) {
    (void) v;
    return false;
}

bool ArduinoClient::validate_pwm_value(int v) {
    (void) v;
    return false;
}

bool ArduinoClient::digital_write(int pin, int value) {
    (void) pin;
    (void) value;
    return false;
}

void ArduinoClient::pwm_write(int pin, int value) {
    (void) pin;
    (void) value;
}

void ArduinoClient::read_analogue() {
}

/* note - timeout is set to 2 in arduino_client.hpp file */
void ArduinoClient::shutdown(bool wait_ack, int timeout) {
    (void) wait_ack;
    (void) timeout;
}

void ArduinoClient::reset() {
}

