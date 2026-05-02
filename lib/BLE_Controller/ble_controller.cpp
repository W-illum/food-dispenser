/** BLEController.cpp
 *
 * BLEController class.
 *
 * @version 1.0.0
 * @author William Hafström <williamillum@gmail.com>
 */

#include <time.h>
#include <Arduino.h>
#include <NimBLEDevice.h>
#include "ble_controller.h"

// UUIDs — all derived from the same base, last byte identifies the characteristic
static const char* kServiceUUID     = "4fafc201-1fb5-459e-8fcc-c5c9c3319100";
static const char* kStatusUUID      = "4fafc201-1fb5-459e-8fcc-c5c9c3319101";
static const char* kScheduleUUID    = "4fafc201-1fb5-459e-8fcc-c5c9c3319102";
static const char* kFoodAmountUUID  = "4fafc201-1fb5-459e-8fcc-c5c9c3319103";
static const char* kManualFeedUUID  = "4fafc201-1fb5-459e-8fcc-c5c9c3319104";
static const char* kClockUUID       = "4fafc201-1fb5-459e-8fcc-c5c9c3319105";

static constexpr uint32_t kStatusIntervalMs = 1000;
static constexpr uint8_t kEventQueueDepth = 8;

// Binary status payload pushed to the phone every second
#pragma pack(push, 1)
struct BLEStatus {
    uint8_t  hour;
    uint8_t  min;
    int16_t  lidar_mm;
    uint8_t  food_amount;
    uint8_t  schedule_count;
    uint8_t  next_hour;
    uint8_t  next_min;
};
#pragma pack(pop)

// ---- Callbacks (run on FreeRTOS BLE task) ----

class ServerCallbacks : public NimBLEServerCallbacks
{
    void onDisconnect( NimBLEServer*, NimBLEConnInfo&, int ) override
    {
        NimBLEDevice::getAdvertising()->start();
    }
};

class ScheduleWriteCallback : public NimBLECharacteristicCallbacks
{
public:
    ScheduleWriteCallback( QueueHandle_t q ) : queue( q ) {}

    void onWrite( NimBLECharacteristic* c, NimBLEConnInfo& ) override
    {
        const uint8_t* data = (const uint8_t*)c->getValue().data();
        size_t len = c->getValue().length();
        if ( len < 1 ) return;

        uint8_t cmd = data[0];
        FeederEvent event;

        if ( cmd == 0x01 && len >= 3 ) // Add
        {
            uint8_t h = data[1];
            uint8_t m = data[2];
            if ( h > 23 || m > 59 ) return;
            event.type = FeederEvent::Type::TIME_ADDED;
            event.time_added = { h, m };
            xQueueSend( queue, &event, 0 );
        }
        else if ( cmd == 0x02 && len >= 2 ) // Remove
        {
            event.type = FeederEvent::Type::TIME_REMOVED;
            event.index_removed = data[1];
            xQueueSend( queue, &event, 0 );
        }
    }

private:
    QueueHandle_t queue;
};

class FoodAmountWriteCallback : public NimBLECharacteristicCallbacks
{
public:
    FoodAmountWriteCallback( QueueHandle_t q ) : queue( q ) {}

    void onWrite( NimBLECharacteristic* c, NimBLEConnInfo& ) override
    {
        if ( c->getValue().length() < 1 ) return;
        FeederEvent event;
        event.type = FeederEvent::Type::FOOD_UPDATED;
        event.updated_grams = (uint8_t)c->getValue()[0];
        xQueueSend( queue, &event, 0 );
    }

private:
    QueueHandle_t queue;
};

class ManualFeedWriteCallback : public NimBLECharacteristicCallbacks
{
public:
    ManualFeedWriteCallback( QueueHandle_t q ) : queue( q ) {}

    void onWrite( NimBLECharacteristic* c, NimBLEConnInfo& ) override
    {
        FeederEvent event;
        event.type = FeederEvent::Type::MANUAL_FEED;
        event.manual_feed_grams = ( c->getValue().length() > 0 ) ? (uint8_t)c->getValue()[0] : 0;
        xQueueSend( queue, &event, 0 );
    }

private:
    QueueHandle_t queue;
};

class ClockWriteCallback : public NimBLECharacteristicCallbacks
{
public:
    ClockWriteCallback( QueueHandle_t q ) : queue( q ) {}

