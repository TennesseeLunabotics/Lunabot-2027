#include <memory>

#include <cstdio>
#include <iostream>
#include <joybuttons.h>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joy.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include "string"
#include "constants.h"
#include "std_msgs/msg/bool.hpp"
#include "std_msgs/msg/string.hpp"
#include "interfaces/srv/set_teleop.hpp"


using std::placeholders::_1;
using namespace std;

class Teleop : public rclcpp::Node
{
 public:
    Teleop()
    : Node("Teleop")
    {
	subscription_ = this->create_subscription<sensor_msgs::msg::Joy>("joy", 10, std::bind(&Teleop::topic_callback, this, _1));
	drivetrainPub = this->create_publisher<sensor_msgs::msg::JointState>("drivetrain_cmd_vel", 10);
	armPub = this->create_publisher<std_msgs::msg::String>("shovel/arm_cmd", 10);
	scoopPub = this->create_publisher<std_msgs::msg::String>("shovel/scoop_cmd", 10);
	bucketPub = this->create_publisher<std_msgs::msg::String>("shovel/bucket_cmd", 10);
	service = this->create_service<interfaces::srv::SetTeleop>("set_teleop", [this](const std::shared_ptr<interfaces::srv::SetTeleop::Request> request,
                  std::shared_ptr<interfaces::srv::SetTeleop::Response> response) { this->set_teleop(request, response); });
    }

  private:
    void topic_callback(const sensor_msgs::msg::Joy &raw) {
        sensor_msgs::msg::JointState drivetrain_states;
        drivetrain_states.velocity.resize(2);
        drivetrain_states.velocity[0] = 0;
        drivetrain_states.velocity[1] = 0;
        std_msgs::msg::String bucket_state;
        std_msgs::msg::String arm_state;
        std_msgs::msg::String scoop_state;

        if(raw.buttons[BUTTON_LSTICK]) {
          robotState = 1;
        }
		if(raw.axes[AXIS_DPAD_X] < - 0.5){
	  		robotState = 0;
		}
		if(raw.axes[AXIS_DPAD_Y] > 0.5){
	  		robotState = 2;
		}
		switch(robotState){
			case 1:
            	if (!raw.buttons[BUTTON_B]) {
	              drivetrain_states.velocity[0] = (raw.axes[AXIS_LEFTY])*MOTOR_MAX;
    	          drivetrain_states.velocity[1] = (raw.axes[AXIS_RIGHTY])*MOTOR_MAX;
        	    } else {
            	  drivetrain_states.velocity[0] = (raw.axes[AXIS_LEFTY])*ARHAN_MODE * MOTOR_MAX;
	              drivetrain_states.velocity[1] = (raw.axes[AXIS_RIGHTY])*ARHAN_MODE * MOTOR_MAX;
    	        }
	
    	        if (raw.buttons[BUTTON_RBUMPER]) {
			scoop_state.data = "f";
	            } else if (raw.axes[AXIS_RTRIGGER] < 0) {
    	          scoop_state.data = "b";
        	    }

	            if (raw.axes[AXIS_LTRIGGER] < 0) {
    	          arm_state.data = "b";
        	    } else if (raw.buttons[BUTTON_LBUMPER]) {
            	  arm_state.data = "f";
	            }

    	        if (raw.buttons[BUTTON_Y]) {
        	      bucket_state.data = "b";
            	} else if (raw.buttons[BUTTON_X]) {
	              bucket_state.data = "f";
    	        }
		break;
	    case 2:
		if(raw.axes[AXIS_DPAD_Y] < -0.5){
		    autoState = "dumping";
		}
    		if(raw.axes[AXIS_DPAD_X] > 0.5){
		    autoState = "mining";
		}
		break;
	}
	if(autoState == "dumping"){
	    this->dump();
	}else if (autoState == "mining"){
	    this->mine();
	}
        drivetrainPub->publish(drivetrain_states);
        scoopPub->publish(scoop_state);
        armPub->publish(arm_state);
        bucketPub->publish(bucket_state);
    }

    //Auto dump
    //Needs to raise arms all the way, tilt scoop all the way back, and lower dump
    //Then needs to reset itself
    void dump(){
	
    }

    //Auto mine
    //Lower arm to the bottom, drive forward for a second, raise a little bit, drive forward a half second, 
    //raise arm to max, and tilt the scoop all the way back 
    void mine(){
    
    }

    void set_teleop(const std::shared_ptr<interfaces::srv::SetTeleop::Request> request, std::shared_ptr<interfaces::srv::SetTeleop::Response> response){
    	string enabled;
      	robotState = request->teleop_enabled;

	    if(robotState == 1) {
    		RCLCPP_INFO(get_logger(), "\033[1;35mMANUAL CONTROL:\033[0m \033[1;32mENABLED\033[0m");
        	enabled = "enabled";
      	} else {
        	RCLCPP_INFO(get_logger(), "\033[1;35mMANUAL CONTROL:\033[0m \033[1;32mDISABLED\033[0m");
        	enabled = "disabled";
      	}

      	response->message = "Teleop: " + enabled;
      	response->success = true;
    }

    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr subscription_;
    rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr drivetrainPub;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr bucketPub;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr armPub;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr scoopPub;
    rclcpp::Service<interfaces::srv::SetTeleop>::SharedPtr service;

    int robotState = 0;
    string autoState = "";
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Teleop>());
  rclcpp::shutdown();
  return 0;
}
