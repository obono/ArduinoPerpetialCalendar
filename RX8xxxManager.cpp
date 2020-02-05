#include <Arduino.h>
#include <Wire.h>
#include "RX8xxxManager.h"

#define I2C_ADRS    0x32
#define YEAR_OFFSET 2000

#define afterTransmission() delay(1)

RX8xxxManager::RX8xxxManager(void)
{
    isInitialized = false;
}

void RX8xxxManager::initialize(void)
{
    initialize(PERIODICAL_OFF);
}

void RX8xxxManager::initialize(int mode)
{
    if (mode < PERIODICAL_OFF || mode > PERIODICAL_EVERY_MONTH) mode = PERIODICAL_OFF;
    Wire.begin();
    Wire.requestFrom(I2C_ADRS, 1, false);
    int status = Wire.read();
    Wire.beginTransmission(I2C_ADRS);
    Wire.write(0xE0);
    Wire.write(0b00100000 | mode);  // 24 hour mode | interrupt mode
    Wire.write(0b00100000); // clear PON & VDET, set XST
    Wire.endTransmission();
    afterTransmission();
    if (bitRead(status, 4) | !bitRead(status, 5)) {
        setDateTime(YEAR_DEFAULT, MONTH_DEFAULT, DAY_DEFAULT,
                HOUR_DEFAULT, MINUTE_DEFAULT, SECOND_DEFAULT);
    }
    isInitialized = true;
}

void RX8xxxManager::clearInterrupedFlags(void)
{
    if (!isInitialized) return;
    Wire.requestFrom(I2C_ADRS, 1, false);
    int status = Wire.read();
    Wire.beginTransmission(I2C_ADRS);
    Wire.write(0xF0);
    Wire.write(status & ~(0b00000100)); // clear CTFG
    Wire.endTransmission();
    afterTransmission();
}

void RX8xxxManager::getDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second)
{
    if (!isInitialized) return;
    Wire.requestFrom(I2C_ADRS, 8);
    Wire.read(); // skip
    second = bcd2value(Wire.read());
    minute = bcd2value(Wire.read());
    hour   = bcd2value(Wire.read());
    Wire.read(); // skip
    day    = bcd2value(Wire.read());
    month  = bcd2value(Wire.read());
    year   = bcd2value(Wire.read()) + YEAR_OFFSET;
}

void RX8xxxManager::setDateTime(int year, int month, int day, int hour, int minute, int second)
{
    if (!isInitialized) return;
    year = constrain(year, YEAR_MIN, YEAR_MAX);
    Wire.beginTransmission(I2C_ADRS);
    Wire.write(0x00);
    Wire.write(value2bcd(second));
    Wire.write(value2bcd(minute));
    Wire.write(value2bcd(hour));
    Wire.write(0x00); // don't care
    Wire.write(value2bcd(day));
    Wire.write(value2bcd(month));
    Wire.write(value2bcd(year - YEAR_OFFSET));
    Wire.endTransmission();
    afterTransmission();
}

int RX8xxxManager::bcd2value(int bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0x0f);
}

int RX8xxxManager::value2bcd(int value)
{
    return (value / 10) << 4 | (value % 10);
}
