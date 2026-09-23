#include "arduino_client.hpp"

/* Link to Boost API documentation:
 * - https://www.boost.org/doc/libs/latest/doc/html/boost_asio/overview/serial_ports.html
 * */


/* note -- baudrate set automaticlly to 9600 in include/arduino_client.hpp */
ArduinoClient::ArduinoClient(boost::asio::io_context& io,
        const std::string& port,
        unsigned int baudrate)
    /* Need to initialize serialPort object before initializing ArduinoClient object */
    : serialPort(io, port) 
{
    (void) baudrate;
}

bool ArduinoClient::send_command(const json& message)
{
    (void) message;
    return false;
}

bool ArduinoClient::validate_digital_value(const unsigned int& v)
{
    (void) v;
    return false;
}

bool ArduinoClient::validate_pwm_value(const unsigned int& v)
{
    (void) v;
    return false;
}

bool ArduinoClient::digital_write(const unsigned int& pin, const unsigned int& value)
{
    (void) pin;
    (void) value;
    return false;
}

bool ArduinoClient::pwm_write(const unsigned int& pin, const unsigned int& value)
{
    (void) pin;
    (void) value;
    return false;
}

std::string ArduinoClient::read_analogue()
{
    return NULL;
}

/* note - timeout is set to 2 in arduino_client.hpp file */
bool ArduinoClient::shutdown(const bool& wait_ack, const unsigned int timeout)
{
    (void) wait_ack;
    (void) timeout;
    return false;
}

void ArduinoClient::reset()
{
}

