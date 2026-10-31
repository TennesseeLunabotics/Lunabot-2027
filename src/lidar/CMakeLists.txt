cmake_minimum_required(VERSION 3.8)
project(lidar)
cmake_policy(SET CMP0074 NEW) # ignore CMP0074 warning from PCL_ROOT

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(robot_common REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(PCL REQUIRED)
find_package(tf2_ros REQUIRED)

include_directories(
  ${PCL_INCLUDE_DIRS} 
  include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

link_directories(
  ${PCL_LIBRARY_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR}
)

add_definitions(${PCL_DEFINITIONS})

add_executable(lidar_node src/lidar_node.cpp)

target_link_libraries( lidar_node 
  ${Boost_SYSTEM_LIBRARY}
  ${PCL_LIBRARIES}
  libunilidar_sdk2.a
)

install(
    DIRECTORY include/ 
    DESTINATION include
)

# Link the ROS 2 dependencies
ament_target_dependencies(lidar_node
  rclcpp
  sensor_msgs
  robot_common
  pcl_conversions # required by lidar sdk
  tf2_ros
  geometry_msgs
)

# Install the executable so ROS 2 can find it using `ros2 run lidar lidar_node`
install(TARGETS lidar_node
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
