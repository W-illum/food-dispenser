/** Menu.cpp
 *
 * Menu class.
 *
 * @version 1.0.0
 * @author William Hafström <williamillum@gmail.com>
 */

#include "menu.h"

Menu::Menu(Qwiic1in3OLED &lcd, const FeedingSchedule& schedule )
    : lcd(lcd), schedule(schedule) {}

void Menu::begin( uint8_t food_amount, Time_t& clock )
{
    lcd.setFont( QW_FONT_8X16 );
    current_screen = Screen::HOME;
    mode = InteractionMode::NAVIGATE;
    pending_event = {};
    system_clock = &clock;
    edit_add_time = {};
    selected_index = 1;
    edit_food_amount = food_amount;
    edit_system_clock = {};
    home_food_amount = food_amount;
    home_lidar_mm = -1;
}

MenuEvent Menu::pollEvent()
{
    MenuEvent e = pending_event;
    pending_event = {};
    return e;
}

void Menu::onRotate(int dir)
{
    if ( mode == InteractionMode::EDIT )
    {
        switch ( current_screen )
        {
            case Screen::ADD:
                if ( selected_index == 1 ) // Hour
                {
                    int h = edit_add_time.hour;
                    h += dir;

                    if ( h < 0 )  h = 23;
                    if ( h > 23 ) h = 0;
                    edit_add_time.hour = static_cast<uint8_t>(h);
                }
                if ( selected_index == 2 ) // Minute
                {
                    int m = edit_add_time.min;
                    m += dir * 5;

                    if ( m < 0 )  m = 55;
                    if ( m > 55 ) m = 0;
                    edit_add_time.min = static_cast<uint8_t>(m);
                }
                break;

            case Screen::FOOD_AMOUNT:
                edit_food_amount += dir * 5;
                if ( edit_food_amount < 0 ) edit_food_amount = 0;
                if ( edit_food_amount > 50 ) edit_food_amount = 50; /* You have a fat cat if u need more */
                break;

            case Screen::CLOCK:
                if ( selected_index == 1 ) // Hour
                {
                    int h = edit_system_clock.hour;
                    h += dir;
                    if ( h < 0 )  h = 23;
                    if ( h > 23 ) h = 0;
                    edit_system_clock.hour = static_cast<uint8_t>(h);
                }
                if ( selected_index == 2 ) // Minute
                {
                    int m = edit_system_clock.min;
                    m += dir;
                    if ( m < 0 )  m = 59;
                    if ( m > 59 ) m = 0;
                    edit_system_clock.min = static_cast<uint8_t>(m);
                }
                break;
        }
    }
    else // InteractionMode::NAVIGATE
    {
        selected_index += dir;
        int max_index = getOptionsPerScreen( current_screen );

        if ( current_screen == Screen::SCHEDULES && ( schedule.size() == MAX_FEEDS ) && ( selected_index == 2 ) )
            selected_index += dir; /* Skip over the Add button if we already are full */

        if ( selected_index > max_index )
            selected_index = 1;
        if ( selected_index < 1 )
            selected_index = max_index;
    }
}

