#pragma once
#include <Arduino.h>

#define BTN_DEBOUNCE_US 350000

enum class EncoderEvent : uint8_t {
    NONE,
    CW,
    CCW,
    BTN
};

enum class RotationState : uint8_t {
    IDLE,
    CW1,
    CW2,
    CW3,
    CCW1,
    CCW2,
    CCW3
};

class Encoder
{
public:
    Encoder( int8_t pin_sw, int8_t pin_a, int8_t pin_b );
    void begin();
    EncoderEvent checkUpdate();

private:
    static void isr();

private:
    static Encoder* instance;
    int8_t pin_sw, pin_a, pin_b;

    volatile bool event_flag = false;

    volatile EncoderEvent pending_event = EncoderEvent::NONE;
    volatile RotationState current_state = RotationState::IDLE;

    volatile uint32_t last_btn_us = 0;
};