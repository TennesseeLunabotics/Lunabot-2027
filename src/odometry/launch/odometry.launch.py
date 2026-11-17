"""
    Odometry stack: lidar relay + KISS-ICP, encoder odometry, and the EKF.

    unilidar/cloud -> lidar_relay -> lidar/cloud_base -> kiss_icp_node -> kiss/odometry -> EKF
    drivetrain_sensor_output -> odometry -> encoder/odom -> EKF
    unilidar/imu -> lidar_relay -> imu/gyro -> EKF
    EKF -> odometry/filtered + odom -> base_link
"""
from launch import LaunchDescription
from launch_ros.actions import Node

# CONFIG
# REQUIRED values must be filled in or the nodes refuse to start.
# Robot axes: x forward, y left, z up; origin on the ground under the robot's center.
# Units are in the names (_m, _rad, _s, _mps, _rps).

# Lidar mount, used by the lidar relay for both the cloud and the gyro (REQUIRED).
# Check: turning counter-clockwise gives positive /imu/gyro z, and in RViz (fixed frame
# base_link) the wall in front of the robot is at +x on /lidar/cloud_base.
LIDAR_MOUNT = {
    'lidar_mount_confirmed': False,  # set to True only after measuring the six values below
    'lidar_x_m': 0.0,                # lidar center relative to base_link
    'lidar_y_m': 0.0,
    'lidar_z_m': 0.0,                # height above the ground
    'lidar_roll_rad': 0.0,           # applied roll (x), then pitch (y), then yaw (z)
    'lidar_pitch_rad': 0.0,
    'lidar_yaw_rad': 0.0,
}

ENCODER_CONFIG = {
    # Drive geometry (REQUIRED)
    'sprocket_radius_m': 0.0,                # sprocket that drives the tread
    'position_units_per_sprocket_rev': 0.0,  # encoder position change per sprocket turn
                                             # (the gearbox ratio if position is in motor rotations)
    # -1 if that side's position decreases when driving forward (the drivetrain
    # inverts the right motor in software, so check before choosing).
    'left_sign': 1,
    'right_sign': 1,
    'twist_linear_variance': 0.5,  # bigger = EKF trusts it less; loose because treads slip
    'max_tread_speed_mps': 3.0,    # faster = bad reading, ignored
    'velocity_window_s': 0.1,      # average speed over at least this long
    'max_message_gap_s': 0.5,      # longer silence = start over
    'joint_state_topic': 'drivetrain_sensor_output',
    'odom_topic': 'encoder/odom',
    'left_joint_name': 'Left',
    'right_joint_name': 'Right',
    'odom_frame': 'odom',
    'base_frame': 'base_link',
}

LIDAR_RELAY_CONFIG = {
    # Gyro bias is measured at startup; the robot must be still.
    'calibration_samples': 200,
    'max_calibration_stddev_rps': 0.02,  # more spread = robot moved, start over
    'max_abs_gyro_rps': 10.0,            # faster = glitch, ignored
    'gyro_variance': 0.001,              # bigger = EKF trusts it less
    'cloud_input_topic': 'unilidar/cloud',
    'cloud_output_topic': 'lidar/cloud_base',  # read by KISS-ICP
    'imu_input_topic': 'unilidar/imu',
    'imu_output_topic': 'imu/gyro',
    'output_frame': 'base_link',
}

# KISS-ICP (github.com/PRBonn/kiss-icp, built by the Dockerfile). Its input cloud is
# already in base_link (lidar_relay), so no TF lookup is needed.
KISS_ICP_CONFIG = {
    'base_frame': '',                # empty = use the cloud's own frame (base_link)
    'lidar_odom_frame': 'odom',
    'publish_odom_tf': False,        # the EKF publishes odom -> base_link
    'publish_debug_clouds': False,   # True to view kiss/frame and kiss/local_map in RViz
    'position_covariance': 0.01,     # bigger = EKF trusts it less
    'orientation_covariance': 0.01,
    'data.deskew': True,             # undo motion blur using each point's time
    'data.max_range': 15.0,          # meters
    'data.min_range': 0.6,           # meters from base_link; drops the robot's own body
    'mapping.voxel_size': 0.1,       # meters; map grid size
    'mapping.max_points_per_voxel': 20,
    'adaptive_threshold.initial_threshold': 2.0,  # max point-matching distance; KISS-ICP adapts it
    'adaptive_threshold.min_motion_th': 0.1,
}

# robot_localization EKF: blends KISS-ICP, the gyro and encoders into odom -> base_link
# and /odometry/filtered. slam_toolbox (map -> odom) corrects the slow drift left over.
# Trust comes from each source's variance (smaller = trusted more).
# *_config: True = use this value from this source (each row is labeled).
EKF_CONFIG = {
    'frequency': 30.0,      # publish rate (Hz)
    'sensor_timeout': 0.5,  # seconds without input before the EKF only predicts
    'two_d_mode': True,     # ignore z, roll and pitch
    'publish_tf': True,
    'map_frame': 'map',
    'odom_frame': 'odom',
    'base_link_frame': 'base_link',
    'world_frame': 'odom',  # this EKF produces odometry, not a map position

    # Encoders: forward and sideways speed (encoder turning is unreliable on treads).
    'odom0': 'encoder/odom',
    'odom0_config': [False, False, False,   # x, y, z
                     False, False, False,   # roll, pitch, yaw
                     True,  True,  False,   # vx, vy, vz
                     False, False, False,   # vroll, vpitch, vyaw
                     False, False, False],  # ax, ay, az
    'odom0_queue_size': 10,

    # KISS-ICP: x, y and heading. differential = only the change between messages is used.
    'odom1': 'kiss/odometry',
    'odom1_config': [True,  True,  False,   # x, y, z
                     False, False, True,    # roll, pitch, yaw
                     False, False, False,   # vx, vy, vz
                     False, False, False,   # vroll, vpitch, vyaw
                     False, False, False],  # ax, ay, az
    'odom1_differential': True,
    'odom1_queue_size': 10,

    # Gyro: turning rate only.
    'imu0': 'imu/gyro',
    'imu0_config': [False, False, False,    # x, y, z
                    False, False, False,    # roll, pitch, yaw
                    False, False, False,    # vx, vy, vz
                    False, False, True,     # vroll, vpitch, vyaw
                    False, False, False],   # ax, ay, az
    'imu0_queue_size': 50,
    'imu0_remove_gravitational_acceleration': False,
}


def generate_launch_description() -> LaunchDescription:
    return LaunchDescription(
        [
            Node(
                package="odometry",
                executable="lidar_relay_node",
                name="lidar_relay",
                output="screen",
                parameters=[LIDAR_MOUNT, LIDAR_RELAY_CONFIG],
            ),
            Node(
                package="kiss_icp",
                executable="kiss_icp_node",
                name="kiss_icp_node",
                output="screen",
                # KISS-ICP always reads "pointcloud_topic", so point it at the relay's output.
                remappings=[("pointcloud_topic", "lidar/cloud_base")],
                parameters=[KISS_ICP_CONFIG],
            ),
            Node(
                package="odometry",
                executable="odometry_node",
                name="odometry",
                output="screen",
                parameters=[ENCODER_CONFIG],
            ),
            Node(
                package="robot_localization",
                executable="ekf_node",
                name="ekf_filter_node",
                output="screen",
                parameters=[EKF_CONFIG],
            ),
        ]
    )
