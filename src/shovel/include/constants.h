#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace CONSTANTS {
    inline constexpr int ARM_PWM[2] = {26, 27};
    inline constexpr int ARM_DIR[2] = {16, 17};
    inline constexpr int ARM_ADC[2] = {35, 32};

    inline constexpr int ARM_INCHES_PER_BIT = 6 / 4096;
    /* ARM_P = .2 * 255 * ARM_INCHES_PER_BIT #percent output / inch disparity */
    /* TODO check if this should be a double or a int, ARM_INCHES_PER_BIT float issue */
    inline constexpr int ARM_P = 4 * 255 * ARM_INCHES_PER_BIT; // percent output / inch disparity

    inline constexpr int SCOOP_PWM = 13;
    inline constexpr int SCOOP_DIR = 14;
    inline constexpr int SCOOP_ADC = 33;

    inline constexpr int BUCKET_PWM = 25;
    inline constexpr int BUCKET_DIR = 23;

    inline constexpr int ARM_DUTY_CYCLE = 200;
    inline constexpr int BUCKET_DUTY_CYCLE = 255;
    inline constexpr int SCOOP_DUTY_CYCLE = 255;

    inline constexpr double ARM_LENGTH = 31.5;
    inline constexpr double ARM_LIN_DIST = 6.5;
    inline constexpr int LIN_UNEXTENDED_DIST = 16;


    /* Control Buttons */
    inline constexpr char ARM_FORWARD = 'f';
    inline constexpr char ARM_BACKWARD = 'b';

    inline constexpr char BUCKET_FORWARD = 'f';
    inline constexpr char BUCKET_BACKWARD = 'b';

    inline constexpr char SCOOP_FORWARD = 'f';
    inline constexpr char SCOOP_BACKWARD = 'b';
}
#endif 

