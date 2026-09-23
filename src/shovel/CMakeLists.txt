cmake_minimum_required(VERSION 3.11)      # required for FetchContent
project(shovel VERSION 1.0 LANGUAGES CXX)

if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

# NOTE: do NOT call add_compile_options() globally before FetchContent —
# it leaks -Wall/-Wextra/-Wpedantic into third-party targets and can
# turn their warnings into build failures. Apply warnings per-target
# to your own executables only (see bottom of file).

include(FetchContent)

# --- Communication with Jetson GPIO pins ---
FetchContent_Declare(
  JetsonGPIO
  GIT_REPOSITORY https://github.com/pjueon/JetsonGPIO.git
  GIT_TAG master   # pin to a commit/tag once confirmed working
)

# --- JSON library (header-only, minimal fetch via release tarball) ---
FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)

# Mark both EXCLUDE_FROM_ALL so their targets are only built when actually
# linked against (not part of the default ALL target) and, critically,
# NOT part of `ament_package()`'s install step. Without this, JetsonGPIO's
# and nlohmann_json's own install()/export rules get pulled into your
# package's colcon install, which is what typically breaks the build
# inside a ROS 2 / colcon container even though a plain `cmake --build`
# might succeed.
#
# IMPORTANT: FetchContent_GetProperties() alone does NOT download or
# configure anything -- it only checks whether content has already been
# populated. You MUST call FetchContent_Populate() (or MakeAvailable)
# to actually create the JetsonGPIO::JetsonGPIO / nlohmann_json::nlohmann_json
# targets. Skipping Populate()/add_subdirectory() is why those targets
# were reported as "not found" at add_executable() time.
FetchContent_GetProperties(JetsonGPIO)
if(NOT jetsongpio_POPULATED)
  FetchContent_Populate(JetsonGPIO)
  add_subdirectory(${jetsongpio_SOURCE_DIR} ${jetsongpio_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
  add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# --- ament / ROS 2 packages ---
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(robot_common REQUIRED)
find_package(Threads REQUIRED)   # -lpthread required for JetsonGPIO

# Do NOT also call find_package(JetsonGPIO) / find_package(json) here --
# neither is installed as a system CMake package; those calls will
# silently no-op (no REQUIRED) and contribute nothing. The targets come
# exclusively from the FetchContent_Populate()+add_subdirectory() above.

# --- Bundle non-ament (modern CMake / imported-target) deps into a single
# INTERFACE library. This is the key fix: CMake forbids mixing the
# "keyword" (PRIVATE/PUBLIC/INTERFACE) and "plain" signatures of
# target_link_libraries() on the SAME target. ament_target_dependencies()
# internally calls the PLAIN signature on shovel_node, so if you also call
# target_link_libraries(shovel_node PRIVATE ...) directly on shovel_node,
# you get:
#   "The keyword signature for target_link_libraries has already been
#    used with the target shovel_node. All uses ... must be either
#    all-keyword or all-plain."
# By attaching the keyword-form calls to a separate INTERFACE target
# instead, shovel_node itself is only ever touched by the plain-signature
# ament_target_dependencies() call, so there's no conflict.
add_library(shovel_external_deps INTERFACE)
target_link_libraries(shovel_external_deps INTERFACE
  JetsonGPIO::JetsonGPIO
  nlohmann_json::nlohmann_json
  Threads::Threads
)

# --- executables ---
add_executable(arduino_client src/arduino_client.cpp)
add_executable(shovel_node src/shovel_node.cpp)

# apply your own warning flags only to your own targets
foreach(tgt shovel_node arduino_client)
  if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${tgt} PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endforeach()

# ament-exported ROS packages -> ament_target_dependencies.
# This is the ONLY target_link_libraries-family call that touches
# shovel_node directly, and it uses the plain signature internally,
# so it must come before/instead of any keyword call on this target.
ament_target_dependencies(shovel_node
  rclcpp
  sensor_msgs
  robot_common
)

# Non-ament imported targets go in via the INTERFACE bundle target
# (keyword call, but on shovel_external_deps, not on shovel_node itself).
target_link_libraries(shovel_node shovel_external_deps)

target_link_libraries(arduino_client PRIVATE
  JetsonGPIO::JetsonGPIO
  nlohmann_json::nlohmann_json
)

# Install executables. No EXPORT set here since we aren't exporting
# a CMake package config for other packages to find_package() this one.
install(TARGETS shovel_node arduino_client
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
