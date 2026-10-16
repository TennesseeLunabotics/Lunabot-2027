/**
 * PROGRAM NAME: shovel_node
 * PROGRAM DESCRIPTION: Implementation of the header file shovel_node.hpp.
 * The program handles messages related to arm, bucket, and scoop (all required
 * for a shovel movement). The Node and ArduionClient (AC) objects are composition objects
 * of this class (Each time a new ShovelNode object is created, it will also create
 * new objects of Node and AC). Node holds message commands from other nodes while
 * ArduinoClient handles writing to the arduino/esp32 pins required for movement of
 * shovel, bucket, and scoop.
 * DATE: 10/15/2026
 * SOURCES:
 *
 * LAST CONTRIBUTOR: James Ogle
 * LAST CONTRIBUTOR NOTES:
 * - TODO eventually have it where ShovelNode does not require an io input but this
 *   change will need to occur in ArduinoClient class.
 * - TODO Also related to ArduinoClient class will be changing its vector inputs to
 *   arrays to avoid heap allocations.
 */


#include <boost/asio/io_context.hpp>
#include <iterator>
#include <memory>
#include <string>
#include <JetsonGPIO.h>
#include "shovel_node.hpp" /* includes necessary ROS libraries and Arduino Header */
#include "constants.h"
#include <vector>

using std::string;

/* Shovel constructor */
ShovelNode::ShovelNode(boost::asio::io_context& io) :
    Node("shovel_node"),
    Arduino(io, "/dev/ttyUSB0", 9600) {

    /* Restarting Arduino in-case it was in shutdown state */
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


/* ARM CALLBACK */
void ShovelNode::arm_callback(const std_msgs::msg::String::SharedPtr msg) {
    string feedback;
    int armDiff;
    int bitmask;
    int compVal;
    std::vector <unsigned int> pwmValues;
    std::vector <unsigned int> digPins;
    std::vector <unsigned int> pwmPins;

    feedback = Arduino.read_analogue();

    /* Checking if Arduino already has values set or not */
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

    /* Determing pwm values to set based off values returned in feedback */
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

/* BUCKET CALLBACK */
void ShovelNode::bucket_callback(const std_msgs::msg::String::SharedPtr msg) {
    /* If message command is move bucket forward */
    if (msg->data.c_str()[0] == CONSTANTS::BUCKET_FORWARD) {
        Arduino.pwm_write({CONSTANTS::BUCKET_PWM}, {CONSTANTS::BUCKET_DUTY_CYCLE});
        Arduino.digital_write({CONSTANTS::BUCKET_PWM}, {1});
    }
    /* If message command is move bucket backward */
    else if (msg->data.c_str()[0] == CONSTANTS::BUCKET_BACKWARD) {
        Arduino.pwm_write({CONSTANTS::BUCKET_PWM}, {CONSTANTS::BUCKET_DUTY_CYCLE});
        Arduino.digital_write({CONSTANTS::BUCKET_DIR}, {0});
    }
    /* command contains is neither forwards nor backwards */
    else { Arduino.pwm_write({CONSTANTS::BUCKET_DIR}, {0}); }
}

/* SCOOP CALLBACK */
void ShovelNode::scoop_callback(const std_msgs::msg::String::SharedPtr msg) {
    /* setting pin values for scoop forward command */
    if (msg->data.c_str()[0] == CONSTANTS::SCOOP_FORWARD) {
        Arduino.pwm_write({CONSTANTS::SCOOP_PWM}, {CONSTANTS::SCOOP_DUTY_CYCLE});
        Arduino.digital_write({CONSTANTS::SCOOP_DIR}, {0});
    }
    /* setting pin values for scoop backward command */
    else if (msg->data.c_str()[0] == CONSTANTS::SCOOP_BACKWARD) {
        Arduino.pwm_write({CONSTANTS::SCOOP_PWM}, {CONSTANTS::SCOOP_DUTY_CYCLE});
        Arduino.digital_write({CONSTANTS::SCOOP_DIR}, {1});
    }
    /* Cmd not forward or backward, setting pwm pin values to 0 */
    else { Arduino.pwm_write({CONSTANTS::SCOOP_PWM}, {0}); }
}

/* SHUTDOWN */
void ShovelNode::shutdown() {
    /* throwing arduino into shutdown state, not disconnecting */
    Arduino.shutdown();
    GPIO::cleanup();    /* cleans up all used GPIO channels */
}

/* Main shows how to include IO context for the ArduinoClient class  */
int main(int argc, char* argv[]) {
    boost::asio::io_context io;
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ShovelNode>(io));
    rclcpp::shutdown();
}
