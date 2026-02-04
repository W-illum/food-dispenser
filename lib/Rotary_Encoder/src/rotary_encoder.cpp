#include <Arduino.h>
#include "rotary_encoder.h"

Encoder* Encoder::instance = nullptr;

Encoder::Encoder( int8_t pin_sw, int8_t pin_a, int8_t pin_b )
    :   pin_sw( pin_sw ), pin_a( pin_a ), pin_b( pin_b )
{
    instance = this;
}

void Encoder::begin()
{
    pinMode( pin_sw, INPUT_PULLUP );
    pinMode( pin_a,  INPUT_PULLUP );
    pinMode( pin_b,  INPUT_PULLUP );

    attachInterrupt( digitalPinToInterrupt( pin_sw ), Encoder::isr, FALLING );
    attachInterrupt( digitalPinToInterrupt( pin_a ), Encoder::isr, CHANGE );
    attachInterrupt( digitalPinToInterrupt( pin_b ), Encoder::isr, CHANGE );
}

EncoderEvent Encoder::checkUpdate()
{
    if ( !event_flag ) 
        return EncoderEvent::NONE;

    noInterrupts();
    EncoderEvent e = pending_event;
    pending_event = EncoderEvent::NONE;
    event_flag = false;
    interrupts();

    return e;
}

void Encoder::isr()
{
    if ( !instance ) return;

    uint32_t now = micros();

    int pin_sw = digitalRead( instance->pin_sw );
    int pin_a =  digitalRead( instance->pin_a );
    int pin_b =  digitalRead( instance->pin_b );

    /* Button */
    if ( pin_sw == LOW ) /* TODO if problem arise: Known issue here is having the button pressed 
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
            case RotationState::IDLE:
                if ( pin_a == LOW && pin_b == HIGH )
                    instance->current_state = RotationState::CW1;
                else if ( pin_a == HIGH && pin_b == LOW )
                    instance->current_state = RotationState::CCW1;
                break;

            /* CW */
            case RotationState::CW1:
                if ( pin_a == LOW && pin_b == LOW )
                    instance->current_state = RotationState::CW2;
                else if ( pin_a == HIGH && pin_b == HIGH )
                {
                    instance->current_state = RotationState::IDLE; /* Bounce or change of direction */
                }
                break;

            case RotationState::CW2:
                if ( pin_a == HIGH && pin_b == LOW )
                {
                    instance->current_state = RotationState::CW3;
                }
                else if ( pin_a == HIGH && pin_b == HIGH )
                {
                    instance->current_state = RotationState::IDLE; /* Bounce or change of direction */
                }
                break;

            case RotationState::CW3:
                if ( pin_a == HIGH && pin_b == HIGH )
                {
                    instance->pending_event = EncoderEvent::CW;
                    instance->current_state = RotationState::IDLE;
                    instance->event_flag = true;
                }
                break;

            /* CCW */
            case RotationState::CCW1:
                if ( pin_a == LOW && pin_b == LOW )
                    instance->current_state = RotationState::CCW2;
                else if ( pin_a == HIGH && pin_b == HIGH )
                {
                    instance->current_state = RotationState::IDLE; /* Bounce or change of direction */
                }
                break;

            case RotationState::CCW2:
                if ( pin_a == LOW && pin_b == HIGH )
                {
                    instance->current_state = RotationState::CCW3;
                }
                else if ( pin_a == HIGH && pin_b == HIGH )
                {
                    instance->current_state = RotationState::IDLE; /* Bounce or change of direction */
                }
                break;

            case RotationState::CCW3:
                if ( pin_a == HIGH && pin_b == HIGH )
                {
                    instance->pending_event = EncoderEvent::CCW;
                    instance->current_state = RotationState::IDLE;
                    instance->event_flag = true;
                }
                break;

            default:
                break;
        }
    }
}