    void onWrite( NimBLECharacteristic* c, NimBLEConnInfo& ) override
    {
        if ( c->getValue().length() < 2 ) return;
        FeederEvent event;
        event.type = FeederEvent::Type::CLOCK_UPDATED;
        event.updated_system_clock.hour = (uint8_t)c->getValue()[0];
        event.updated_system_clock.min  = (uint8_t)c->getValue()[1];
        xQueueSend( queue, &event, 0 );
    }

private:
    QueueHandle_t queue;
};

// ---- BLEController ----

void BLEController::begin( const FeedingSchedule& sched, uint8_t init_food, Time_t init_clock )
{
    schedule = &sched;
    food_amount = init_food;
    system_clock = init_clock;

    event_queue = xQueueCreate( kEventQueueDepth, sizeof( FeederEvent ) );

    NimBLEDevice::init( "CatFeeder" );
    NimBLEServer* server = NimBLEDevice::createServer();
    server->setCallbacks( new ServerCallbacks() );

    NimBLEService* service = server->createService( kServiceUUID );

    status_char = service->createCharacteristic(
        kStatusUUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );

    schedule_char = service->createCharacteristic(
        kScheduleUUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
    );
    schedule_char->setCallbacks( new ScheduleWriteCallback( event_queue ) );

    food_amount_char = service->createCharacteristic(
        kFoodAmountUUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    food_amount_char->setCallbacks( new FoodAmountWriteCallback( event_queue ) );

    manual_feed_char = service->createCharacteristic(
        kManualFeedUUID,
        NIMBLE_PROPERTY::WRITE
    );
    manual_feed_char->setCallbacks( new ManualFeedWriteCallback( event_queue ) );

    clock_char = service->createCharacteristic(
        kClockUUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    clock_char->setCallbacks( new ClockWriteCallback( event_queue ) );

    server->start();

    // Set initial characteristic values before advertising
    pushStatus();
    pushSchedule();
    food_amount_char->setValue( &food_amount, 1 );
    uint8_t clk[2] = { system_clock.hour, system_clock.min };
    clock_char->setValue( clk, 2 );

    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
    advertising->setName( "CatFeeder" );
    advertising->addServiceUUID( kServiceUUID );
    advertising->start();
}

void BLEController::update()
{
    if ( millis() - last_status_ms < kStatusIntervalMs ) return;
    last_status_ms = millis();
    pushStatus();
}

void BLEController::setFoodAmount( uint8_t grams )
{
    food_amount = grams;
    if ( food_amount_char )
        food_amount_char->setValue( &food_amount, 1 );
}

void BLEController::setSystemClock( Time_t clock )
{
    system_clock = clock;
    if ( clock_char )
    {
        uint8_t buf[2] = { system_clock.hour, system_clock.min };
        clock_char->setValue( buf, 2 );
    }
}

void BLEController::setLidarMm( int16_t mm )
{
    lidar_mm = mm;
}

void BLEController::notifyScheduleChanged()
{
    pushSchedule();
}

FeederEvent BLEController::pollEvent()
{
    FeederEvent event;
    if ( xQueueReceive( event_queue, &event, 0 ) != pdTRUE )
        event.type = FeederEvent::Type::NONE;
    return event;
}

void BLEController::pushStatus()
{
    if ( !status_char ) return;

    BLEStatus s = {};
    s.hour           = system_clock.hour;
    s.min            = system_clock.min;
    s.lidar_mm       = lidar_mm;
    s.food_amount    = food_amount;
    s.schedule_count = schedule ? schedule->size() : 0;

    if ( schedule && schedule->size() > 0 )
    {
        const Time_t* times = schedule->data();
        const uint8_t count = schedule->size();
        bool found = false;
        for ( uint8_t i = 0; i < count; i++ )
        {
            if ( times[i].hour > system_clock.hour ||
                 ( times[i].hour == system_clock.hour && times[i].min >= system_clock.min ) )
            {
                s.next_hour = times[i].hour;
                s.next_min  = times[i].min;
                found = true;
                break;
            }
        }
        if ( !found ) { s.next_hour = times[0].hour; s.next_min = times[0].min; }
    }

    status_char->setValue( (uint8_t*)&s, sizeof( s ) );
    status_char->notify();
}

void BLEController::pushSchedule()
{
    if ( !schedule_char || !schedule ) return;

    uint8_t buf[1 + MAX_FEEDS * 2];
    const uint8_t count = schedule->size();
    buf[0] = count;
    const Time_t* times = schedule->data();
    for ( uint8_t i = 0; i < count; i++ )
    {
        buf[1 + i * 2]     = times[i].hour;
        buf[1 + i * 2 + 1] = times[i].min;
    }
    schedule_char->setValue( buf, 1 + count * 2 );
    schedule_char->notify();
}
