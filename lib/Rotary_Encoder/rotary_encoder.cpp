/** Rotary_Encoder.cpp
 *
 * Rotary_Encoder class.
 *
 * @version 1.0.0
 * @author William Hafström <williamillum@gmail.com>
 */

#include "rotary_encoder.h"

Encoder *Encoder::instance = nullptr;

Encoder::Encoder( gpio_num_t SW, gpio_num_t A, gpio_num_t B )
    :   SW( SW ), A( A ), B( B ) {}

void Encoder::begin()
{
    instance = this;
    gpio_set_direction( SW, GPIO_MODE_INPUT ); // gpio_set_pull_mode(SW, GPIO_PULLUP_ONLY);
    gpio_set_direction( A,  GPIO_MODE_INPUT ); // gpio_set_pull_mode(A, GPIO_PULLUP_ONLY);
    gpio_set_direction( B,  GPIO_MODE_INPUT ); // gpio_set_pull_mode(B, GPIO_PULLUP_ONLY);

    attachInterrupt( SW, Encoder::isr, FALLING );
    attachInterrupt( A, Encoder::isr, CHANGE );
    attachInterrupt( B, Encoder::isr, CHANGE );
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

    int SW = gpio_get_level( instance->SW );
    int A =  gpio_get_level( instance->A );
    int B =  gpio_get_level( instance->B );

    /* Button */
    if ( SW == LOW ) /* TODO if problem arise: Known issue here is having the button pressed
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
                if ( A == LOW && B == HIGH )
                    instance->current_state = RotationState::CW1;
                else if ( A == HIGH && B == LOW )
                    instance->current_state = RotationState::CCW1;
                break;

            /* CW */
            case RotationState::CW1:
                if ( A == LOW && B == LOW )
                {
                    instance->current_state = RotationState::CW2;
                }
                else if ( A == HIGH && B == HIGH )
                {
                    instance->current_state = RotationState::IDLE; /* Bounce or change of direction */
                }
                break;

            case RotationState::CW2:
                if ( A == HIGH && B == LOW )
                {
                    instance->current_state = RotationState::CW3;
                }
                else if ( A == HIGH && B == HIGH )
                {
                    instance->current_state = RotationState::IDLE; /* Bounce or change of direction */
                }
                break;

            case RotationState::CW3:
                if ( A == HIGH && B == HIGH )
                {
                    instance->pending_event = EncoderEvent::CW;
                    instance->current_state = RotationState::IDLE;
                    instance->event_flag = true;
                }
                break;

            /* CCW */
            case RotationState::CCW1:
                if ( A == LOW && B == LOW )
                {
                    instance->current_state = RotationState::CCW2;
                }
                else if ( A == HIGH && B == HIGH )
                {
                    instance->current_state = RotationState::IDLE; /* Bounce or change of direction */
                }
                break;

            case RotationState::CCW2:
                if ( A == LOW && B == HIGH )
                {
                    instance->current_state = RotationState::CCW3;
                }
                else if ( A == HIGH && B == HIGH )
                {
                    instance->current_state = RotationState::IDLE; /* Bounce or change of direction */
                }
                break;

            case RotationState::CCW3:
                if ( A == HIGH && B == HIGH )
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