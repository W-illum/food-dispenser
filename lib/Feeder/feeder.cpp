#include "feeder.h"

Feeder::Feeder()
    : lcd(),
      enc( ENCODER_SW, ENCODER_A, ENCODER_B ),
      stepper( TMC_DIR, TMC_STEP, TMC_MS2, TMC_MS1, TMC_EN ),
      menu( lcd, schedule ),
      rtc( DS_RST, DS_CLK, DS_DAT ) {}

void Feeder::begin()
{
    Wire.begin( LCD_SDA, LCD_SCL );
    lcd.begin();
    enc.begin();
    stepper.begin();

    // TODO: Load eeprom to fill schedule and food amount
    rtc.init();
    menu.begin( food_amount, system_clock );
}

void Feeder::update()
{
    rtc.getDateTime(&dt);
    system_clock.hour = dt.hour;
    system_clock.min = dt.minute;

    static uint8_t last_min = 0;
    if( dt.minute != last_min )
    {
        last_min = dt.minute;
        Serial.printf("%02d:%02d", dt.hour, dt.minute);
    }

    /*
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
    delay(500);*/

    EncoderEvent enc_e = enc.checkUpdate();
    if ( enc_e != EncoderEvent::NONE)
    {
        switch( enc_e )
        {
            case EncoderEvent::BTN:
                menu.onClick();
                break;

            case EncoderEvent::CW:
                menu.onRotate( 1 );
                stepper.rotate(45, Dir::CW);
                break;

            case EncoderEvent::CCW:
                menu.onRotate( -1 );
                stepper.rotate(45, Dir::CCW);
                break;
        }
    }

    MenuEvent menu_e = menu.pollEvent();
    if ( menu_e.type != MenuEvent::Type::NONE )
    {
        switch( menu_e.type )
        {
            case MenuEvent::Type::TIME_ADDED:
                schedule.add( menu_e.time_added );
                // TODO: Save to eeprom
                break;

            case MenuEvent::Type::TIME_REMOVED:
                schedule.remove( menu_e.index_removed );
                // TODO: Save to eeprom
                break;

            case MenuEvent::Type::FOOD_UPDATED:
                food_amount = menu_e.updated_grams;
                // TODO: Save to eeprom
                break;

            case MenuEvent::Type::CLOCK_UPDATED:
                system_clock = menu_e.updated_system_clock;
                dt.hour = system_clock.hour;
                dt.minute = system_clock.min;
                dt.second = 0;
                rtc.setDateTime(&dt);
                break;
        }
    }
    menu.render();
}
