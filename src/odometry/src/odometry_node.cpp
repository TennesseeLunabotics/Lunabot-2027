// odometry_node: drive encoders -> forward speed on encoder/odom (sideways speed is always 0).
// No turn rate: treads skid when turning, so the gyro handles that.
// Speed is averaged over velocity_window_s because the motor controller only
// refreshes its position every few tens of ms, so per-message speeds jump around.
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>

#include <nav_msgs/msg/odometry.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>

#include "odometry/params.hpp"

namespace odometry {

namespace {
constexpr int kLogThrottleMs = 2000;  // min time between repeated warnings
constexpr std::size_t kQueueDepth = 10;
constexpr double kUnmeasuredVariance = 1.0e6;  // huge variance = "EKF, ignore this value"
constexpr double kTwoPi = 6.28318530717958647692;
}  // namespace

// Meters a tread rolled for a change in encoder position:
// (delta / units per sprocket turn) * sprocket circumference.
inline double tread_travel_m(double delta_position, double position_units_per_sprocket_rev,
                             double sprocket_radius_m) {
    return delta_position / position_units_per_sprocket_rev * kTwoPi * sprocket_radius_m;
}

// Settings from ENCODER_CONFIG in launch/odometry.launch.py.
struct OdometryConfig {
    double sprocket_radius_m{0.0};
    double position_units_per_sprocket_rev{0.0};
    int left_sign{1};
    int right_sign{1};
    double max_tread_speed_mps{0.0};
    double twist_linear_variance{0.0};
    double velocity_window_s{0.0};
    double max_message_gap_s{0.0};
    std::string joint_state_topic;
    std::string odom_topic;
    std::string left_joint_name;
    std::string right_joint_name;
    std::string odom_frame;
    std::string base_frame;
};

class OdometryNode : public rclcpp::Node {
   public:
    OdometryNode() : Node("odometry"), cfg_(load_config()) {
        odom_pub_ = create_publisher<nav_msgs::msg::Odometry>(cfg_.odom_topic, kQueueDepth);
        joint_sub_ = create_subscription<sensor_msgs::msg::JointState>(
            cfg_.joint_state_topic, kQueueDepth,
            [this](const sensor_msgs::msg::JointState& msg) { on_joint_state(msg); });

        RCLCPP_INFO(get_logger(), "Encoder odometry started: '%s' -> '%s'",
                    cfg_.joint_state_topic.c_str(), cfg_.odom_topic.c_str());
    }

   private:
    // ---- Configuration -------------------------------------------------------

    int declare_sign(const std::string& name) {
        declare_parameter<int>(name, 1);
        const int value = static_cast<int>(get_parameter(name).as_int());
        if (value != 1 && value != -1) {
            throw std::invalid_argument("parameter '" + name + "' must be 1 or -1");
        }
        return value;
    }

    // Robot measurements have no default (0.0), so the node won't start until they're set.
    OdometryConfig load_config() {
        OdometryConfig c;
        c.sprocket_radius_m = declare_positive(*this, "sprocket_radius_m");
        c.position_units_per_sprocket_rev =
            declare_positive(*this, "position_units_per_sprocket_rev");
        c.max_tread_speed_mps = declare_positive(*this, "max_tread_speed_mps", 3.0);
        c.twist_linear_variance = declare_positive(*this, "twist_linear_variance", 0.1);
        c.velocity_window_s = declare_positive(*this, "velocity_window_s", 0.1);
        c.max_message_gap_s = declare_positive(*this, "max_message_gap_s", 0.5);
        if (!(c.max_message_gap_s > c.velocity_window_s)) {
            throw std::invalid_argument(
                "'max_message_gap_s' must be longer than 'velocity_window_s'");
        }
        c.left_sign = declare_sign("left_sign");
        c.right_sign = declare_sign("right_sign");
        c.joint_state_topic =
            declare_nonempty(*this, "joint_state_topic", "drivetrain_sensor_output");
        c.odom_topic = declare_nonempty(*this, "odom_topic", "encoder/odom");
        c.left_joint_name = declare_nonempty(*this, "left_joint_name", "Left");
        c.right_joint_name = declare_nonempty(*this, "right_joint_name", "Right");
        c.odom_frame = declare_nonempty(*this, "odom_frame", "odom");
        c.base_frame = declare_nonempty(*this, "base_frame", "base_link");
        return c;
    }

    // ---- Input ---------------------------------------------------------------

