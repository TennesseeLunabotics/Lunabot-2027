#include <sys/socket.h>
#include <JetsonGPIO.h>
#include "nlohmann/json.hpp"

using nlohmann::json;

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
         * AF_INET     -- IPv4 protocol (domain)
         * SOCK_STREAM -- TCP socket    (type connection)
         * 0           --               (protocol)
         **/
        ArduinoClient(int port, int baudrate)  {
            int serverSocket;
            bool wait_ack = false;

            baudrate = 9600;
            serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        }

    private:
        /**
         * @brief Sends message to esp32, view esp32.ino for expected formatting
         * @param message Message to send to esp32 formatted in json 
         * @return
         **/
        bool send_command (json message);
        /**
         * @brief Sends message to esp32, view esp32.ino for expected formatting
         * @param message Message to send to esp32 formatted in json 
         * @return
         **/
        bool _validate_digital_value (int v);
        /**
         * @brief Sends message to esp32, view esp32.ino for expected formatting
         * @param message Message to send to esp32 formatted in json 
         * @return
         **/
        bool _validate_pwm_value (int v);
        /**
         * @brief Sends message to esp32, view esp32.ino for expected formatting
         * @param message Message to send to esp32 formatted in json 
         * @return
         **/
        bool digital_write (int pin, int value);
        /**
         * @brief Sends message to esp32, view esp32.ino for expected formatting
         * @param message Message to send to esp32 formatted in json 
         * @return
         **/
        void pwm_write (int pin, int value);
        /**
         * @brief Sends message to esp32, view esp32.ino for expected formatting
         * @param message Message to send to esp32 formatted in json 
         * @return
         **/
        void read_analogue ();
        /**
         * @brief Sends message to esp32, view esp32.ino for expected formatting
         * @param message Message to send to esp32 formatted in json 
         * @return
         **/
        void shutdown (bool wait_ack, int timeout = 2);
        /**
         * @brief Sends message to esp32, view esp32.ino for expected formatting
         * @param message Message to send to esp32 formatted in json 
         * @return
         **/
        void reset ();
};
 
int main() {
    return 0;
}
