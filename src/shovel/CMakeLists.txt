cmake_minimum_required(VERSION 3.11)      # updated to use FetchContent
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

include(FetchContent)

# Communication with Jetson GPIO pins
FetchContent_Declare(
  JetsonGPIO
  GIT_REPOSITORY https://github.com/pjueon/JetsonGPIO.git 
  GIT_TAG master # change to a specific branch later after we confirm this works
)

# Json libraries
# - needed to be different due to how large the json lirbary is 
# - takes in only neccessary bit required for the code
FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)

FetchContent_GetProperties(json)
FetchContent_GetProperties(JetsonGPIO)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(robot_common REQUIRED)
find_package(Threads REQUIRED)
find_package(JetsonGPIO)
find_package(json)

# executable target
add_executable(arduino_client src/arduino_client.cpp)

add_executable(shovel_node src/shovel_node.cpp)
# external 
target_link_libraries(shovel_node
  JetsonGPIO::JetsonGPIO
  nlohmann_json::nlohmann_json
  Threads::Threads
)

# ink dependencies
ament_target_dependencies(shovel_node 
  rclcpp 
  sensor_msgs 
  robot_common
)

target_link_libraries(arduino_client PRIVATE
  nlohmann_json::nlohmann_json
)

# Install executable
install(TARGETS shovel_node arduino_client
  EXPORT export_${PROJECT_NAME}
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
