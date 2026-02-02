#include <Arduino.h>
#include "SparkFun_Qwiic_OLED.h"
#include "rotary_encoder.h"
/*  Function Declarations   */

/* Pins */
#define LIDAR_PWM   D3
#define TMC_DIR     D4
#define TMC_STEP    D5
#define TMC_MS2     D6
#define TMC_MS1     D7
#define TMC_EN      D8
#define ENCODER_SW  D9
#define ENCODER_A   D10
#define ENCODER_B   D11
#define SDA         A4
#define SCL         A5

/* Globals */
//Qwiic1in3OLED myOLED;
Encoder myEncoder(D9, D10, D11);

void setup()
{
    Serial.begin(115200);
    delay(500);

    pinMode(LIDAR_PWM,  INPUT);
    pinMode(TMC_DIR,    OUTPUT);
    pinMode(TMC_STEP,   OUTPUT);
    pinMode(TMC_MS2,    OUTPUT);
    pinMode(TMC_MS1,    OUTPUT);
    pinMode(TMC_EN,     OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    myEncoder.begin();
    Wire.begin(SDA, SCL);
    delay(100);
}

void loop()
{
    EncoderEvent e = myEncoder.checkUpdate();
    if ( e != EncoderEvent::None)
        Serial.print(int(e));
}