void Menu::onClick()
{
    switch ( current_screen )
    {
        case Screen::HOME:
            current_screen = Screen::SETTINGS;
            selected_index = 1;
            break;

        case Screen::SETTINGS:
            switch ( selected_index )
            {
                case 1: current_screen = Screen::HOME; break;           // Back
                case 2: current_screen = Screen::SCHEDULES; break;      // Schedules
                case 3: current_screen = Screen::FOOD_AMOUNT; break;    // Food Amount
                case 4:
                    current_screen = Screen::CLOCK;
                    edit_system_clock = *system_clock;
            }
            selected_index = 1;
            break;

        case Screen::SCHEDULES:
            switch ( selected_index )
            {
                case 1: current_screen = Screen::SETTINGS; break;   // Back
                case 2: current_screen = Screen::ADD; break;        // Add
                case 3: current_screen = Screen::REMOVE; break;     // Remove
            }
            selected_index = 1;
            break;

        case Screen::FOOD_AMOUNT:
            switch ( selected_index )
            {
                case 1: // Change food amount
                    if ( mode == InteractionMode::NAVIGATE )
                        mode = InteractionMode::EDIT;
                    else
                        mode = InteractionMode::NAVIGATE;
                    break;

                case 2: // Confirm
                    pending_event.type = MenuEvent::Type::FOOD_UPDATED;
                    pending_event.updated_grams = edit_food_amount;
                    current_screen = Screen::SETTINGS;
                    selected_index = 1;
                    break;
            }
            break;

        case Screen::ADD:
            switch ( selected_index )
            {
                case 1: // Hour
                case 2: // Minute
                    if ( mode == InteractionMode:: NAVIGATE )
                        mode = InteractionMode::EDIT;
                    else
                        mode = InteractionMode::NAVIGATE;
                    break;

                case 3: // Confirm
                    pending_event.type = MenuEvent::Type::TIME_ADDED;
                    pending_event.time_added = edit_add_time;
                    edit_add_time = { 0, 0 };
                    current_screen = Screen::SCHEDULES;
                    selected_index = 1;
                    break;

                case 4: // Back
                    edit_add_time = { 0, 0 };
                    current_screen = Screen::SCHEDULES;
                    selected_index = 1;
                    break;

            }
            break;

        case Screen::REMOVE:
            switch ( selected_index )
            {
                case 1: // Back
                    current_screen = Screen::SCHEDULES;
                    break;
                case 2: // schedule.data[ix = 0]
                case 3: // schedule.data[ix = 1]
                case 4: // schedule.data[ix = 2]
                case 5: // schedule.data[ix = 3]
                    pending_event.type = MenuEvent::Type::TIME_REMOVED;
                    pending_event.index_removed = selected_index - 2;
                    /* Keep same screen if user want to remove multiple times */
                    selected_index = 1;
                    break;
            }
            break;

        case Screen::CLOCK:
            switch( selected_index )
            {
                case 1: // Hour
                case 2: // Minute
                    if ( mode == InteractionMode:: NAVIGATE )
                        mode = InteractionMode::EDIT;
                    else
                        mode = InteractionMode::NAVIGATE;
                    break;

                case 3: // Confirm
                    pending_event.type = MenuEvent::Type::CLOCK_UPDATED;
                    pending_event.updated_system_clock = edit_system_clock;
                    current_screen = Screen::SETTINGS;
                    selected_index = 1;
                    break;

                case 4: // Back
                    current_screen = Screen::SETTINGS;
                    selected_index = 1;
                    break;
            }
            break;
    }
}

void Menu::render()
{
    lcd.erase();
    switch (current_screen)
    {
        case Screen::HOME: renderHome(); break;

        case Screen::SETTINGS: renderSettings(); break;

        case Screen::SCHEDULES: renderSchedules(); break;

        case Screen::ADD: renderAdd(); break;

        case Screen::FOOD_AMOUNT: renderFoodAmount(); break;

        case Screen::REMOVE: renderRemove(); break;

        case Screen::CLOCK: renderClock(); break;
    }
    lcd.display();
}

void Menu::setFoodAmount( uint8_t grams )
{
    home_food_amount = grams;
}

void Menu::setLidarMm( int16_t mm )
{
    home_lidar_mm = mm;
}

uint8_t Menu::getOptionsPerScreen( Screen screen ) const
{
    switch( screen )
    {
        case Screen::HOME: return 1;

        case Screen::SETTINGS: return 4;

        case Screen::SCHEDULES: return 3;

        case Screen::ADD: return 4;

        case Screen::FOOD_AMOUNT: return 2;

        case Screen::REMOVE: return 1 + schedule.size();

        case Screen::CLOCK: return 4;

        default: return 0;
    }
}

void Menu::renderHome()
{
    char buff[24] = {0};

    const Time_t now = ( system_clock != nullptr ) ? *system_clock : Time_t{0, 0};
    snprintf( buff, sizeof( buff ), "Time %02d:%02d", now.hour, now.min );
    lcd.text( 0, 0, buff, COLOR_WHITE );

    Time_t next = {0, 0};
    if ( getNextFeedTime( now, next ) )
    {
        snprintf( buff, sizeof( buff ), "Next %02d:%02d %ug", next.hour, next.min, home_food_amount );
    }
    else
    {
        snprintf( buff, sizeof( buff ), "Next --:-- %ug", home_food_amount );
    }
    lcd.text( 0, 16, buff, COLOR_WHITE );

    if ( home_lidar_mm >= 0 )
    {
        const int max_mm = 400;
        const int clamped = ( home_lidar_mm > max_mm ) ? max_mm : home_lidar_mm;
        const int pct = 100 - ( clamped * 100 / max_mm );
        snprintf( buff, sizeof( buff ), "Lvl %3dmm %3d%%", home_lidar_mm, pct );
    }
    else if ( home_lidar_mm == -2 )
    {
        snprintf( buff, sizeof( buff ), "Lvl no target" );
    }
    else
    {
        snprintf( buff, sizeof( buff ), "Lvl reading..." );
    }
    lcd.text( 0, 32, buff, COLOR_WHITE );

    if ( selected_index == 1 )
    {
        lcd.text( 0, 48, ">Settings", COLOR_WHITE );
    }
}

