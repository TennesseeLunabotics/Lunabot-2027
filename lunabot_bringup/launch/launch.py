import os
import subprocess
import time
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import ExecuteProcess, IncludeLaunchDescription, OpaqueFunction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node

# CONFIG
STATIC_TF_CONFIG = {
    'x': 0.0,
    'y': 0.0,
    'z': 0.0,
    'yaw': 0.0,
    'pitch': 0.0,
    'roll': 0.0,
    'frame_id': 'odom',
    'child_frame_id': 'base_link',
}

SLAM_CONFIG = {
    'odom_frame': 'odom',
    'base_frame': 'base_link',
    'map_frame': 'map',
    'scan_topic': '/scan',
    'use_sim_time': False,
    'transform_timeout': 2.0,
    'tf_buffer_duration': 60.0,
    'map_update_interval': 1.0,
    'minimum_travel_distance': 0.0,
    'minimum_travel_heading': 0.0,
    'throttle_scans': 1
}

LIDAR_CONFIG = {
    'port': '/dev/ttyUSB0',
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

def wait_for_can0_up(context, *args, **kwargs):
    # trying to bring up the CAN interface
    subprocess.run(
        ['sudo', 'ip', 'link', 'set', 'can0', 'up', 'type', 'can', 'bitrate', '1000000'],
        capture_output=True,
        text=True
    )
    print("Waiting for CAN interface can0 to come up...")

    for _ in range(10):
        result = subprocess.run(['ip', 'link', 'show', 'can0'], capture_output=True, text=True)
        if 'state UP' in result.stdout or 'UP' in result.stdout:
            print("CAN interface can0 is up.")
            break
        time.sleep(1)
    else:
        print("Timeout: CAN interface can0 is not up after 10 seconds.")

    # Launch drivetrain node after CAN interface check
    return [
        Node(
            package="drivetrain",
            executable="drivetrain_node",
            output="screen",
        )
    ]

def create_static_tf_args(config: dict) -> list[str]:
    """it translate TF config dictionary into CLI arguments for tf2_ros."""
    args = []
    for key, value in config.items():
        cli_flag = f"--{key.replace('_', '-')}"
        args.extend([cli_flag, str(value)])
    return args

def generate_launch_description():
    # find package share paths dynamically
    bringup_pkg_share = get_package_share_directory('lunabot_bringup')

    teleop_node = Node(
        package="teleop",
        executable="teleop_node",
        output="screen",
    )

    shovel_node = Node(
        package="shovel",
        executable="shovel_node",
        output="screen",
    )

    static_tf_node = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='static_transform_publisher',
        arguments=create_static_tf_args(STATIC_TF_CONFIG)
    )

    lidar_node = Node(
        package='lidar',
        executable='lidar_node',
        name='lidar_node',
        output='screen',
        parameters=[LIDAR_CONFIG]
    )

    cloud_to_scan_node = Node(
        package='lunabot_bringup',
        executable='cloud_to_scan',
        name='cloud_to_scan',
        output='screen'
    )

    slam_node = Node(
        package='slam_toolbox',
        executable='async_slam_toolbox_node',
        name='slam_toolbox',
        output='screen',
        parameters=[SLAM_CONFIG]
    )

    ld = LaunchDescription()

    ld.add_action(shovel_node)
    ld.add_action(teleop_node)
    ld.add_action(OpaqueFunction(function=wait_for_can0_up))

    ld.add_action(static_tf_node)
    ld.add_action(lidar_node)
    ld.add_action(cloud_to_scan_node)
    ld.add_action(slam_node)

    return ld
