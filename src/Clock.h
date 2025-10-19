#pragma once

#include <Arduino.h>

class Clock
{
public:
    // Constructs a clock with optional initial time
    Clock();

    // Set time (24-hour)
    void setTime(uint8_t hh, uint8_t mm, uint8_t ss);

    // Get time as HH:MM:SS
    String getTime() const;

    // Return seconds since midnight
    unsigned long secondsSinceMidnight() const;

    // Update internal millis baseline (call in loop if needed)
    void update();

private:
    unsigned long _lastMillis; // last reference millis
    unsigned long _seconds;    // seconds since midnight
};
