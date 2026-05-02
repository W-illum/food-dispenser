/** FeederEvent.h
 *
 * Shared event type consumed by Feeder and emitted by any input source
 * (Menu, BLE, etc.).
 *
 * @version 1.0.0
 * @author William Hafström <williamillum@gmail.com>
 */

#pragma once
#include <stdint.h>
#include "feeding_schedule.h"

struct FeederEvent
{
    enum class Type {
        NONE,
        TIME_ADDED,
        TIME_REMOVED,
        FOOD_UPDATED,
        CLOCK_UPDATED,
        MANUAL_FEED
    };

    Type type = Type::NONE;

    Time_t time_added = { 0, 0 };
    uint8_t index_removed = 0;
    uint8_t updated_grams = 0;
    Time_t updated_system_clock = { 0, 0 };
    uint8_t manual_feed_grams = 0;
};