bool Menu::getNextFeedTime( Time_t now, Time_t &next ) const
{
    const uint8_t count = schedule.size();
    const Time_t *times = schedule.data();

    if ( count == 0 || times == nullptr )
    {
        return false;
    }

    for ( uint8_t i = 0; i < count; ++i )
    {
        if ( times[i].hour > now.hour || ( times[i].hour == now.hour && times[i].min >= now.min ) )
        {
            next = times[i];
            return true;
        }
    }

    // Wrapped to next day.
    next = times[0];
    return true;
}

void Menu::renderSettings()
{
    static const char* items[] = {
        "Back",
        "Schedules",
        "Food Amount",
        "System Clock"
    };

    for ( int i = 0; i < 4; i++ )
    {
        lcd.setCursor( 0, i * 15 );
        if ( (i + 1) == selected_index ) lcd.print( ">" );
        lcd.print( items[i] );
    }
}

void Menu::renderSchedules()
{
    static const char* items[] = {
        "Back",
        "Add",
        "Remove"
    };

    for ( int i = 0; i < 3; i++ )
    {
        lcd.setCursor( 0, i * 15 );
        if ( (i + 1) == selected_index ) lcd.print( ">" );
        lcd.print( items[i] );
    }
    lcd.setCursor( 0, 45 );
    const uint8_t count = schedule.size();
    lcd.print( count );
    lcd.print( "/" );
    lcd.print( MAX_FEEDS );

    const Time_t *times = schedule.data();
    char buff[6];
    for ( int i = 0; i < count; i++ )
    {
        lcd.setCursor( 75, i * 15 );
        snprintf( buff, sizeof( buff ), "%02d:%02d", times[i].hour, times[i].min );
        lcd.print( buff );
    }
}

void Menu::renderFoodAmount()
{
    lcd.setCursor( 0, 0 );
    if ( selected_index == 1 )
        lcd.print( ">" );
    if ( mode == InteractionMode::EDIT )
        lcd.print( "*" );
    else
        lcd.print( " " );
    lcd.print( edit_food_amount );
    lcd.print( " g" );

    lcd.setCursor( 0, 24 );
    if ( selected_index == 2 ) lcd.print( ">" );
    lcd.print( "Confirm" );
}

void Menu::renderAdd()
{
    char time_str[6];
    snprintf( time_str, sizeof( time_str ) , "%02d:%02d", edit_add_time.hour, edit_add_time.min );

    lcd.setCursor( 0, 0 );
    if ( selected_index == 1 )
    {
        if ( mode == InteractionMode::NAVIGATE ) lcd.print( ">" );
        if ( mode == InteractionMode::EDIT ) lcd.print( ">*" );
    }

    lcd.print( time_str );

    if ( selected_index == 2 )
    {
        if ( mode == InteractionMode::NAVIGATE ) lcd.print( "<" );
        if ( mode == InteractionMode::EDIT ) lcd.print( "*<" );
    }

    lcd.setCursor( 0, 24 );
    if ( selected_index == 3 ) lcd.print( ">" );
    lcd.print( "Confirm" );

    lcd.setCursor( 0, 48 );
    if ( selected_index == 4 ) lcd.print( ">" );
    lcd.print( "Back" );
}

void Menu::renderRemove()
{
    lcd.setCursor( 0, 0 );
    if ( selected_index == 1 ) lcd.print( ">" );
    lcd.print( "Back" );

    const uint8_t count = schedule.size();
    const Time_t *times = schedule.data();
    char buff[6];
    for ( int i = 0; i < count; i++ )
    {
        if ( (i + 2) == selected_index )
        {
            lcd.setCursor( 65, i * 15 );
            lcd.print( ">" );
        }
        lcd.setCursor( 75, i * 15 );
        snprintf( buff, sizeof( buff ), "%02d:%02d", times[i].hour, times[i].min );
        lcd.print( buff );
    }
}

void Menu::renderClock()
{
    char clock_str[6];
    snprintf( clock_str, sizeof( clock_str ) , "%02d:%02d", edit_system_clock.hour, edit_system_clock.min );

    lcd.setCursor( 0, 0 );
    if ( selected_index == 1 )
    {
        if ( mode == InteractionMode::NAVIGATE ) lcd.print( ">" );
        if ( mode == InteractionMode::EDIT ) lcd.print( ">*" );
    }

    lcd.print( clock_str );

    if ( selected_index == 2 )
    {
        if ( mode == InteractionMode::NAVIGATE ) lcd.print( "<" );
        if ( mode == InteractionMode::EDIT ) lcd.print( "*<" );
    }

    lcd.setCursor( 0, 24 );
    if ( selected_index == 3 ) lcd.print( ">" );
    lcd.print( "Confirm" );

    lcd.setCursor( 0, 48 );
    if ( selected_index == 4 ) lcd.print( ">" );
    lcd.print( "Back" );
}
