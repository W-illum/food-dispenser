/** Feeder.cpp
 *
 * Feeder class.
 *
 * @version 1.0.0
 * @author William Hafström <williamillum@gmail.com>
 */

#include "feeder.h"

namespace
{
constexpr uint32_t kSettingsMagic = 0x46445431UL; // "FDT1"
constexpr uint8_t kSettingsVersion = 1;
constexpr int kEepromSize = 256;

constexpr uint16_t kLidarNoDetectionPulseUs = 1850;
constexpr unsigned int kDegreesPerGram = 9;

volatile uint32_t g_lidar_rise_us = 0;
volatile uint16_t g_lidar_pulse_us = 0;
volatile bool g_lidar_new_sample = false;

void IRAM_ATTR lidarEdgeISR()
{
    const uint32_t now_us = micros();
    const int level = gpio_get_level( LIDAR_PWM );

    if ( level == HIGH )
    {
        g_lidar_rise_us = now_us;
    }
    else if ( g_lidar_rise_us != 0 )
    {
        g_lidar_pulse_us = static_cast<uint16_t>( now_us - g_lidar_rise_us );
        g_lidar_new_sample = true;
    }
}

int16_t lidarPulseToMm( uint16_t pulse_us )
{
    if ( pulse_us == 0 ) return -1; // Invalid/timeout
    if ( pulse_us > kLidarNoDetectionPulseUs ) return -2; // No target

    int16_t d = ( static_cast<int16_t>( pulse_us ) - 1000 ) * 3 / 4;
    if ( d < 0 ) d = 0;
    return d;
}
}

Feeder::Feeder()
    : lcd(),
      enc( ENCODER_SW, ENCODER_A, ENCODER_B ),
      stepper( TMC_DIR, TMC_STEP, TMC_MS2, TMC_MS1, TMC_EN ),
      menu( lcd, schedule ),
      rtc( DS_RST, DS_CLK, DS_DAT ) {}

void Feeder::begin()
{
    EEPROM.begin( kEepromSize );

    Wire.begin( LCD_SDA, LCD_SCL );
    lcd.begin();
    enc.begin();
    stepper.begin();
    pinMode( LIDAR_PWM, INPUT );
    attachInterrupt( digitalPinToInterrupt( LIDAR_PWM ), lidarEdgeISR, CHANGE );

    food_amount = 15;
    if ( !loadSettings() )
    {
        saveSettings();
    }

    rtc.init();
    if ( rtc.isHalted() )
    {
        Ds1302::DateTime init_time = {};
        init_time.year = 26;
        init_time.month = 1;
        init_time.day = 1;
        init_time.hour = 12;
        init_time.minute = 0;
        init_time.second = 0;
        init_time.dow = Ds1302::DOW_WED;
        rtc.setDateTime( &init_time );
    }

    rtc.getDateTime( &dt );
    system_clock.hour = dt.hour;
    system_clock.min = dt.minute;

    menu.begin( food_amount, system_clock );
    menu.setFoodAmount( food_amount );
    menu.setLidarMm( lidar_mm );
}

void Feeder::update()
{
    updateClock();
    processLidar();
    runFeedingSchedule();

    handleEncoder();
    handleMenuEvents();
    menu.render();
}

void Feeder::updateClock()
{
    rtc.getDateTime( &dt );
    system_clock.hour = dt.hour;
    system_clock.min = dt.minute;
}

void Feeder::processLidar()
{
    if ( !g_lidar_new_sample ) return;

    uint32_t now_us = micros();

    uint16_t pulse_us = 0;
    noInterrupts();
    pulse_us = g_lidar_pulse_us;
    g_lidar_new_sample = false;
    interrupts();

    int16_t mm = lidarPulseToMm( pulse_us );

    if ( mm >= 0 )
    {
        if ( now_us - lidar_last_sample_us >= 100000UL )
        {
            lidar_samples[lidar_sample_count] = mm;
            lidar_sample_count++;
            lidar_last_sample_us = now_us;

            if ( lidar_sample_count >= 5 )
            {
                int16_t sorted[5] = {lidar_samples[0], lidar_samples[1], lidar_samples[2], lidar_samples[3], lidar_samples[4]};

                for ( int i = 0; i < 4; i++ )
                {
                    for ( int j = i + 1; j < 5; j++ )
                    {
                        if ( sorted[j] < sorted[i] )
                        {
                            int16_t tmp = sorted[i];
                            sorted[i] = sorted[j];
                            sorted[j] = tmp;
                        }
                    }
                }

                lidar_mm = sorted[2];
                menu.setLidarMm( lidar_mm );
                lidar_sample_count = 0;
            }
        }
    }
    else
    {
        lidar_mm = mm;
        menu.setLidarMm( lidar_mm );
        lidar_sample_count = 0;
        lidar_last_sample_us = now_us;
    }
}

