#include <Arduino.h>
#include "SparkFun_Qwiic_OLED.h"
#include "rotary_encoder.h"
#include "tmc_stepper.h"
/*  Function Declarations   */

/* Pins */
#define LIDAR_PWM   GPIO_NUM_6  // D3
#define TMC_DIR     GPIO_NUM_7  // D4
#define TMC_STEP    GPIO_NUM_8  // D5
#define TMC_MS2     GPIO_NUM_9  // D6
#define TMC_MS1     GPIO_NUM_10 // D7
#define TMC_EN      GPIO_NUM_17 // D8
#define ENCODER_SW  GPIO_NUM_18 // D9
#define ENCODER_A   GPIO_NUM_21 // D10
#define ENCODER_B   GPIO_NUM_38 // D11
#define SDA         GPIO_NUM_11 // A4
#define SCL         GPIO_NUM_12 // A5

/* Globals */
Qwiic1in3OLED lcd;
Encoder enc( ENCODER_SW, ENCODER_A, ENCODER_B );
TMCStepper stepper( TMC_DIR, TMC_STEP, TMC_MS2, TMC_MS1, TMC_EN );

void setup()
{
    Serial.begin(115200);
    delay(500);
    Wire.begin(SDA, SCL);
    //pinMode(LIDAR_PWM,  INPUT);
    //pinMode(LED_BUILTIN, OUTPUT);

    lcd.begin();
    enc.begin();
    stepper.begin();
    delay(100);
}

void loop()
{
    EncoderEvent e = enc.checkUpdate();
    if ( e != EncoderEvent::NONE)
    {
        switch(e)
        {
            case EncoderEvent::BTN:
                Serial.print("BTN");
                break;

            case EncoderEvent::CW:
                Serial.print("CW");
                stepper.rotate( 360, Dir::CW );
                break;

            case EncoderEvent::CCW:
                Serial.print("CCW");
                stepper.rotate( 360, Dir::CCW );
                break;
        }
        Serial.print("||");
    }
}