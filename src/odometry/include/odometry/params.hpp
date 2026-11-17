// params.hpp: declare a setting (with a default), read it from launch/odometry.launch.py,
// and throw if it's invalid, so a bad value stops the node at startup with a clear message.
#ifndef ODOMETRY_PARAMS_HPP
#define ODOMETRY_PARAMS_HPP

#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>

#include <rclcpp/rclcpp.hpp>

namespace odometry {

// Any real number (not NaN/inf), e.g. mount offsets and angles.
inline double declare_finite(rclcpp::Node& node, const std::string& name, double fallback) {
    node.declare_parameter<double>(name, fallback);
    const double value = node.get_parameter(name).as_double();
    if (!std::isfinite(value)) {
        throw std::invalid_argument("parameter '" + name + "' must be a finite number");
    }
    return value;
}

// Must be > 0. Robot-specific values ship as 0.0, so forgetting one fails here.
inline double declare_positive(rclcpp::Node& node, const std::string& name, double fallback = 0.0) {
    const double value = declare_finite(node, name, fallback);
    if (!(value > 0.0)) {
        throw std::invalid_argument("parameter '" + name + "' must be > 0 (got " +
                                    std::to_string(value) +
                                    "). Set it in launch/odometry.launch.py "
                                    "(write it with a decimal point, e.g. 0.1)");
    }
    return value;
}

inline int declare_positive_int(rclcpp::Node& node, const std::string& name, int fallback) {
    node.declare_parameter<int>(name, fallback);
    const int64_t value = node.get_parameter(name).as_int();
    if (value <= 0 || value > 1000000) {
        throw std::invalid_argument("parameter '" + name +
                                    "' must be a whole number from 1 to 1000000");
    }
    return static_cast<int>(value);
}

inline std::string declare_nonempty(rclcpp::Node& node, const std::string& name,
                                    const std::string& fallback) {
    node.declare_parameter<std::string>(name, fallback);
    const std::string value = node.get_parameter(name).as_string();
    if (value.empty()) {
        throw std::invalid_argument("parameter '" + name + "' must not be empty");
    }
    return value;
}

// A wrong lidar mount silently gives wrong motion, so refuse to start until it's confirmed.
inline void require_mount_confirmed(rclcpp::Node& node) {
    node.declare_parameter<bool>("lidar_mount_confirmed", false);
    if (!node.get_parameter("lidar_mount_confirmed").as_bool()) {
        throw std::invalid_argument(
            "parameter 'lidar_mount_confirmed' is false. Measure the lidar_* mount values in "
            "launch/odometry.launch.py, then set lidar_mount_confirmed to True");
    }
}

}  // namespace odometry

#endif  // ODOMETRY_PARAMS_HPP
