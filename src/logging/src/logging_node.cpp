/**
 * @file Logging.cpp
 * @brief Parse data from different encoders and prepre it for 
 * 
 * @author James Overall
 */

#include <iostream>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <logging_constants.h>
#include <logging_node.hpp>


LoggingNode::LoggingNode() :
    Node("logging_node"),
    last_msg_time_(this->now()) {

        camera_subscription = this->create_subscription<diagnostic_msgs::msg::DiagnosticArray>(
        "/diagnostics", 10,
        std::bind(&LoggingNode::camera_callback, this, std::placeholders::_1));
        log_publisher = this->create_publisher<std_msgs::msg::String>("/camera_log", 10);

        }





void LoggingNode::camera_callback(const diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg) {
  last_msg_time_ = this->now();
  size_t asic_index = msg.find("key: Asic Temperature")
  double asic_temp = stod(msg.substring(asic_index + 34,2)) 
  size_t depth_index = msg.find("key: Asic Temperature")
  double depth_hz = stod(msg.substring(asic_index + 40,6)) 

  find()
  if(str.getline == "asic")
    str:getline(i++):substring
  asic_temp = get
  std::string line = full_message();
  log_file << line << std::endl;         
  std_msgs::msg::String out; out.data = line;
  log_publisher->publish(out);           
}

std::string LoggingNode::full_message() {
  std::string line = "ASIC: " + std::to_string(asic_temp) + " C, Hz: " + std::to_string(depth_hz) + " 1/s";

  return line;
}

double LoggingNode::avg_time_btwn_image() const {
  return depth_hz > 0.0 ? 1.0 / depth_hz : 0.0;
}

bool LoggingNode::receiving_stream() const {
  return (this->now() - last_msg_time).seconds() < 3.0;  
}



