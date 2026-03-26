/** Rotary_Encoder.h
 *
 * Rotary_Encoder class.
 *
 * @version 1.0.0
 * @author William Hafström <williamillum@gmail.com>
 */

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
    /** @brief Construct a new Encoder instance.
     *  @param SW The GPIO pin for the button.
     *  @param A The GPIO pin for the A channel.
     *  @param B The GPIO pin for the B channel.
     */
    Encoder( gpio_num_t SW, gpio_num_t A, gpio_num_t B );

    /** @brief Initialize the encoder.
     */
    void begin();

    /** @brief Check for updates to the encoder state.
     *  @return The encoder event, or EncoderEvent::NONE if no event is pending.
     */
    EncoderEvent checkUpdate();

private:
    static void isr();

private:
    static Encoder *instance;
    gpio_num_t SW, A, B;

    volatile bool event_flag = false;

    volatile EncoderEvent pending_event = EncoderEvent::NONE;
    volatile RotationState current_state = RotationState::IDLE;

    volatile uint32_t last_btn_us = 0;
};