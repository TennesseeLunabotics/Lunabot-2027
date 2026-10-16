/**
 * PROGRAM NAME: 
 * PROGRAM DESCRIPTION: 
 * DATE: 
 * SOURCES:
 *
 * LAST CONTRIBUTOR: James Ogle
 * LAST CONTRIBUTOR NOTES:
 */

#include <chrono>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <string>
#include <JetsonGPIO.h>
#include "arduino_client.hpp"

class ShovelNode : public rclcpp::Node {
    public:
        /**
         * @brief
         * @parameter io
         **/
        ShovelNode(boost::asio::io_context& io);
        /*arduino(boost::asio::io_context io, "/dev/ttyUSB0", 9600)*/

    private:
        ArduinoClient Arduino;

        /* arm cmd */
        void arm_callback(const std_msgs::msg::String::SharedPtr msg);

        /* bucket cmd */
        void bucket_callback(const std_msgs::msg::String::SharedPtr msg);

        /* scoop cmd */
        void scoop_callback(const std_msgs::msg::String::SharedPtr msg);

        rclcpp::Subscription<std_msgs::msg::String>::SharedPtr arm_subscription;
        rclcpp::Subscription<std_msgs::msg::String>::SharedPtr bucket_subscription;
        rclcpp::Subscription<std_msgs::msg::String>::SharedPtr scoop_subscription;
};

