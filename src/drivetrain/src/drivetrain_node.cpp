#include <cstdio>
#include <iostream>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>

#include <robot_common/SparkMax.hpp>
#include <robot_common/robot_constants.hpp>
#include "string"

using std::placeholders::_1;
using namespace std;
using namespace robot_constants;

class Drivetrain : public rclcpp::Node {
   public:
    Drivetrain() : Node("Drivetrain") {
        subscription_ = this->create_subscription<sensor_msgs::msg::JointState>(
            "drivetrain_cmd_vel", 10,
            std::bind(&Drivetrain::topic_callback, this, _1));
        sensorPub = this->create_publisher<sensor_msgs::msg::JointState>(
            "drivetrain_sensor_output", 10);

        motors[0].SetInverted(false);
        motors[1].SetInverted(true);
        for (int i = 0; i < MotorMapping::NUM_MOTOR; i++) {
            motors[i].SetIdleMode(IdleMode::kBrake);
            motors[i].SetMotorType(MotorType::kBrushless);
        }
    }

   private:
    SparkMax motors[MotorMapping::NUM_MOTOR] = {SparkMax("can0", MotorMapping::MOTOR_LEFT),
                                                SparkMax("can0", MotorMapping::MOTOR_RIGHT)};
    void topic_callback(const sensor_msgs::msg::JointState& drivetrain_states) {
        // set motor values
        SparkMax::Heartbeat();
        for (int i = 0; i < MotorMapping::NUM_MOTOR; i++) {
            motors[i].SetVoltage(drivetrain_states.velocity[i] * MotorMapping::MOTOR_MAX);
        }

        // publish sensor data
        sensor_msgs::msg::JointState motor_states;
        motor_states.name.resize(MotorMapping::NUM_MOTOR);
        motor_states.velocity.resize(MotorMapping::NUM_MOTOR);
        motor_states.position.resize(MotorMapping::NUM_MOTOR);
        motor_states.effort.resize(MotorMapping::NUM_MOTOR);

        for (int i = 0; i < MotorMapping::NUM_MOTOR; i++) {
            motor_states.name[i] = MotorMapping::MOTOR_LOCATIONS[i];
            motor_states.velocity[i] = motors[i].GetVelocity();
            motor_states.position[i] = motors[i].GetPosition();
            motor_states.effort[i] = motors[i].GetVoltage();
        }
        sensorPub->publish(motor_states);
    }
    rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr subscription_;
    rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr sensorPub;
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Drivetrain>());
    rclcpp::shutdown();
    return 0;
}
