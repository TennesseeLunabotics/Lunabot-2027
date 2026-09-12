cmake_minimum_required(VERSION 3.8)
project(robot_common)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(sparkcan SHARED
    src/PIDController.cpp
    src/SparkBase.cpp
    src/SparkMax.cpp 
    src/SparkFlex.cpp
)

find_package(ament_cmake REQUIRED)
find_package(sensor_msgs REQUIRED)

target_include_directories(sparkcan PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(drivetrain_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

ament_target_dependencies(drivetrain_node rclcpp sensor_msgs)
# Install the library
install(TARGETS sparkcan
    EXPORT sparkcanTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install the header files
install(DIRECTORY include/ DESTINATION include)

# Export the targets
install(EXPORT sparkcanTargets
    FILE sparkcanTargets.cmake
    NAMESPACE sparkcan::
    DESTINATION lib/cmake/sparkcan
)

# Export include directories to downstream packages
ament_export_include_directories(include)
ament_export_dependencies(sensor_msgs)

ament_package()