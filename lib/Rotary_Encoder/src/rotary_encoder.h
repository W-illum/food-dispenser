#pragma once
#include <Arduino.h>

enum class EncoderEvent : uint8_t {
    None,
    CCW,
    CW,
    BTN
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

    volatile uint32_t last_isr_us = 0;
    volatile uint32_t last_btn_us = 0;
};