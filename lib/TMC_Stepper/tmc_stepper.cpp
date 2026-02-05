#include "tmc_stepper.h"

TMCStepper::TMCStepper( gpio_num_t DIR, gpio_num_t STEP, gpio_num_t MS2, gpio_num_t MS1, gpio_num_t EN )
    :   DIR( DIR ), STEP( STEP ), MS2( MS2 ), MS1( MS1), EN( EN )
{
}

void TMCStepper::begin()
{
    gpio_set_direction( DIR,  GPIO_MODE_OUTPUT );
    gpio_set_direction( STEP, GPIO_MODE_OUTPUT );
    gpio_set_direction( MS2,  GPIO_MODE_OUTPUT );
    gpio_set_direction( MS1,  GPIO_MODE_OUTPUT );
    gpio_set_direction( EN,   GPIO_MODE_OUTPUT );

    gpio_set_level( EN, LOW ); /* Enabled */
    setMicroStep( 8 ); /* Default */
}

bool TMCStepper::setMicroStep(int step)
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

void TMCStepper::turnCW()
{
    gpio_set_level(DIR, HIGH);

    for ( int i = 0; i < 200 * micro_step; i++ )
    {
        gpio_set_level(STEP, HIGH);
        delayMicroseconds(2);
        gpio_set_level(STEP, LOW);
        delayMicroseconds(800);
    }
}

void TMCStepper::turnCCW()
{
    gpio_set_level(DIR, LOW);

    for ( int i = 0; i < 200 * micro_step; i++ )
    {
        gpio_set_level(STEP, HIGH);
        delayMicroseconds(2);
        gpio_set_level(STEP, LOW);
        delayMicroseconds(800);
    }
}