void Feeder::runFeedingSchedule()
{
    if ( !schedule.containsTime( {dt.hour, dt.minute} ) ) return;

    if ( last_dispense_hour == dt.hour && last_dispense_min == dt.minute )
    {
        return;
    }

    dispenseFood();
    last_dispense_hour = dt.hour;
    last_dispense_min = dt.minute;
}

void Feeder::dispenseFood()
{
    if ( food_amount == 0 ) return;

    const unsigned int deg = static_cast<unsigned int>( food_amount ) * kDegreesPerGram;
    stepper.rotate( deg, Dir::CW );
}

void Feeder::handleEncoder()
{
    EncoderEvent enc_e = enc.checkUpdate();
    if ( enc_e != EncoderEvent::NONE )
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

            default:
                break;
        }
    }
}

void Feeder::handleMenuEvents()
{
    MenuEvent menu_e = menu.pollEvent();
    if ( menu_e.type != MenuEvent::Type::NONE )
    {
        switch( menu_e.type )
        {
            case MenuEvent::Type::TIME_ADDED:
                if ( schedule.add( menu_e.time_added ) )
                {
                    saveSettings();
                }
                break;

            case MenuEvent::Type::TIME_REMOVED:
                schedule.remove( menu_e.index_removed );
                saveSettings();
                break;

            case MenuEvent::Type::FOOD_UPDATED:
                food_amount = menu_e.updated_grams;
                menu.setFoodAmount( food_amount );
                saveSettings();
                break;

            case MenuEvent::Type::CLOCK_UPDATED:
                system_clock = menu_e.updated_system_clock;
                dt.hour = system_clock.hour;
                dt.minute = system_clock.min;
                dt.second = 0;
                rtc.setDateTime( &dt );
                break;

            default:
                break;
        }
    }
}

bool Feeder::loadSettings()
{
    PersistedSettings settings = {};

    EEPROM.get( 0, settings );

    if ( settings.magic != kSettingsMagic ) return false;
    if ( settings.version != kSettingsVersion ) return false;
    if ( settings.schedule_count > MAX_FEEDS ) return false;

    const uint32_t expected = calcChecksum( settings );
    if ( expected != settings.checksum ) return false;

    food_amount = settings.food_amount;
    if ( food_amount > 50 ) food_amount = 50;

    for ( uint8_t i = 0; i < settings.schedule_count; i++ )
    {
        if ( settings.schedule_times[i].hour <= 23 && settings.schedule_times[i].min <= 59 )
        {
            schedule.add( settings.schedule_times[i] );
        }
    }
    return true;
}

bool Feeder::saveSettings()
{
    PersistedSettings settings = {};
    settings.magic = kSettingsMagic;
    settings.version = kSettingsVersion;
    settings.food_amount = food_amount;
    settings.schedule_count = schedule.size();

    const Time_t *times = schedule.data();
    for ( uint8_t i = 0; i < settings.schedule_count && i < MAX_FEEDS; i++ )
    {
        settings.schedule_times[i] = times[i];
    }

    settings.checksum = calcChecksum( settings );

    EEPROM.put( 0, settings );
    return EEPROM.commit();
}

uint32_t Feeder::calcChecksum( const PersistedSettings &data ) const
{
    const uint8_t *raw = reinterpret_cast<const uint8_t *>( &data );
    const size_t len = sizeof( PersistedSettings ) - sizeof( data.checksum );

    uint32_t sum = 0;
    for ( size_t i = 0; i < len; i++ )
    {
        sum = ( sum * 131U ) + raw[i];
    }
    return sum;
}
