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
#include "feeder_event.h"

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
    void begin( uint8_t food_amount, const Time_t& clock );

    /** @brief Poll for pending events.
     *  @return The pending event, or FeederEvent::Type::NONE if no event is pending.
     */
    FeederEvent pollEvent();

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
    void renderHome() const;
    void renderSettings() const;
    void renderSchedules() const;
    void renderFoodAmount() const;
    void renderAdd() const;
    void renderRemove() const;
    void renderClock() const;

private:
    Qwiic1in3OLED &lcd;
    const FeedingSchedule& schedule;
    Screen current_screen;
    InteractionMode mode;
    FeederEvent pending_event;
    const Time_t* system_clock = nullptr;

    Time_t edit_add_time;
    int selected_index;
    int edit_food_amount;
    Time_t edit_system_clock;
    uint8_t home_food_amount = 0;
    int16_t home_lidar_mm = -1;
};