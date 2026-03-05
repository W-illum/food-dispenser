#pragma once
#include <stdint.h>
#include "time.h"

#define MAX_FEEDS 4

class FeedingSchedule
{
public:
    uint8_t size() const;
    const Time_t* data() const;
    bool add( Time_t time );
    void remove( uint8_t index );

private:
    bool time_already_exists( Time_t time );

private:
    Time_t times[MAX_FEEDS] = { {99, 99}, {99, 99}, {99, 99}, {99, 99} }; /* Default values (out of range 24H clock) */
    uint8_t count = 0;
};