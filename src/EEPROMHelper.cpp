#include "EEPROMHelper.h"

EEPROMHelper::EEPROMHelper(int addrStart, int maxLen)
    : _addrStart(addrStart), _maxLen(maxLen) {}

void EEPROMHelper::begin()
{
    // For AVR (Uno/Nano) nothing special needed. For ESP platforms, user can
    // uncomment and call EEPROM.begin(size) in their setup if desired.
    // EEPROM.begin(_maxLen + 2);
}

void EEPROMHelper::writeString(int addrOffset, const String &strToWrite)
{
    byte len = strToWrite.length();
    if (len > _maxLen)
    {
        len = _maxLen;
    }
    EEPROM.write(addrOffset, len);
    for (int i = 0; i < len; i++)
    {
        EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
    }
    EEPROM.write(addrOffset + 1 + len, '\0');
    // Note: on ESP32/ESP8266 call EEPROM.commit() after writes if using EEPROM.begin()
}

String EEPROMHelper::readString(int addrOffset)
{
    int newStrLen = EEPROM.read(addrOffset);
    if (newStrLen == 0xFF || newStrLen == 0 || newStrLen > _maxLen)
    {
        return String("");
    }
    char data[newStrLen + 1];
    for (int i = 0; i < newStrLen; i++)
    {
        data[i] = EEPROM.read(addrOffset + 1 + i);
    }
    data[newStrLen] = '\0';
    return String(data);
}

int EEPROMHelper::getAddrStart() const { return _addrStart; }

int EEPROMHelper::getMaxLen() const { return _maxLen; }
