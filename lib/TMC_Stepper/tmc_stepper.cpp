#include "tmc_stepper.h"

TMCStepper *TMCStepper::instance = nullptr;
hw_timer_t *TMCStepper::timer = nullptr;

TMCStepper::TMCStepper( gpio_num_t DIR, gpio_num_t STEP, gpio_num_t MS2, gpio_num_t MS1, gpio_num_t EN )
    :   DIR( DIR ), STEP( STEP ), MS2( MS2 ), MS1( MS1), EN( EN ) {}

void TMCStepper::begin()
{
    instance = this;
    timer = timerBegin( 0, 80, true );
    timerAttachInterrupt( timer, &TMCStepper::timerISR, true );
    timerAlarmWrite( timer, 1000, true ); /* PSC:80 -> 1Mhz -> 1 step each 1ms */

    gpio_set_direction( DIR,  GPIO_MODE_OUTPUT );
    gpio_set_direction( STEP, GPIO_MODE_OUTPUT );
    gpio_set_direction( MS2,  GPIO_MODE_OUTPUT );
    gpio_set_direction( MS1,  GPIO_MODE_OUTPUT );
    gpio_set_direction( EN,   GPIO_MODE_OUTPUT );

    gpio_set_level( EN, LOW ); /* Enabled */
    setMicroStep( 8 ); /* Default */
}

bool TMCStepper::setMicroStep( int step )
{
    /* Stated in datasheet
    MS1     MS2     STEPS
    GND     GND     8
    VIO     GND     2
    GND     VIO     4
    VIO     VIO     16 
    */
    micro_step = step;

    switch( step )
    {
        case 8:
            gpio_set_level( MS1, LOW );
            gpio_set_level( MS2, LOW );
            return true;

        case 2:
            gpio_set_level( MS1, HIGH );
            gpio_set_level( MS2, LOW );
            return true;

        case 4:
            gpio_set_level( MS1, LOW );
            gpio_set_level( MS2, HIGH );
            return true;
        case 16:
            gpio_set_level( MS1, HIGH );
            gpio_set_level( MS2, HIGH );
            return true;

        default:
            return false; /* Silently fail basically */
    }
}

void TMCStepper::rotate( uint deg, Dir dir )
{
    if ( dir == Dir::CW ) 
        GPIO.out_w1ts = ( 1UL << DIR );
    else 
        GPIO.out_w1tc = ( 1UL << DIR );

    remaining_steps = (uint32_t)( deg * steps_per_rev * micro_step / 360.0 );
    timerAlarmEnable( timer );
}

void IRAM_ATTR TMCStepper::timerISR()
{
    if ( !instance ) return;

    if ( instance->remaining_steps > 0 )
    {
        GPIO.out_w1ts = ( 1UL << instance->STEP );
        ets_delay_us( 2 );
        GPIO.out_w1tc = ( 1UL << instance->STEP );

        instance->remaining_steps -= 1;
    }
    else timerAlarmDisable( timer );
}