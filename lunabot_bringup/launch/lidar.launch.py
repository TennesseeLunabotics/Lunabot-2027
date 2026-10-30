from launch import LaunchDescription
from launch_ros.actions import Node

LIDAR_PARAMS = {
    'initialize_type': 1,
    'serial_port': '/dev/ttyUSB0',
    'baudrate': 4000000,
    'rotate_yaw_bias': 0.0,
    'range_scale': 0.001,
    'range_bias': 0.0,
    'range_max': 50.0,
    'range_min': 0.0,
    'cloud_frame': 'unilidar_lidar',
    'cloud_topic': 'unilidar/cloud',
    'cloud_scan_num': 18,
    'imu_frame': 'unilidar_imu',
    'imu_topic': 'unilidar/imu',
}

STATIC_TRANSFORMS = [
    {
        'name': 'odom_to_base_link',
        'x': 0.0, 'y': 0.0, 'z': 0.0,
        'yaw': 0.0, 'pitch': 0.0, 'roll': 0.0,
        'frame_id': 'odom',
        'child_frame_id': 'base_link',
    },
    {
        'name': 'base_link_to_lidar',
        'x': 0.0, 'y': 0.0, 'z': 0.0,
        'yaw': 0.0, 'pitch': 0.0, 'roll': 0.0,
        'frame_id': 'base_link',
        'child_frame_id': 'unilidar_lidar',
    },
    {
        'name': 'lidar_to_imu',
        'x': 0.0, 'y': 0.0, 'z': 0.0,
        'yaw': 0.0, 'pitch': 0.0, 'roll': 0.0,
        'frame_id': 'unilidar_lidar',
        'child_frame_id': 'unilidar_imu',
    },
]

SLAM_CONFIG = {
    'odom_frame': 'odom',
    'base_frame': 'base_link',
    'map_frame': 'map',
    'scan_topic': '/scan',
    'mode': 'mapping',
    'use_sim_time': False,
    'transform_timeout': 2.0,
    'tf_buffer_duration': 60.0,
    'map_update_interval': 1.0,
    'minimum_travel_distance': 0.0,
    'minimum_travel_heading': 0.0,
    'throttle_scans': 1
}

def generate_launch_description():
    nodes = []

    for tf in STATIC_TRANSFORMS:
        nodes.append(
            Node(
                package='tf2_ros',
                executable='static_transform_publisher',
                name=tf['name'],
                arguments=[
                    str(tf['x']), str(tf['y']), str(tf['z']),
                    str(tf['yaw']), str(tf['pitch']), str(tf['roll']),
                    tf['frame_id'], tf['child_frame_id']
                ]
            )
        )

    nodes.append(
        Node(
            package='lidar',
            executable='lidar_node',
            name='lidar_node',
            output='screen',
            parameters=[LIDAR_PARAMS]
        )
    )

    nodes.append(
        Node(
            package='lunabot_bringup',
            executable='cloud_to_scan',
            name='cloud_to_scan',
            output='screen'
        )
    )

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
