/** Menu.h
 *
 * Menu class.
 *
 * @version 1.0.0
 * @author William Hafström <williamillum@gmail.com>
 */

#pragma once
#include <stdint.h>
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
    /** @brief Construct a new Menu instance.
     *  @param lcd The OLED display instance.
     *  @param schedule The feeding schedule instance.
     */
    Menu( Qwiic1in3OLED &lcd, const FeedingSchedule& schedule );

    /** @brief Initialize the menu.
     *  @param food_amount The initial food amount.
     *  @param clock The system clock.
     */
    void begin( uint8_t food_amount, Time_t& clock );

    /** @brief Poll for menu events.
     *  @return The menu event, or MenuEvent::Type::NONE if no event is pending.
     */
    MenuEvent pollEvent();

    /** @brief Handle rotation input.
     *  @param dir The rotation direction.
     */
    void onRotate( int dir );

    /** @brief Handle click input.
     */
    void onClick();

    /** @brief Render the menu.
     */
    void render();

    /** @brief Set the food amount.
     *  @param grams The food amount in grams.
     */
    void setFoodAmount( uint8_t grams );

    /** @brief Set the LiDAR distance.
     *  @param mm The distance in millimeters.
     */
    void setLidarMm( int16_t mm );

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
    bool getNextFeedTime( Time_t now, Time_t &next ) const;
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
    uint8_t home_food_amount = 0;
    int16_t home_lidar_mm = -1;
};