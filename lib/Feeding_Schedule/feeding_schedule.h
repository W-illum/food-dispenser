/** Feeding_Schedule.h
 *
 * FeedingSchedule class.
 *
 * @version 1.0.0
 * @author William Hafström <williamillum@gmail.com>
 */

#pragma once
#include <stdint.h>
#include "time.h"

#define MAX_FEEDS 4

class FeedingSchedule
{
public:
    /** @brief Get the number of feeding times in the schedule.
     *  @return The number of feeding times.
     */
    uint8_t size() const;

    /** @brief Get a pointer to the array of feeding times.
     *  @return A pointer to the array of feeding times.
     */
    const Time_t* data() const;

    /** @brief Add a new feeding time to the schedule.
     *  @param time The feeding time to add.
     *  @return True if the time was added, false otherwise.
     */
    bool add( Time_t time );

    /** @brief Remove a feeding time from the schedule.
     *  @param index The index of the feeding time to remove.
     */
    void remove( uint8_t index );

    /** @brief Check if the schedule contains a specific feeding time.
     *  @param time The feeding time to check.
     *  @return True if the time is in the schedule, false otherwise.
     */
    bool containsTime( Time_t time ) const;

private:
    Time_t times[MAX_FEEDS] = { {99, 99}, {99, 99}, {99, 99}, {99, 99} }; /* Default values (out of range 24H clock) */
    uint8_t count = 0;
};