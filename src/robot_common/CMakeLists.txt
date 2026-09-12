cmake_minimum_required(VERSION 3.10)
project(robot_common)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)

add_library(robot_common SHARED
    src/PIDController.cpp
    src/SparkBase.cpp
    src/SparkMax.cpp 
    src/SparkFlex.cpp
)

target_include_directories(robot_common 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

ament_target_dependencies(robot_common
    rclcpp
    sensor_msgs
)

install(
    TARGETS robot_common
    EXPORT robot_commonTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(
    DIRECTORY include/ 
    DESTINATION include
)

install(EXPORT robot_commonTargets
    FILE robot_commonTargets.cmake
    NAMESPACE robot_common::
    DESTINATION lib/cmake/robot_common
)

ament_export_targets(robot_commonTargets)
ament_export_dependencies(rclcpp sensor_msgs)
ament_export_include_directories(include)

ament_package()