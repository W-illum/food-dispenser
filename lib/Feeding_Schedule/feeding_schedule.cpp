#include "feeding_schedule.h"

uint8_t FeedingSchedule::size() const
{
    return count;
}

const Time_t *FeedingSchedule::data() const
{
    return times;
}

bool FeedingSchedule::add( Time_t time )
{
    if ( count >= MAX_FEEDS ) return false;
    if ( time_already_exists( time ) ) return false;

    int i = count - 1;
    while ( i >= 0 && ( times[i].hour > time.hour || ( times[i].hour == time.hour && times[i].min > time.min ) ) )
    {
        times[i + 1] = times[i];
        i--;
    }

    times[i + 1] = time;
    count++;
    return true;
}

bool FeedingSchedule::time_already_exists( Time_t time )
{
    for (const Time_t &t : times )
    {
        if ( t.hour == time.hour && t.min == time.min ) return true;
    }
    return false;
}

void FeedingSchedule::remove( uint8_t index )
{
    if ( index >= count ) return;

    for ( int i = index; i < count - 1; i++ )
    {
        times[i] = times[i + 1];
    }
    count--;
    times[count] = {99, 99}; /* Reset its value */
}
