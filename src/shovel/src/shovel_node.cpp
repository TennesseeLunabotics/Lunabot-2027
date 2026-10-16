/**
 * PROGRAM NAME: 
 * PROGRAM DESCRIPTION: 
 * DATE: 
 * SOURCES:
 *
 * LAST CONTRIBUTOR: James Ogle
 * LAST CONTRIBUTOR NOTES:
 */


#include <iterator>
#include <memory>
//#include <rclcpp/rclcpp.hpp>
//#include <std_msgs/msg/string.hpp>
//#include <rclcpp/node.hpp>
#include <string>
#include <JetsonGPIO.h>
//#include "arduino_client.hpp"
#include "shovel_node.hpp"
#include "constants.h"
#include <cstdlib>
#include <math.h>
#include <vector>

using std::string;

/* Shovel constructor */
ShovelNode::ShovelNode(boost::asio::io_context& io) :
    Node("shovel_node"),
    Arduino(io, "/dev/ttyUSB0", 9600) {
    /* Establish Connection with esp32 */

    Arduino.reset();

    /* Creating Subscriptions for arm, bucket, and scoop */
    arm_subscription = this->create_subscription<std_msgs::msg::String>(
        "/shovel/arm_cmd", 10,
        std::bind(&ShovelNode::arm_callback, this, std::placeholders::_1));

    bucket_subscription = this->create_subscription<std_msgs::msg::String>(
        "/shovel/bucket_cmd", 10,
        std::bind(&ShovelNode::bucket_callback, this,
                  std::placeholders::_1));

    scoop_subscription = this->create_subscription<std_msgs::msg::String>(
        "/shovel/scoop_cmd", 10,
        std::bind(&ShovelNode::scoop_callback, this,
                  std::placeholders::_1));
}


void ShovelNode::arm_callback(const std_msgs::msg::String::SharedPtr msg) {
    string feedback;
    int armDiff;
    int bitmask;
    int compVal;
    std::vector <unsigned int> pwmValues;
    std::vector <unsigned int> digPins;
    std::vector<unsigned int>  pwmPins;

    feedback = Arduino.read_analogue();

    if (feedback.length() >= 3 && (feedback[0] == 'A')) {
        armDiff = (int) (feedback[3] - feedback[1]);
        /* Grabbing absolute value of number using bit arithmetic */
        bitmask = armDiff >> 31;
        armDiff = (armDiff^bitmask) - bitmask; // absolute val of armDiff
        compVal = armDiff * CONSTANTS::ARM_P + CONSTANTS::ARM_DUTY_CYCLE;
        if (compVal < 255) compVal = 255;
    }
    else {
        armDiff = 0;
        compVal = CONSTANTS::ARM_DUTY_CYCLE;
    }
    if ( ((bool)(armDiff > 0)) ^
            ((bool)(msg->data.c_str()[0] == CONSTANTS::ARM_BACKWARD)) )
    {
        pwmValues = {(unsigned int)compVal, CONSTANTS::ARM_DUTY_CYCLE};
    }
    else {
        pwmValues = {CONSTANTS::ARM_DUTY_CYCLE, (unsigned int)compVal};
    }

    /* Writing Values to Arduino pins based off Arm movement command */
    if (msg->data.c_str()[0] == CONSTANTS::ARM_FORWARD) {
        digPins.assign(std::begin(CONSTANTS::ARM_DIR), std::end(CONSTANTS::ARM_DIR));
        Arduino.digital_write(digPins, {1,1});
    }
    else if (msg->data.c_str()[0] == CONSTANTS::ARM_BACKWARD) {
        digPins.assign(std::begin(CONSTANTS::ARM_DIR), std::end(CONSTANTS::ARM_DIR));
        Arduino.digital_write(digPins, {0,0});
    }
    else { pwmValues = {0,0}; }
    pwmPins.assign(std::begin(CONSTANTS::ARM_PWM), std::end(CONSTANTS::ARM_PWM));
    Arduino.pwm_write(pwmPins, pwmValues);
}

void ShovelNode::bucket_callback(const std_msgs::msg::String::SharedPtr msg) {
    (void) msg;
    
}

void ShovelNode::scoop_callback(const std_msgs::msg::String::SharedPtr msg) {
    (void) msg;
}

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ShovelNode>());
    rclcpp::shutdown();
}
