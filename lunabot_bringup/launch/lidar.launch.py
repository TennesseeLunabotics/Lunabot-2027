from launch import LaunchDescription
from launch_ros.actions import Node

LIDAR_PARAMS = {
    'initialize_type': 1,               # 2 = Ethernet (UDP)
    'work_mode': 8,                     # Switches internal mode to UART/TTL
    'serial_port': '/dev/ttyACM0',
    'baudrate': 4000000,
    'rotate_yaw_bias': 0.0,
    'range_scale': 0.001,
    'range_bias': 0.0,
    'range_max': 50.0,
    'range_min': 0.0,
    'cloud_frame': 'unilidar_lidar',
    'cloud_topic': 'unilidar/cloud',
    'cloud_scan_num': 10,
    'imu_frame': 'unilidar_imu',
    'imu_topic': 'unilidar/imu',
}

SLAM_CONFIG = {
    'odom_frame': 'odom',
    'base_frame': 'base_link',
    'map_frame': 'map',
    'scan_topic': '/scan',
    'mode': 'mapping',
    'use_sim_time': False,
    'transform_timeout': 1.0,
    'tf_buffer_duration': 60.0,
    'map_update_interval': 0.5,
    'minimum_travel_distance': 0.0,
    'minimum_travel_heading': 0.0,
    'throttle_scans': 1
}

def generate_launch_description():
    nodes = []

    # 1. Unitree LiDAR Driver Node
    nodes.append(
        Node(
            package='lidar',
            executable='lidar_node',
            name='lidar_node',
            output='screen',
            parameters=[LIDAR_PARAMS]
        )
    )

    # 2. PointCloud2 to LaserScan Converter
    nodes.append(
        Node(
            package='lunabot_bringup',
            executable='cloud_to_scan',
            name='cloud_to_scan',
            output='screen',
            parameters=[{
                'target_frame': 'unilidar_lidar',  # Ensures /scan uses the sensor frame
                'range_min': 0.15,                 # Filters out chassis reflections (<30 cm)
                'range_max': 30.0,
                'min_height': -0.15,
                'max_height': 0.5,
                'angle_min': -3.14159,
                'angle_max': 3.14159,
                'angle_increment': 0.0087
            }]
        )
    )

    # 3. Static Transform: odom -> base_link
    nodes.append(
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            name='odom_to_base_link',
            arguments=[
                '--x', '0.0', '--y', '0.0', '--z', '0.0',
                '--roll', '0.0', '--pitch', '0.0', '--yaw', '0.0',
                '--frame-id', 'odom',
                '--child-frame-id', 'base_link'
            ]
        )
    )

    # 4. Static Transform: base_link -> unilidar_imu_initial (Root of Unitree sensor tree)
    nodes.append(
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            name='base_link_to_imu_initial',
            arguments=[
                '--x', '0.0', '--y', '0.0', '--z', '0.0',
                '--roll', '0.0', '--pitch', '0.0', '--yaw', '0.0',
                '--frame-id', 'base_link',
                '--child-frame-id', 'unilidar_imu_initial'
            ]
        )
    )

    # 5. SLAM Toolbox
    nodes.append(
        Node(
            package='slam_toolbox',
            executable='async_slam_toolbox_node',
            name='slam_toolbox',
            output='screen',
            parameters=[SLAM_CONFIG]
        )
    )

    return LaunchDescription(nodes)
