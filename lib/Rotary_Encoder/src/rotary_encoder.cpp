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
    attachInterrupt(digitalPinToInterrupt(a), Encoder::isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(b), Encoder::isr, CHANGE);
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

    int sw = digitalRead( instance->sw );
    int a =  digitalRead( instance->a );
    int b =  digitalRead( instance->b );

    /* Button */
    if ( sw == LOW ) /* TODO if problem arise: Known issue here is having the button pressed 
                        while turning will generate button presses, fine for now */
    {
        if ( now - instance->last_btn_us < BTN_DEBOUNCE_US )
            return;
        instance->last_btn_us = now;
        instance->pending_event = EncoderEvent::BTN;
        instance->event_flag = true;
    }

    /* Rotation */
    else
    {
        switch( instance->current_state )
        {
            case RotationState::Idle:
                if ( a == LOW && b == HIGH )
                    instance->current_state = RotationState::CW1;
                else if ( a == HIGH && b == LOW )
                    instance->current_state = RotationState::CCW1;
                break;

            /* CW */
            case RotationState::CW1:
                if ( a == LOW && b == LOW )
                    instance->current_state = RotationState::CW2;
                else if ( a == HIGH && b == HIGH )
                {
                    instance->current_state = RotationState::Idle; /* Bounce */
                }
                break;

            case RotationState::CW2:
                if ( a == HIGH && b == LOW )
                {
                    instance->current_state = RotationState::CW3;
                }
                break;

            case RotationState::CW3:
                if ( a == HIGH && b == HIGH )
                {
                    instance->pending_event = EncoderEvent::CW;
                    instance->current_state = RotationState::Idle;
                    instance->event_flag = true;
                }
                break;

            /* CCW */
            case RotationState::CCW1:
                if ( a == LOW && b == LOW )
                    instance->current_state = RotationState::CCW2;
                else if ( a == HIGH && b == HIGH )
                {
                    instance->current_state = RotationState::Idle; /* Bounce */
                }
                break;

            case RotationState::CCW2:
                if ( a == LOW && b == HIGH )
                {
                    instance->current_state = RotationState::CCW3;
                }
                break;

            case RotationState::CCW3:
                if ( a == HIGH && b == HIGH )
                {
                    instance->pending_event = EncoderEvent::CCW;
                    instance->current_state = RotationState::Idle;
                    instance->event_flag = true;
                }
                break;

            default:
                break;
        }
    }
}