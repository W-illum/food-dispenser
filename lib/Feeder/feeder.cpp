#include "feeder.h"

Feeder::Feeder()
    : lcd(),
      enc( ENCODER_SW, ENCODER_A, ENCODER_B ),
      stepper( TMC_DIR, TMC_STEP, TMC_MS2, TMC_MS1, TMC_EN ),
      menu( lcd, schedule ) {}

void Feeder::begin()
{
    Wire.begin(SDA, SCL);
    lcd.begin();
    enc.begin();
    stepper.begin();

    // TODO: Load eeprom to fill schedule and food amount
    menu.begin( food_amount );
}

void Feeder::update()
{
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
                break;

            case EncoderEvent::CCW:
                menu.onRotate( -1 );
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
        }
    }
    menu.render();
}
