#pragma once
#include <stdint.h>

#define MAX_FEEDS 4

struct FeedingTime
{
    uint8_t hour;
    uint8_t min;
};

class FeedingSchedule
{
public:
    uint8_t size() const;
    const FeedingTime* data() const;
    bool add( FeedingTime time );
    void remove( uint8_t index );

private:
    bool time_already_exists( FeedingTime time );

private:
    FeedingTime times[MAX_FEEDS] = { {99, 99}, {99, 99}, {99, 99}, {99, 99} };
    uint8_t count = 0;
};