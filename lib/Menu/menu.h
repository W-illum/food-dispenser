#pragma once
#include "SparkFun_Qwiic_OLED.h"
#include "res/qw_fnt_8x16.h"
#include "res/qw_fnt_5x7.h"
#include "feeding_schedule.h"

struct MenuEvent
{
    enum class Type{
        NONE,
        TIME_ADDED,
        TIME_REMOVED,
        FOOD_UPDATED,
        CLOCK_UPDATED
    };

    Type type = Type::NONE;

    Time_t time_added = { 0, 0 };
    uint8_t index_removed = 0;
    uint8_t updated_grams = 0;
    Time_t updated_system_clock = { 0, 0 };
};

class Menu
{
public:
    Menu( Qwiic1in3OLED &lcd, const FeedingSchedule& schedule );
    void begin( uint8_t food_amount, Time_t& clock );
    MenuEvent pollEvent();
    void onRotate( int dir );
    void onClick();
    void render();

public:
    enum class Screen{
        HOME,
        SETTINGS,
        SCHEDULES,
        ADD,
        REMOVE,
        FOOD_AMOUNT,
        CLOCK
    };

    enum class InteractionMode{
        NAVIGATE,
        EDIT
    };

private:
    uint8_t getOptionsPerScreen( Screen screen ) const;
    void renderHome();
    void renderSettings();
    void renderSchedules();
    void renderFoodAmount();
    void renderAdd();
    void renderRemove();
    void renderClock();

private:
    Qwiic1in3OLED &lcd;
    const FeedingSchedule& schedule;
    Screen current_screen;
    InteractionMode mode;
    MenuEvent pending_event;
    const Time_t* system_clock = nullptr;

    Time_t edit_add_time;
    int selected_index;
    int edit_food_amount;
    Time_t edit_system_clock;
};