#include <Arduino.h>
#include "rotary_encoder.h"

Encoder* Encoder::instance = nullptr;

Encoder::Encoder(int8_t pin_sw, int8_t pin_a, int8_t pin_b)
    :   sw(pin_sw), a(pin_a), b(pin_b)
{
    instance = this;
}

void Encoder::begin()
{
    pinMode(sw, INPUT_PULLUP);
    pinMode(a,  INPUT_PULLUP);
    pinMode(b,  INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(sw), Encoder::isr, FALLING);
    attachInterrupt(digitalPinToInterrupt(a), Encoder::isr, RISING); // only need to check one of pin a or b
}

EncoderEvent Encoder::checkUpdate()
{
    if ( !event_flag ) 
        return EncoderEvent::None;

    noInterrupts();
    EncoderEvent e = pending_event;
    pending_event = EncoderEvent::None;
    event_flag = false;
    interrupts();

    return e;
}

void Encoder::isr()
{
    if ( !instance ) return;

    uint32_t now = micros();

    if ( now - instance->last_isr_us < 200 )
        return;
    instance->last_isr_us = now;

    if ( digitalRead( instance->sw ) == LOW )
    {
        if ( now - instance->last_btn_us < 15000 )
            return;
        instance->last_btn_us = now;
        instance->pending_event = EncoderEvent::BTN;
    }
    else
    {
        if ( digitalRead( instance->a ) == digitalRead( instance->b) )
            instance->pending_event = EncoderEvent::CCW;
        else
            instance->pending_event = EncoderEvent::CW;
    }

    instance->event_flag = true;
}