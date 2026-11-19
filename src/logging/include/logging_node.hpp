/**
 * PROGRAM NAME: logging
 * PROGRAM DESCRIPTION: Header file for logging.cpp.
 * DATE: 10/29/2026
 * SOURCES:
 *
 * LAST CONTRIBUTOR: James Overall
 * LAST CONTRIBUTOR NOTES:
 */

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <logging_constants.h>
#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <string>
#include <fstream>


class LoggingNode : public rclcpp::Node{
        /**
         * @brief Constructor for Logging node
         **/
        public:
                LoggingNode();
        private:
                /**
                 * @brief Calculates difference in hertz between two pulses
                 * 
                 **/

                const double avg_time_btwn_image();
                /**
                 * @brief Checks time between images recieved, if over 3 seconds, says not recieving stream
                 **/
                 
                const bool recieving_stream();
                
                /**
                 * @brief Combines variables from log to make consise messsage for log and WebBridge
                 **/

                std::string full_message();

                /**
                * @brief From subscribing to camera diagnostics node, pulls message sent to parse data
                * @parameter msg the diagnostics message the camera is sending - gets turned into a string and 
                * desired values pulled out
                **/
                void camera_callback(diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg);
                

        //log file that holds basic information abt camera, currently logging camera temp and hz
        std::ofstream log_file;
        //Holds time of previous message to calculate time between messages
        rclcpp::Time last_msg_time;
        //Holds camera temp so it can be fed to log file
        double asic_temp = 0;
        //Holds hz so it can be fed to log file
        double depth_hz = 0;
        //Text line to search for in log right before temp reading so diagnostics message can be parsed with substring
        size_t asic_index;
        //Same thing but for the hz
        size_t depth_index;
        //Creates a publisher that pushes the log holding all past camera temps and hzs
        rclcpp::Publisher<std_msgs::msg::String>::SharedPtr log_publisher;
        //Creates a subscription to the camera(duh) but specifically the diagnostics message - sends message into camera_callback
        rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr camera_subscription;
};