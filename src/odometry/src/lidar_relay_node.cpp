// lidar_relay_node: puts the lidar driver's data into robot axes (base_link).
//   - Cloud -> same cloud rotated/shifted into base_link on lidar/cloud_base, for KISS-ICP
//     (every other field is kept, including per-point time).
//   - IMU -> bias-corrected gyro on imu/gyro. Orientation and acceleration are left out
//     (the driver's orientation isn't trustworthy).
// Uses measured mount values instead of TF, because the driver's lidar frame moves with the
// IMU's orientation estimate. The gyro bias is measured at startup, so keep the robot still
// until the log says "Gyro calibration done" (an uncorrected 0.01 rad/s bias = ~34 degrees of
// drift per minute).
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <sensor_msgs/point_cloud2_iterator.hpp>

#include "odometry/params.hpp"

namespace odometry {

namespace {
constexpr int kLogThrottleMs = 2000;         // min time between repeated warnings
constexpr std::size_t kCloudQueueDepth = 5;  // clouds are big; keep only the newest few
constexpr std::size_t kImuQueueDepth = 50;   // the IMU is fast
constexpr int kMinCalibrationSamples = 2;
}  // namespace

// ---- Math -------------------------------------------------------------------

// A turning rate (rad/s) or a point (m).
struct Vec3 {
    double x{0.0};
    double y{0.0};
    double z{0.0};
};

inline bool is_finite(const Vec3& v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

inline double max_abs(const Vec3& v) {
    return std::max({std::abs(v.x), std::abs(v.y), std::abs(v.z)});
}

// Lidar/IMU axes -> robot axes (x forward, y left, z up).
// Mount angles are applied roll (x), then pitch (y), then yaw (z), about the robot's
// fixed axes (R = Rz * Ry * Rx).
inline Vec3 rotate_to_base(const Vec3& v, double roll_rad, double pitch_rad, double yaw_rad) {
    if (!is_finite(v)) {
        throw std::invalid_argument("rotate_to_base: vector must be finite");
    }
    if (!(std::isfinite(roll_rad) && std::isfinite(pitch_rad) && std::isfinite(yaw_rad))) {
        throw std::invalid_argument("rotate_to_base: mount angles must be finite");
    }

    const double cr = std::cos(roll_rad);
    const double sr = std::sin(roll_rad);
    const double cp = std::cos(pitch_rad);
    const double sp = std::sin(pitch_rad);
    const double cy = std::cos(yaw_rad);
    const double sy = std::sin(yaw_rad);

    // Rotate about x (roll).
    const double x1 = v.x;
    const double y1 = cr * v.y - sr * v.z;
    const double z1 = sr * v.y + cr * v.z;
    // Rotate about y (pitch).
    const double x2 = cp * x1 + sp * z1;
    const double y2 = y1;
    const double z2 = -sp * x1 + cp * z1;
    // Rotate about z (yaw).
    Vec3 out;
    out.x = cy * x2 - sy * y2;
    out.y = sy * x2 + cy * y2;
    out.z = z2;
    return out;
}

// Measures a gyro's constant offset (bias) while the robot is still:
// averages required_samples readings, and stable() says whether their spread stayed
// under max_stddev (rad/s). If not, reset() and try again.
class BiasEstimator {
   public:
    BiasEstimator(std::size_t required_samples, double max_stddev)
        : required_(required_samples), max_stddev_(max_stddev) {
        if (required_samples < 2) {
            throw std::invalid_argument("BiasEstimator: required_samples must be >= 2");
        }
        if (!(std::isfinite(max_stddev) && max_stddev > 0.0)) {
            throw std::invalid_argument("BiasEstimator: max_stddev must be > 0");
        }
    }

    // Ignored once complete().
    void add_sample(const Vec3& v) {
        if (!is_finite(v)) {
            throw std::invalid_argument("BiasEstimator: sample must be finite");
        }
        if (complete()) {
            return;
        }
        ++count_;
        const double n = static_cast<double>(count_);
        update_axis(v.x, n, mean_.x, m2_.x);
        update_axis(v.y, n, mean_.y, m2_.y);
        update_axis(v.z, n, mean_.z, m2_.z);
    }

    bool complete() const { return count_ >= required_; }

    // Largest standard deviation across the three axes so far.
    double max_stddev_observed() const {
        if (count_ == 0) {
            return 0.0;
        }
        const double n = static_cast<double>(count_);
        return std::max({std::sqrt(m2_.x / n), std::sqrt(m2_.y / n), std::sqrt(m2_.z / n)});
    }

    bool stable() const { return complete() && max_stddev_observed() <= max_stddev_; }

    // The average reading. Throws if called before complete().
    Vec3 bias() const {
        if (!complete()) {
            throw std::logic_error("BiasEstimator: bias requested before enough samples");
        }
        return mean_;
    }

    void reset() {
        count_ = 0;
        mean_ = Vec3{};
        m2_ = Vec3{};
    }

   private:
    // Running mean and spread without storing every sample.
    static void update_axis(double sample, double n, double& mean, double& m2) {
        const double delta = sample - mean;
        mean += delta / n;
        m2 += delta * (sample - mean);
    }

    std::size_t required_;
    double max_stddev_;
    std::size_t count_{0};
    Vec3 mean_;
    Vec3 m2_;  // sum of squared deviations
};

// ---- Node -------------------------------------------------------------------

// Settings from LIDAR_MOUNT and LIDAR_RELAY_CONFIG in launch/odometry.launch.py.
struct LidarRelayConfig {
    double lidar_x_m{0.0};
    double lidar_y_m{0.0};
    double lidar_z_m{0.0};
    double lidar_roll_rad{0.0};
    double lidar_pitch_rad{0.0};
    double lidar_yaw_rad{0.0};
    std::size_t calibration_samples{0};
    double max_calibration_stddev_rps{0.0};
    double max_abs_gyro_rps{0.0};
    double gyro_variance{0.0};
    std::string cloud_input_topic;
    std::string cloud_output_topic;
    std::string imu_input_topic;
    std::string imu_output_topic;
    std::string output_frame;
};

class LidarRelayNode : public rclcpp::Node {
   public:
    LidarRelayNode()
        : Node("lidar_relay"),
          cfg_(load_config()),
          estimator_(cfg_.calibration_samples, cfg_.max_calibration_stddev_rps) {
        // Sensor-data QoS (lost clouds aren't resent), matching the driver and KISS-ICP.
        const auto cloud_qos = rclcpp::SensorDataQoS().keep_last(kCloudQueueDepth);
        cloud_pub_ =
            create_publisher<sensor_msgs::msg::PointCloud2>(cfg_.cloud_output_topic, cloud_qos);
        // UniquePtr lets us edit the cloud in place and pass it on without copying.
        cloud_sub_ = create_subscription<sensor_msgs::msg::PointCloud2>(
            cfg_.cloud_input_topic, cloud_qos,
            [this](sensor_msgs::msg::PointCloud2::UniquePtr msg) { on_cloud(std::move(msg)); });

        imu_pub_ = create_publisher<sensor_msgs::msg::Imu>(cfg_.imu_output_topic, kImuQueueDepth);
        imu_sub_ = create_subscription<sensor_msgs::msg::Imu>(
            cfg_.imu_input_topic, kImuQueueDepth,
            [this](const sensor_msgs::msg::Imu& msg) { on_imu(msg); });

        RCLCPP_INFO(get_logger(),
                    "Lidar relay started. Keep the robot still while the gyro bias is measured "
                    "(%zu samples).",
                    cfg_.calibration_samples);
    }

   private:
    // ---- Configuration -------------------------------------------------------

    LidarRelayConfig load_config() {
        LidarRelayConfig c;
        require_mount_confirmed(*this);
        c.lidar_x_m = declare_finite(*this, "lidar_x_m", 0.0);
        c.lidar_y_m = declare_finite(*this, "lidar_y_m", 0.0);
        c.lidar_z_m = declare_finite(*this, "lidar_z_m", 0.0);
        c.lidar_roll_rad = declare_finite(*this, "lidar_roll_rad", 0.0);
        c.lidar_pitch_rad = declare_finite(*this, "lidar_pitch_rad", 0.0);
        c.lidar_yaw_rad = declare_finite(*this, "lidar_yaw_rad", 0.0);

        const int samples = declare_positive_int(*this, "calibration_samples", 200);
        if (samples < kMinCalibrationSamples) {
            throw std::invalid_argument("parameter 'calibration_samples' must be at least 2");
        }
        c.calibration_samples = static_cast<std::size_t>(samples);
        c.max_calibration_stddev_rps = declare_positive(*this, "max_calibration_stddev_rps", 0.02);
        c.max_abs_gyro_rps = declare_positive(*this, "max_abs_gyro_rps", 10.0);
        c.gyro_variance = declare_positive(*this, "gyro_variance", 1.0e-3);

        c.cloud_input_topic = declare_nonempty(*this, "cloud_input_topic", "unilidar/cloud");
        c.cloud_output_topic = declare_nonempty(*this, "cloud_output_topic", "lidar/cloud_base");
        c.imu_input_topic = declare_nonempty(*this, "imu_input_topic", "unilidar/imu");
        c.imu_output_topic = declare_nonempty(*this, "imu_output_topic", "imu/gyro");
        c.output_frame = declare_nonempty(*this, "output_frame", "base_link");
        return c;
    }

    // ---- Cloud ---------------------------------------------------------------

    void on_cloud(sensor_msgs::msg::PointCloud2::UniquePtr msg) {
        try {
            // Step through each point's x/y/z (throws if the cloud has no x/y/z fields).
            sensor_msgs::PointCloud2Iterator<float> it_x(*msg, "x");
            sensor_msgs::PointCloud2Iterator<float> it_y(*msg, "y");
            sensor_msgs::PointCloud2Iterator<float> it_z(*msg, "z");
            for (; it_x != it_x.end(); ++it_x, ++it_y, ++it_z) {
                const Vec3 raw{*it_x, *it_y, *it_z};
                if (!is_finite(raw)) {
                    continue;  // left as is; KISS-ICP drops invalid points itself
                }
                // Rotate by the mount angles, then shift by the mount position.
                const Vec3 r = rotate_to_base(raw, cfg_.lidar_roll_rad, cfg_.lidar_pitch_rad,
                                              cfg_.lidar_yaw_rad);
                *it_x = static_cast<float>(r.x + cfg_.lidar_x_m);
                *it_y = static_cast<float>(r.y + cfg_.lidar_y_m);
                *it_z = static_cast<float>(r.z + cfg_.lidar_z_m);
            }
        } catch (const std::exception& e) {
            RCLCPP_ERROR_THROTTLE(get_logger(), *get_clock(), kLogThrottleMs,
                                  "Could not read point cloud, dropping it: %s", e.what());
            return;
        }
        msg->header.frame_id = cfg_.output_frame;  // points are now in robot axes
        cloud_pub_->publish(std::move(msg));
    }

    // ---- IMU -----------------------------------------------------------------

    // Reject glitches -> rotate into robot axes -> calibrate or remove bias -> publish.
    void on_imu(const sensor_msgs::msg::Imu& msg) {
        const Vec3 raw{msg.angular_velocity.x, msg.angular_velocity.y, msg.angular_velocity.z};
        if (!is_finite(raw) || max_abs(raw) > cfg_.max_abs_gyro_rps) {
            RCLCPP_ERROR_THROTTLE(get_logger(), *get_clock(), kLogThrottleMs,
                                  "Rejected implausible gyro reading (%.3f, %.3f, %.3f rad/s)",
                                  raw.x, raw.y, raw.z);
            return;
        }

        Vec3 base;
        try {
            base =
                rotate_to_base(raw, cfg_.lidar_roll_rad, cfg_.lidar_pitch_rad, cfg_.lidar_yaw_rad);
        } catch (const std::exception& e) {
            RCLCPP_ERROR_THROTTLE(get_logger(), *get_clock(), kLogThrottleMs,
                                  "Gyro axis correction failed: %s", e.what());
            return;
        }

        if (!calibrated_) {
            calibrate(base);
            return;
        }

        Vec3 corrected;
        corrected.x = base.x - bias_.x;
        corrected.y = base.y - bias_.y;
        corrected.z = base.z - bias_.z;
        publish_gyro(corrected);
    }

    // Average readings while still; if they're too shaky (robot moved), start over.
    // Note: a perfectly steady turn would pass as bias, hence "keep still at startup".
    void calibrate(const Vec3& sample) {
        estimator_.add_sample(sample);
        if (!estimator_.complete()) {
            return;
        }
        if (estimator_.stable()) {
            bias_ = estimator_.bias();
            calibrated_ = true;
            RCLCPP_INFO(get_logger(), "Gyro calibration done. Bias (rad/s): x %.5f, y %.5f, z %.5f",
                        bias_.x, bias_.y, bias_.z);
        } else {
            RCLCPP_ERROR(get_logger(),
                         "Gyro too noisy during calibration (spread %.4f > %.4f rad/s). Was the "
                         "robot moving? Starting over.",
                         estimator_.max_stddev_observed(), cfg_.max_calibration_stddev_rps);
            estimator_.reset();
        }
    }

    void publish_gyro(const Vec3& gyro) {
        sensor_msgs::msg::Imu out;
        out.header.stamp = this->now();  // driver timestamps may not match this computer's clock
        out.header.frame_id = cfg_.output_frame;

        // Covariance -1 = "no data" (ROS convention), so the EKF skips these.
        out.orientation.w = 1.0;
        out.orientation_covariance[0] = -1.0;
        out.linear_acceleration_covariance[0] = -1.0;

        out.angular_velocity.x = gyro.x;
        out.angular_velocity.y = gyro.y;
        out.angular_velocity.z = gyro.z;
        out.angular_velocity_covariance[0] = cfg_.gyro_variance;  // 3x3 diagonal: 0, 4, 8
        out.angular_velocity_covariance[4] = cfg_.gyro_variance;
        out.angular_velocity_covariance[8] = cfg_.gyro_variance;
        imu_pub_->publish(out);
    }

    // ---- State ---------------------------------------------------------------
    LidarRelayConfig cfg_;
    BiasEstimator estimator_;
    bool calibrated_{false};
    Vec3 bias_;

    rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr cloud_sub_;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr cloud_pub_;
    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
};

}  // namespace odometry

// Exits with code 1 and a clear message if a setting is invalid.
int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    int exit_code = 0;
    try {
        rclcpp::spin(std::make_shared<odometry::LidarRelayNode>());
    } catch (const std::exception& e) {
        RCLCPP_FATAL(rclcpp::get_logger("lidar_relay"), "Fatal: %s", e.what());
        exit_code = 1;
    }
    rclcpp::shutdown();
    return exit_code;
}
