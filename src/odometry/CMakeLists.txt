# Builds the two odometry nodes and installs launch/.
cmake_minimum_required(VERSION 3.8)
project(odometry)

# Warnings on, but not errors, so a new compiler warning can't break the team's build.
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)

# Drive encoders -> forward speed.
add_executable(odometry_node src/odometry_node.cpp)
target_include_directories(odometry_node PRIVATE include)
ament_target_dependencies(odometry_node rclcpp sensor_msgs nav_msgs)

# Lidar cloud -> robot axes (for KISS-ICP), and lidar's IMU -> bias-corrected gyro.
add_executable(lidar_relay_node src/lidar_relay_node.cpp)
target_include_directories(lidar_relay_node PRIVATE include)
ament_target_dependencies(lidar_relay_node rclcpp sensor_msgs)

install(TARGETS odometry_node lidar_relay_node DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})

ament_package()
