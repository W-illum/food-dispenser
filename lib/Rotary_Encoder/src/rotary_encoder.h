#pragma once
#include <Arduino.h>

#define BTN_DEBOUNCE_US 350000

enum class EncoderEvent : uint8_t {
    None,
    CW,
    CCW,
    BTN
};

enum class RotationState : uint8_t {
    Idle,
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
    Encoder(int8_t pin_sw, int8_t pin_a, int8_t pin_b);
    void begin();
    EncoderEvent checkUpdate();

private:
    static void isr();

private:
    static Encoder* instance;
    int8_t sw, a, b;

    volatile bool event_flag = false;

    volatile EncoderEvent pending_event = EncoderEvent::None;
    volatile RotationState current_state = RotationState::Idle;

    volatile uint32_t last_btn_us = 0;
};