    // Finds each tread by name (name[i] goes with position[i]). Returns false and
    // logs why if the message is malformed.
    bool read_wheel_positions(const sensor_msgs::msg::JointState& msg, double& left_pos,
                              double& right_pos) {
        const auto left_it = std::find(msg.name.begin(), msg.name.end(), cfg_.left_joint_name);
        const auto right_it = std::find(msg.name.begin(), msg.name.end(), cfg_.right_joint_name);
        if (left_it == msg.name.end() || right_it == msg.name.end()) {
            RCLCPP_ERROR_THROTTLE(
                get_logger(), *get_clock(), kLogThrottleMs,
                "JointState has no joint named '%s' and/or '%s'; dropping message",
                cfg_.left_joint_name.c_str(), cfg_.right_joint_name.c_str());
            return false;
        }

        const auto left_idx = static_cast<std::size_t>(std::distance(msg.name.begin(), left_it));
        const auto right_idx = static_cast<std::size_t>(std::distance(msg.name.begin(), right_it));
        if (left_idx >= msg.position.size() || right_idx >= msg.position.size()) {
            RCLCPP_ERROR_THROTTLE(
                get_logger(), *get_clock(), kLogThrottleMs,
                "JointState 'position' array is shorter than 'name'; dropping message");
            return false;
        }

        left_pos = msg.position[left_idx];
        right_pos = msg.position[right_idx];
        if (!(std::isfinite(left_pos) && std::isfinite(right_pos))) {
            RCLCPP_ERROR_THROTTLE(get_logger(), *get_clock(), kLogThrottleMs,
                                  "Wheel position is NaN or infinite; dropping message");
            return false;
        }
        return true;
    }

    // Start a new averaging window at these readings.
    void set_anchor(double left_pos, double right_pos, const rclcpp::Time& stamp) {
        anchor_left_pos_ = left_pos;
        anchor_right_pos_ = right_pos;
        anchor_stamp_ = stamp;
        have_anchor_ = true;
    }

    void on_joint_state(const sensor_msgs::msg::JointState& msg) {
        const rclcpp::Time now = this->now();

        double left_pos = 0.0;
        double right_pos = 0.0;
        if (!read_wheel_positions(msg, left_pos, right_pos)) {
            return;
        }

        // The drivetrain only reports while commanded; after a long silence, start over.
        if (!have_anchor_ || (now - last_msg_stamp_).seconds() > cfg_.max_message_gap_s) {
            set_anchor(left_pos, right_pos, now);
            last_msg_stamp_ = now;
            return;
        }
        last_msg_stamp_ = now;

        const double dt_s = (now - anchor_stamp_).seconds();
        if (dt_s < cfg_.velocity_window_s) {
            return;  // keep accumulating
        }

        try {
            const double left_m =
                tread_travel_m(cfg_.left_sign * (left_pos - anchor_left_pos_),
                               cfg_.position_units_per_sprocket_rev, cfg_.sprocket_radius_m);
            const double right_m =
                tread_travel_m(cfg_.right_sign * (right_pos - anchor_right_pos_),
                               cfg_.position_units_per_sprocket_rev, cfg_.sprocket_radius_m);

            // Faster than the robot can move = bad reading (CAN glitch, controller reset).
            if (std::abs(left_m) / dt_s > cfg_.max_tread_speed_mps ||
                std::abs(right_m) / dt_s > cfg_.max_tread_speed_mps) {
                RCLCPP_WARN_THROTTLE(
                    get_logger(), *get_clock(), kLogThrottleMs,
                    "Rejected implausible tread jump (left %.3f m, right %.3f m in %.3f s)", left_m,
                    right_m, dt_s);
            } else {
                publish(now, 0.5 * (left_m + right_m) / dt_s);  // average of both treads
            }
        } catch (const std::exception& e) {
            RCLCPP_ERROR_THROTTLE(get_logger(), *get_clock(), kLogThrottleMs,
                                  "Odometry update failed: %s", e.what());
        }
        set_anchor(left_pos, right_pos, now);
    }

    // ---- Output --------------------------------------------------------------

    void publish(const rclcpp::Time& stamp, double linear_mps) {
        nav_msgs::msg::Odometry odom;
        odom.header.stamp = stamp;
        odom.header.frame_id = cfg_.odom_frame;
        odom.child_frame_id = cfg_.base_frame;
        odom.pose.pose.orientation.w = 1.0;
        // Mark everything unmeasured (diagonal of each 6x6 covariance = indexes 0, 7, ..., 35),
        // then fill in forward and sideways speed.
        for (std::size_t i = 0; i < 6; ++i) {
            odom.pose.covariance[i * 7] = kUnmeasuredVariance;
            odom.twist.covariance[i * 7] = kUnmeasuredVariance;
        }
        odom.twist.twist.linear.x = linear_mps;
        odom.twist.twist.linear.y = 0.0;
        odom.twist.covariance[0] = cfg_.twist_linear_variance;
        odom.twist.covariance[7] = cfg_.twist_linear_variance;
        odom_pub_->publish(odom);
    }

    // ---- State ---------------------------------------------------------------
    OdometryConfig cfg_;
    bool have_anchor_{false};
    double anchor_left_pos_{0.0};  // encoder positions at the start of the window
    double anchor_right_pos_{0.0};
    rclcpp::Time anchor_stamp_{0, 0, RCL_ROS_TIME};
    rclcpp::Time last_msg_stamp_{0, 0, RCL_ROS_TIME};

    rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_sub_;
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
};

}  // namespace odometry

// Exits with code 1 and a clear message if a setting is invalid.
int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    int exit_code = 0;
    try {
        rclcpp::spin(std::make_shared<odometry::OdometryNode>());
    } catch (const std::exception& e) {
        RCLCPP_FATAL(rclcpp::get_logger("odometry"), "Fatal: %s", e.what());
        exit_code = 1;
    }
    rclcpp::shutdown();
    return exit_code;
}
