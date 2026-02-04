#include "tmc_stepper.h"

TMCStepper::TMCStepper( int8_t pin_dir, int8_t pin_step, int8_t pin_ms2, int8_t pin_ms1, int8_t pin_en )
    :   pin_dir( pin_dir ), pin_step( pin_step ), pin_ms2( pin_ms2 ), pin_ms1( pin_ms1), pin_en( pin_en )
{
}

void TMCStepper::begin()
{
    pinMode( pin_dir,  OUTPUT );
    pinMode( pin_step, OUTPUT );
    pinMode( pin_ms2,  OUTPUT );
    pinMode( pin_ms1,  OUTPUT );
    pinMode( pin_en,   OUTPUT );

    digitalWrite( pin_en, LOW ); /* Enabled */
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
            digitalWrite( pin_ms1, LOW );
            digitalWrite( pin_ms2, LOW );
            return true;

        case 2:
            digitalWrite( pin_ms1, HIGH );
            digitalWrite( pin_ms2, LOW );
            return true;

        case 4:
            digitalWrite( pin_ms1, LOW );
            digitalWrite( pin_ms2, HIGH );
            return true;
        case 16:
            digitalWrite( pin_ms1, HIGH );
            digitalWrite( pin_ms2, HIGH );
            return true;

        default:
            return false; /* Silently fail basically */
    }
}

void TMCStepper::turnCW()
{
    digitalWrite(pin_dir, HIGH);
    
    for ( int i = 0; i < 200 * micro_step; i++ )
    {
        digitalWrite(pin_step, HIGH);
        delayMicroseconds(2);
        digitalWrite(pin_step, LOW);
        delayMicroseconds(800);
    }
}

void TMCStepper::turnCCW()
{
    digitalWrite(pin_dir, LOW);

    for ( int i = 0; i < 200 * micro_step; i++ )
    {
        digitalWrite(pin_step, HIGH);
        delayMicroseconds(2);
        digitalWrite(pin_step, LOW);
        delayMicroseconds(800);
    }
}