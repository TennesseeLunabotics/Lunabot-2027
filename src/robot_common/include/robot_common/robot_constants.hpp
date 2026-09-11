#ifndef ROBOT_COMMON_CONSTANTS_HPP
#define ROBOT_COMMON_CONSTANTS_HPP

#include <string_view>
#include <sensor_msgs/msg/joy.hpp>

namespace robot_constants {
    constexpr int NUM_MOTOR = 2;
    constexpr std::string_view[NUM_MOTOR] MOTOR_LOCATIONS = {"Left", "Right"};
    
    constexpr std::string_view FORWARD = "f";
    constexpr std::string_view BACKWARD = "b";

    struct ControllerMapping {
        static constexpr int AXIS_LEFTX = 0;
        static constexpr int AXIS_LEFTY = 1;
        static constexpr int AXIS_RIGHTX = 3;
        static constexpr int AXIS_RIGHTY = 4;
        static constexpr int AXIS_DPAD_X = 6;
        static constexpr int AXIS_DPAD_Y = 7;

        static constexpr int BUTTON_A = 0;
        static constexpr int BUTTON_B = 1;
        static constexpr int BUTTON_X = 2;
        static constexpr int BUTTON_Y = 3;
        static constexpr int BUTTON_LB = 4;
        static constexpr int BUTTON_RB = 5;
        static constexpr int BUTTON_BACK = 6;
        static constexpr int BUTTON_START = 7;
        static constexpr int BUTTON_LSTICK = 9;
        static constexpr int BUTTON_RSTICK = 10;
    };

    struct MotorMapping {
        static constexpr int MOTOR_LEFT = 2;
        static constexpr int MOTOR_RIGHT = 3;
        static constexpr int MOTOR_MAX = 12;
    };
    
    struct Actions {
        bool mode_switch;
        bool scoop_forward;

        Actions(const sensor_msgs::msg::Joy::SharedPtr& joy_msg) {
            using namespace ControllerMapping;

            default_mode = joy_msg->axes[AXIS_DPAD_X] < -0.5 || joy_msg->buttons[BUTTON_RSTICK];
            mode_switch_1 = joy_msg->buttons[BUTTON_LSTICK];
            mode_switch_2 = joy_msg->axes[AXIS_DPAD_Y] > 0.5;

            scoop_forward = joy_msg->buttons[BUTTON_RB];
            scoop_backward = joy_msg->axes[AXIS_RTRIGGER] < 0; // convert [0, 1] to boolean

            bucket_forward = joy_msg->buttons[BUTTON_X];
            bucket_backward = joy_msg->buttons[BUTTON_Y];

            arm_forward = joy_msg->buttons[BUTTON_LB];
            arm_backward = joy_msg->axes[AXIS_LTRIGGER] < 0;
        }
    };
}

#endif