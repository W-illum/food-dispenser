#pragma once
#include <Arduino.h>
#include "SparkFun_Qwiic_OLED.h"
#include "rotary_encoder.h"
#include "tmc_stepper.h"
#include "menu.h"
#include "feeding_schedule.h"
#include "Ds1302.h"

#define DS_RST      GPIO_NUM_47 // D12
#define DS_DAT      GPIO_NUM_38 // D11
#define DS_CLK      GPIO_NUM_21 // D10
#define ENCODER_SW  GPIO_NUM_10 // D7
#define ENCODER_B   GPIO_NUM_9  // D6
#define ENCODER_A   GPIO_NUM_8  // D5
#define LIDAR_PWM   GPIO_NUM_7  // D4
#define LCD_SCL     GPIO_NUM_6  // D3
#define LCD_SDA     GPIO_NUM_5  // D2

#define TMC_DIR     GPIO_NUM_3  // A2
#define TMC_STEP    GPIO_NUM_4  // A3
#define TMC_MS2     GPIO_NUM_11 // A4
#define TMC_MS1     GPIO_NUM_12 // A5
#define TMC_EN      GPIO_NUM_13 // A6

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
    Ds1302 rtc;

    uint8_t food_amount = 0;
    Ds1302::DateTime dt;
    Time_t system_clock;
};
