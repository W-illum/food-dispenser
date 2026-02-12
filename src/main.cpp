#include <Arduino.h>
#include "feeder.h"

Feeder feeder;

void setup()
{
    Serial.begin(115200);
    delay(500);
    feeder.begin();
    delay(100);
}

void loop()
{
    feeder.update();
}