#pragma once

#include <Arduino.h>
#include <EEPROM.h>

class EEPROMHelper
{
public:
    EEPROMHelper(int addrStart = 0, int maxLen = 128);

    // Initialize EEPROM if necessary (ESP32/ESP8266)
    void begin();

    // Write and read helpers
    void writeString(int addrOffset, const String &strToWrite);
    String readString(int addrOffset);

    // Accessors
    int getAddrStart() const;
    int getMaxLen() const;

private:
    int _addrStart;
    int _maxLen;
};
