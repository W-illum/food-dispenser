#pragma once
#include <Arduino.h>

class TMCStepper
{
public:
    TMCStepper( int8_t pin_dir, int8_t pin_step, int8_t pin_ms2, int8_t pin_ms1, int8_t pin_en );
    void begin();
    bool setMicroStep( int step );
    void turnCW();
    void turnCCW();

private:
    int8_t pin_dir, pin_step, pin_ms2, pin_ms1, pin_en;
    int micro_step;
};