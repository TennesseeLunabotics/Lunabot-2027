#ifndef ROBOT_COMMON_CONSTANTS_HPP
#define ROBOT_COMMON_CONSTANTS_HPP

#include <string_view>
#include <sensor_msgs/msg/joy.hpp>

namespace robot_constants {
    const std::string_view FORWARD = "f";
    const std::string_view BACKWARD = "b";

    namespace ControllerMapping {
        constexpr int BUTTON_A = 0;
        constexpr int BUTTON_B = 1;
        constexpr int BUTTON_X = 2;
        constexpr int BUTTON_Y = 3;
        constexpr int BUTTON_BACK = 6;
        constexpr int BUTTON_MANUFACTURER = 8;
        constexpr int BUTTON_START = 7;
        constexpr int BUTTON_LSTICK = 9;
        constexpr int BUTTON_RSTICK = 10;
        constexpr int BUTTON_LBUMPER = 4;
        constexpr int BUTTON_RBUMPER = 5;

        constexpr int AXIS_LEFTX = 0;
        constexpr int AXIS_LEFTY = 1;
        constexpr int AXIS_RIGHTX = 3;
        constexpr int AXIS_RIGHTY = 4;
        constexpr int AXIS_LTRIGGER = 2;
        constexpr int AXIS_RTRIGGER = 5;
        constexpr int AXIS_DPAD_X = 6;
        constexpr int AXIS_DPAD_Y = 7;
    };

    namespace MotorMapping {
        const int NUM_MOTOR = 2;
        const std::string MOTOR_LOCATIONS[NUM_MOTOR] = {"Left", "Right"};
        constexpr int MOTOR_LEFT = 2;
        constexpr int MOTOR_RIGHT = 3;
        constexpr int MOTOR_MAX = 12;
    };
    
    struct Actions {
        bool default_mode;
        bool mode_switch_1;
        bool mode_switch_2;

        bool scoop_forward;
        bool scoop_backward;

        bool bucket_forward;
        bool bucket_backward;

        bool arm_forward;
        bool arm_backward;

        Actions(const sensor_msgs::msg::Joy::SharedPtr& joy_msg) {
            using namespace ControllerMapping;

            default_mode = joy_msg->axes[AXIS_DPAD_X] < -0.5 || joy_msg->buttons[BUTTON_RSTICK];
            mode_switch_1 = joy_msg->buttons[BUTTON_LSTICK];
            mode_switch_2 = joy_msg->axes[AXIS_DPAD_Y] > 0.5;

            scoop_forward = joy_msg->buttons[BUTTON_RBUMPER];
            scoop_backward = joy_msg->axes[AXIS_RTRIGGER] < 0; // convert [0, 1] to boolean

            bucket_forward = joy_msg->buttons[BUTTON_X];
            bucket_backward = joy_msg->buttons[BUTTON_Y];

            arm_forward = joy_msg->buttons[BUTTON_LBUMPER];
            arm_backward = joy_msg->axes[AXIS_LTRIGGER] < 0;
        }
    };
}

#endif