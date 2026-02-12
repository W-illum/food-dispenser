#pragma once
#include <Arduino.h>
#include "SparkFun_Qwiic_OLED.h"
#include "rotary_encoder.h"
#include "tmc_stepper.h"
#include "menu.h"
#include "feeding_schedule.h"

#define LIDAR_PWM   GPIO_NUM_6  // D3
#define TMC_DIR     GPIO_NUM_7  // D4
#define TMC_STEP    GPIO_NUM_8  // D5
#define TMC_MS2     GPIO_NUM_9  // D6
#define TMC_MS1     GPIO_NUM_10 // D7
#define TMC_EN      GPIO_NUM_17 // D8
#define ENCODER_SW  GPIO_NUM_18 // D9
#define ENCODER_A   GPIO_NUM_21 // D10
#define ENCODER_B   GPIO_NUM_38 // D11
#define SDA         GPIO_NUM_11 // A4
#define SCL         GPIO_NUM_12 // A5

class Feeder
{
public:
    Feeder();
    void begin();
    void update();

private:
    Qwiic1in3OLED lcd;
    Encoder enc;
    TMCStepper stepper;
    Menu menu;
    FeedingSchedule schedule;

    uint8_t food_amount = 0;
};
