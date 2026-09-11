cmake_minimum_required(VERSION 3.8)
project(robot_common)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)

# Install header files so other packages can include them
install(
  DIRECTORY include/
  DESTINATION include
)

# Export include directories to downstream packages
ament_export_include_directories(include)

ament_package()