/** TMCStepper.h
 *
 * TMCStepper class.
 *
 * @version 1.0.0
 * @author William Hafström <williamillum@gmail.com>
 */

#pragma once
#include <Arduino.h>

enum class Dir : uint8_t {
    CCW,
    CW
};

class TMCStepper
{
public:
    /** @brief Construct a new TMCStepper instance.
     *  @param DIR The GPIO pin for the direction signal.
     *  @param STEP The GPIO pin for the step signal.
     *  @param MS2 The GPIO pin for the microstep 2 signal.
     *  @param MS1 The GPIO pin for the microstep 1 signal.
     *  @param EN The GPIO pin for the enable signal.
     */
    TMCStepper( gpio_num_t DIR, gpio_num_t STEP, gpio_num_t MS2, gpio_num_t MS1, gpio_num_t EN );

    /** @brief Initialize the TMCStepper.
     */
    void begin();

    /** @brief Set the microstepping configuration.
     *  @param step The microstep value.
     */
    void setMicroStep( int step );

    /** @brief Rotate the stepper motor.
     *  @param deg The angle to rotate.
     *  @param dir The direction of rotation.
     */
    void rotate( unsigned int deg, Dir dir );

    /** @brief Timer interrupt service routine.
     */
    static void IRAM_ATTR timerISR();

private:
    volatile uint32_t remaining_steps = 0;
    gpio_num_t DIR, STEP, MS2, MS1, EN;
    int micro_step;
    int steps_per_rev = 200;

    static TMCStepper *instance;
    static hw_timer_t *timer;
};