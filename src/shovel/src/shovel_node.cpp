#include <chrono>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <string>

class ShovelNode : public rclcpp::Node {
   public:
    ShovelNode() : Node("shovel_node") {
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

   private:
    void arm_callback(const std_msgs::msg::String::SharedPtr msg) {
        // arm cmd
    }

    void bucket_callback(const std_msgs::msg::String::SharedPtr msg) {
        // bucket cmd
    }

    void scoop_callback(const std_msgs::msg::String::SharedPtr msg) {
        // scoop cmd
    }

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr arm_subscription;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr bucket_subscription;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr scoop_subscription;
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ShovelNode>());
    rclcpp::shutdown();
    return 0;
}