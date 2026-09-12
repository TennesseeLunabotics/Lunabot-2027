cmake_minimum_required(VERSION 3.10)
project(shovel VERSION 1.0 LANGUAGES CXX)

if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(robot_common REQUIRED)

# executable target
add_executable(shovel_node src/shovel_node.cpp)

# ink dependencies
ament_target_dependencies(shovel_node 
  rclcpp 
  sensor_msgs 
  robot_common
)

# Install executable
install(TARGETS shovel_node
  EXPORT export_${PROJECT_NAME}
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()