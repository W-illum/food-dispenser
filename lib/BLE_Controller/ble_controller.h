/** BLEController.h
 *
 * BLEController class. Exposes the feeder state over BLE and emits
 * FeederEvents into a FreeRTOS queue for safe consumption on the main loop.
 *
 * @version 1.0.0
 * @author William Hafström <williamillum@gmail.com>
 */

#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "feeder_event.h"
#include "feeding_schedule.h"

// Forward declaration — keeps NimBLE headers out of user code
class NimBLECharacteristic;

class BLEController
{
public:
    /** @brief Initialize BLE, create service and characteristics, start advertising.
     *  @param schedule The feeding schedule (read for notifications and reads).
     *  @param food_amount Initial food amount for the characteristic value.
     *  @param clock Initial system clock for the characteristic value.
     */
    void begin( const FeedingSchedule& schedule, uint8_t food_amount, Time_t clock );

    /** @brief Push a status notification if 1 second has elapsed. Call from main loop.
     */
    void update();

    /** @brief Update cached food amount and the BLE characteristic value.
     */
    void setFoodAmount( uint8_t grams );

    /** @brief Update cached system clock and the BLE characteristic value.
     */
    void setSystemClock( Time_t clock );

    /** @brief Update cached LiDAR reading (included in next status notification).
     */
    void setLidarMm( int16_t mm );

    /** @brief Rebuild and notify the schedule characteristic. Call after any schedule change.
     */
    void notifyScheduleChanged();

    /** @brief Drain one event from the BLE event queue.
     *  @return The next event, or FeederEvent::Type::NONE if the queue is empty.
     */
    FeederEvent pollEvent();

private:
    void pushStatus();
    void pushSchedule();

    QueueHandle_t event_queue = nullptr;

    const FeedingSchedule* schedule = nullptr;
    uint8_t food_amount = 0;
    Time_t system_clock = { 0, 0 };
    int16_t lidar_mm = -1;
    uint32_t last_status_ms = 0;

    NimBLECharacteristic* status_char = nullptr;
    NimBLECharacteristic* schedule_char = nullptr;
    NimBLECharacteristic* food_amount_char = nullptr;
    NimBLECharacteristic* manual_feed_char = nullptr;
    NimBLECharacteristic* clock_char = nullptr;
};
