#include "Clock.h"

Clock::Clock() : _lastMillis(0), _seconds(0) {}

void Clock::setTime(uint8_t hh, uint8_t mm, uint8_t ss)
{
    if (hh >= 24)
        hh = 0;
    if (mm >= 60)
        mm = 0;
    if (ss >= 60)
        ss = 0;
    _seconds = (unsigned long)hh * 3600UL + (unsigned long)mm * 60UL + (unsigned long)ss;
    _lastMillis = millis();
}

String Clock::getTime() const
{
    unsigned long secs = _seconds;
    // compute hh:mm:ss from secs (seconds since midnight)
    unsigned long hh = secs / 3600UL;
    unsigned long mm = (secs % 3600UL) / 60UL;
    unsigned long ss = secs % 60UL;
    char buf[9];
    sprintf(buf, "%02lu:%02lu:%02lu", hh, mm, ss);
    return String(buf);
}

unsigned long Clock::secondsSinceMidnight() const
{
    return _seconds;
}

void Clock::update()
{
    unsigned long now = millis();
    unsigned long deltaMs = now - _lastMillis;
    if (deltaMs >= 1000UL)
    {
        unsigned long addSec = deltaMs / 1000UL;
        _seconds += addSec;
        _lastMillis += addSec * 1000UL;
        // wrap around after 24h
        if (_seconds >= 86400UL)
        {
            _seconds %= 86400UL;
        }
    }
}
