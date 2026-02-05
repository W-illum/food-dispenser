#pragma once
#include <Arduino.h>

class TMCStepper
{
public:
    TMCStepper( gpio_num_t DIR, gpio_num_t STEP, gpio_num_t MS2, gpio_num_t MS1, gpio_num_t EN );
    void begin();
    bool setMicroStep( int step );
    void turnCW();
    void turnCCW();

private:
    gpio_num_t DIR, STEP, MS2, MS1, EN;
    int micro_step;
};