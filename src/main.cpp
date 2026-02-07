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
    gpio_set_direction(LIDAR_PWM,  GPIO_MODE_INPUT);

    lcd.begin();
    enc.begin();
    stepper.begin();
    delay(100);
}

void loop()
{
    static const char* msg = "hello";
    lcd.rectangleFill(4, 4, lcd.getWidth() - 8, lcd.getHeight() - 8, COLOR_BLACK);
    EncoderEvent e = enc.checkUpdate();
    if ( e != EncoderEvent::NONE)
    {
        switch(e)
        {
            case EncoderEvent::BTN:
                msg = "BTN";
                break;

            case EncoderEvent::CW:
                stepper.rotate( 360, Dir::CW );
                msg = "CW";
                break;

            case EncoderEvent::CCW:
                stepper.rotate( 360, Dir::CCW );
                msg = "CCW";
                break;
        }
    }

    int x0 = (lcd.getWidth() - lcd.getStringWidth(msg)) / 2;
    int y0 = (lcd.getHeight() - lcd.getStringHeight(msg)) / 2;
    lcd.text(x0, y0, msg, COLOR_WHITE);
    lcd.display();

    /*
    Distance Sensor (needs 5V though...)
    int16_t t = pulseIn(LIDAR_PWM, HIGH);
    if ( t == 0 )
        Serial.println("timeout");
    else if ( t > 1850 )
        Serial.println("no detection");
    else
    {
        int16_t d = (t - 1000 ) * 3 / 4;
        if ( d < 0 ) d = 0;
        Serial.print(d);
        Serial.println(" mm");
    }
    delay(500);
    */
}