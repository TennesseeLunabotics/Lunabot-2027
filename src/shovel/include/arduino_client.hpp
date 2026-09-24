#include <boost/asio/serial_port.hpp>
#include <ctime>
#include <boost/asio.hpp> /* serial socket communication */
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include "nlohmann/json.hpp"

/**
 * Documentation Example
 * @brief Calculates the area of a rectangle.
 * 
 * Detailed description goes here. You can use **bold text**,
 * `code blocks`, or lists to explain complex logic.
 * 
 * @param width The width of the rectangle in centimeters.
 * @param height The height of the rectangle in centimeters.
 * @return The total calculated area.
 * @note Ensure both inputs are positive numbers.
 * @see Circle::calculateArea
 */

class ArduinoClient {
    public:
        /**
         * @brief Initializes the ArduinoClient object.
         * @param io holds the input/output stream context for what the serial port
         *           will register on after reading in the port string
         * @param port string name of the port the ArduinoClient object is connecting to
         * @param baudrate controls transmission speed, receiver timing
         *                 (how often to sample voltage), and ensures sender and receiver
         *                 are on the same "tempo"
         *                 - DEFAULT: 9600
         */
        ArduinoClient(boost::asio::io_context& io,
                const std::string& port,
                unsigned int baudrate = 9600);

        /**
         * @brief sends a command to arduino/eps32 in json format
         * @param message Message to send to arduino/eps32 formatted in json 
         * @return bool where true is success and vice versa
         */
        bool send_command(const nlohmann::json& message);

        /**
         * @brief writes value to the digital pins on the connected device. The
         * lenght of the pins and value list must be the same
         * @param pin pin number to write to
         * @param value list of values be written to certain number of pins
         * @return bool where true is success and vice versa
         */
        bool digital_write(const std::vector<unsigned int>& pin, const std::vector<unsigned int>& value);

        /**
         * @brief writes value to the PWM pins on the connected device
         * @param pin pin number number to write to
         * @param value value writing to pin
         */
        bool pwm_write(const std::vector<unsigned int>& pin, const std::vector<unsigned int>& value);

        /**
         * @brief sends command to the connected device and receives the
         *        pin values on the device. The function recieves the values
         *        by reading them on stdin.
         * @return the value of the pins in a string
         */
        std::string read_analogue ();

        /**
         * @brief shutdown the connected device and waits for 'timeout' seconds
         *        for the connected device to print its shutdown. Note this doesn't
         *        disconnect you from the device, only throws the device in a shutdown
         *        state. This prevents you from reading/writing to it.
         * @param wait_ack telling function whether to wait or not
         *                 (TRUE -- wait, FALSE -- don't wait)
         *                 - DEFAULT: false
         * @param timeout amount of time function should wait for shutdown reponse
         *        before return TRUE/FALSE
         *        - DEFAULT: 2
         * @return returns TRUE if connected devices returns shutdown response
         *         within expected timeout time and FALSE if it does not.
         */
        bool shutdown(const bool& wait_ack = false, const unsigned int& timeout = 2);

        /**
         * @brief tells device to no longer be reading as "shutdown".
         *        Important when you need to start read/write to device again
         *        after shutting down. 
         */
        void reset ();

    private:
        /**
         * @brief Main Variable used for handling communication signals with connected device
         *        Note this is an object with no defined constructor. It will need to be defined before
         *        ArduinoClient constructor body can run. Refer to arduino_client.cpp to see how this is
         *        accomplished.
         */
        boost::asio::serial_port serialPort;

        /**
         * @brief Validating the value written to digital pins on the connected device.
         *        Value range: 0 <= x <= 255
         * @param Value being written to digital pin that needs to be validated 
         * @return returns true if value is good, false if not
         */
        bool validate_digital_value(const unsigned int& v);

        /**
         * @brief Validating the value written to the PWM pins on the connected device
         *        Value range: 0 <= x <= 255
         * @param Value being written to PWM pin that needs to be verified 
         * @return returns true if value is good, false if not
         */
        bool validate_pwm_value(const unsigned int& v);
};
