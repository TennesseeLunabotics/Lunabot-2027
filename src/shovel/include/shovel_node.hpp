/**
 * PROGRAM NAME: shovel_node
 * PROGRAM DESCRIPTION: Header file for shovel_node.cpp.
 * DATE: 10/15/2026
 * SOURCES:
 *
 * LAST CONTRIBUTOR: James Ogle
 * LAST CONTRIBUTOR NOTES:
 */

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include "arduino_client.hpp"

class ShovelNode : public rclcpp::Node {
    public:
        /**
         * @brief Constructor for ShovelNode Class
         * @parameter io used for the ArduinoClient class which will store the serial port
         * information when the ArduinoClient object is declared. The ArduinoClient object
         * and Node object (ROS2 node) have a object composition relationship with the ShovelNode.
         **/
        ShovelNode(boost::asio::io_context& io);

    private:
        ArduinoClient Arduino;

        /**
         * @brief Controls the arm based off the command sent, writing necessary values to
         * the arduino/esp32 pins
         * @parameter msg the command being "sent" where the data is a string of size
         * 1. Expected values are "f" (forwards), "b" (backwards), neither.
         **/
        void arm_callback(const std_msgs::msg::String::SharedPtr msg);

        /**
         * @brief Controls the bucket based off the command sent, writing necessary values to
         * the arduino/esp32 pins
         * @parameter msg the command being "sent" where the data is a string of size
         * 1. Expected values are "f" (forwards), "b" (backwards), neither.
         **/
        void bucket_callback(const std_msgs::msg::String::SharedPtr msg);

        /**
         * @brief Controls the scoop based off the command sent, writing necessary values to
         * the arduino/esp32 pins
         * @parameter msg the command being "sent" where the data is a string of size
         * 1. Expected values are "f" (forwards), "b" (backwards), neither.
         **/
        void scoop_callback(const std_msgs::msg::String::SharedPtr msg);

        /**
         * @brief The shutdown() function puts the arduino/esp32 in a shutdown state
         * as well as clearing all GPIO channels.
         **/
        void shutdown();

        rclcpp::Subscription<std_msgs::msg::String>::SharedPtr arm_subscription;
        rclcpp::Subscription<std_msgs::msg::String>::SharedPtr bucket_subscription;
        rclcpp::Subscription<std_msgs::msg::String>::SharedPtr scoop_subscription;
};

