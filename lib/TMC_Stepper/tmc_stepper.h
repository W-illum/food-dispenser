#pragma once
#include <Arduino.h>

enum class Dir : uint8_t {
    CCW,
    CW
};

class TMCStepper
{
public:
    TMCStepper( gpio_num_t DIR, gpio_num_t STEP, gpio_num_t MS2, gpio_num_t MS1, gpio_num_t EN );
    void begin();
    bool setMicroStep( int step );
    void rotate( uint deg, Dir dir );
    static void IRAM_ATTR timerISR();

public:
    volatile uint32_t remaining_steps = 0;

private:
    gpio_num_t DIR, STEP, MS2, MS1, EN;
    int micro_step;
    int steps_per_rev = 200;

    static TMCStepper *instance;
    static hw_timer_t *timer;
};