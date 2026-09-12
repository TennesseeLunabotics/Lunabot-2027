cmake_minimum_required(VERSION 3.8)
project(robot_common)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)

add_library(sparkcan SHARED
    src/PIDController.cpp
    src/SparkBase.cpp
    src/SparkMax.cpp 
    src/SparkFlex.cpp
)

target_include_directories(sparkcan 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

ament_target_dependencies(sparkcan
    rclcpp
    sensor_msgs
)

# add_executable(drivetrain_node src/drivetrain_node.cpp)
# target_include_directories(drivetrain_node PUBLIC
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#     $<INSTALL_INTERFACE:include>
# )
# ament_target_dependencies(drivetrain_node rclcpp sensor_msgs sparkcan)
# install(TARGETS drivetrain_node DESTINATION lib/${PROJECT_NAME})

install(
    TARGETS sparkcan
    EXPORT sparkcanTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(
    DIRECTORY include/ 
    DESTINATION include
)

install(EXPORT sparkcanTargets
    FILE sparkcanTargets.cmake
    NAMESPACE sparkcan::
    DESTINATION lib/cmake/sparkcan
)

ament_export_targets(sparkcanTargets)
ament_export_dependencies(rclcpp sensor_msgs)

ament_package()