/**
 * @file SparkFlex.cpp
 * @brief Source file for the SparkFlex class for controlling REV Robotics SPARK
 * Flex controllers
 * @author Grayson Arendt
 */

#include "robot_common/SparkFlex.hpp"

SparkFlex::SparkFlex(const std::string& interfaceName, uint8_t deviceId)
    : SparkBase(interfaceName, deviceId) {}
