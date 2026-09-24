/**
 * PROGRAM NAME: arduino_client
 * PROGRAM DESCRIPTION: Communicates with a connected arduino/esp32 microcontroller
 * connected to the inputted port when constructing an object of the class.
 * DATE: 9/24/2026 
 * SOURCES:
 * - https://www.boost.org/doc/libs/latest/doc/html/boost_asio/overview/serial_ports.html
 * - https://www.geeksforgeeks.org/cpp/chrono-in-c/
 *
 * LAST CONTRIBUTOR: James Ogle
 * LAST CONTRIBUTOR NOTES:
 * - check for bug in shutdown() to make sure it breaks when it as waited longer then 
 *   the set amount
 * - when a value being assigned to a pin is invalid, it is being skipped rather than 
 *   causing the program to be skipped complelety
 *   - mostly likely should log this on stdout or stderr
 */

#include "arduino_client.hpp"
#include <boost/asio/buffer.hpp>
#include <boost/asio/impl/read.hpp>
#include <chrono>
#include <thread>

namespace chrono = std::chrono;
using nlohmann::json;
using std::string;
using std::vector;

/* note -- baudrate set automaticlly to 9600 in include/arduino_client.hpp */
ArduinoClient::ArduinoClient(boost::asio::io_context& io,
        const string& port,
        unsigned int baudrate)
    : serialPort(io)
{
    try {
        /* Opening connection to specified port and setting baudrate of it */
        serialPort.open(port);
        serialPort.set_option(
                boost::asio::serial_port_base::baud_rate(baudrate)
                );

        /* wait 2 seconds for Arduino/esp32 to reset */
        std::this_thread::sleep_for(chrono::seconds(2));
    }
    catch (const boost::system::system_error& e) {
        /**
         * did not connect properly to arduino, boost class will return
         * specific error for why it failed
         */
        fprintf(stderr, "ERROR: %s", e.what());
    }
}

bool ArduinoClient::send_command(const json& message)
{
    try {
        string buffer;
        unsigned int bufSize;

        /**
         * converting json object to string then writing sending to
         * connected device on port
         * */
        buffer = message.dump();
        bufSize = buffer.size() + 8;
        boost::asio::write(serialPort, boost::asio::buffer(buffer, bufSize));
    }
    catch (const boost::system::system_error& e) {
        fprintf(stderr, "ERROR: %s", e.what());
        return false;
    }
    return true;
}

bool ArduinoClient::validate_digital_value(const unsigned int& v)
{
    if (v != 0 || v != 1) {
        fprintf(stderr,"digital_write value must be 0 or 1, got %d\n", v);
        return false;
    }
    return false;
}

bool ArduinoClient::validate_pwm_value(const unsigned int& v)
{
    /* unsigned already makes sure its not negative */
    if (v > 255) {
        fprintf(stderr,"pwm_write value must be an integer 0–255, got %d\n", v);
        return false;
    }
    return true;
}

bool ArduinoClient::digital_write(const vector<unsigned int>& pin, const vector<unsigned int>& value)
{
    int i;
    json message;
    string buf;

    /* first check if lengths are the same */
    if (pin.size() != value.size()) {
        fprintf(stderr,"Pins and values must have same length\n");
        fprintf(stderr,"%4spin size: %ld, value size: %ld\n", "", pin.size(), value.size());
        return false;
    }

    /**
     * only need to check if one is empty since we already know at this point they are
     * the same size
     */
    if (pin.empty()) {
        fprintf(stderr,"Pin and Value vectors must both have values in them\n");
        return false;
    }

    for(i = 0; i < (int) pin.size(); i++) {
        /**
         * Creating a json message to send, this code creates a messaage format like:
         *  {
         *      "type": "D",
         *      "pin": pin[i],
         *      "value": value[i]
         *  }
         */

        /* if value is invalid, now just skipping it */
        if (!validate_digital_value(value[i])) continue;
        message = {
            {"type", "D"},
            {"pin", pin[i]},
            {"value", value[i]}
        };

        /**
         * to access the values if using the same nlohmann/json library, you would do:
         * - message[type] will return "D" in this case 
         *
         */ 
       
        /* simply going to send the message */
        send_command(message);
    }

    return true;
}

bool ArduinoClient::pwm_write(const vector<unsigned int>& pin, const vector<unsigned int>& value)
{
    int i;
    json message;
    string buf;

    /* first check if lengths are the same */
    if (pin.size() != value.size()) {
        fprintf(stderr,"Pins and values must have same length\n");
        return false;
    }


    for(i = 0; i < (int) pin.size(); i++) {
        if(!validate_pwm_value(value[i])) continue;

        /* To understand how json formatting works here, go to method digital_write */
        message = {
            {"type", "D"},
            {"pin", pin[i]},
            {"value", value[i]}
        };
      
        /* simply going to send the message */
        send_command(message);
    }

    return true;
}

string ArduinoClient::read_analogue()
{
    string lines;
    /**
     * boost::asio creates a object that dynamically allocates itself
     * - using this to read from the port 
     * - NOTE: data will be read into 'lines' variable
     */
    boost::asio::dynamic_string_buffer buf(lines); 
    json cmd;
    size_t bytesRead;
    
    cmd = {
        {"type", "R"}
    };

    /* Telling esp32/arduino to print out pins & values on stdout */
    send_command(cmd);

    /**
     * Wait for esp32/arduino to receive before immediately start trying to retrieve data
     * - time: 0.02 seconds
     */
    std::this_thread::sleep_for(chrono::milliseconds(200));


    /* read from the serial port until esp32/arduino is done writing */
    bytesRead = boost::asio::read_until(serialPort, buf, '\n');

    if (bytesRead <= 0) {
        fprintf(stderr,"no bytes were read on read_analogue call\n");
        return NULL;
    }
    
    return lines;
}

/* note - timeout is set to 2 in arduino_client.hpp file */
bool ArduinoClient::shutdown(const bool& wait_ack, const unsigned int& timeout)
{
    json cmd;
    string data;
    /* NOTE: data allocated for by 'buf' will be read into 'data'  */
    boost::asio::dynamic_string_buffer buf(data);
    chrono::seconds time;
    chrono::time_point<chrono::steady_clock> start;

    cmd = {
        {"type", "R"}
    };
    send_command(cmd);

    /**
     * Storing the timeout value into chrono object so it can be compared to the other
     * chrono objects in the while condition.
     */
    time = chrono::seconds(timeout);


    if (wait_ack) {
        start = chrono::steady_clock::now();

        /**
         * Waiting for the arduino/esp32 to shutdown
         * - this function will return false if it doesn't send acceptable
         *   message within the alloted "timeout" time
         *
         * TODO: check if this properly ends after trying to read for set amount of time
         */
        while( (chrono::steady_clock::now() - start) < time) {
            boost::asio::read_until(serialPort, buf, '\n');

            /**
             * Checking if esp32/arduino sent "SHUTDOWN" or "OK", both are acceptable
             * statements that it was successful
             * - npos -> out of bounds string position
             */
            if (data.find("SHUTDOWN") != string::npos ||
                    data.find("OK") != string::npos) {
                return true;
            }
        }
    
        
    }

    /**
     * if it does not return in the while statement when 'wait_ack' is true,
     * then the esp32/arduino did not shutdown properly
     */
    return false;
}

void ArduinoClient::reset()
{
    json cmd;

    cmd = {
        {"type", "RESET"}
    };

    send_command(cmd);
}

