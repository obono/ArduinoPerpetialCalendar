#pragma once
#include <Adafruit_GFX.h>
#include "RX8xxxManager.h"

//#define USE_RTC

class CalendarManager
{
public:
#ifdef USE_RTC
    CalendarManager(RX8xxxManager &rtc, Adafruit_GFX &gfx);
#else
    CalendarManager(Adafruit_GFX &gfx);
#endif
    void    initialize(void);
    void    execute(int seconds, bool isPressUp, bool isPressDown, bool isPressSet);

private:
#ifdef USE_RTC
    void    getCurrentDateTimeFromRtc(void);
#endif
    int     zeller(int year, int month, int day);
    int     calculateDayMax(int year, int month);
    uint32_t generateHolidayBits(int year, int month, int startYoubi);
    uint32_t generateMemorialBits(int year, int month, int startYoubi);
    void    forwardOneSecond(void);
    int     adjustParameter(int value, int minValue, int maxValue);
    void    drawCurrentDate(bool isEntire);
    void    drawCurrentTime(bool isEntire);
    void    drawCalendarHeader(void);
    void    drawCalendar(int year, int month);
    void    drawNumber(int x, int y, int number, int digits, char padding,
                int (CalendarManager::*func)(int, int, char, uint16_t, uint16_t),
                uint16_t fgColor, uint16_t bgColor);
    int     drawSmallChar(int x, int y, char c, uint16_t fgColor, uint16_t bgColor);
    int     drawMediumChar(int x, int y, char c, uint16_t fgColor, uint16_t bgColor);
    int     drawLargeChar(int x, int y, char c, uint16_t fgColor, uint16_t bgColor);
    void    drawTodayFrame(int day, uint16_t color);

#ifdef USE_RTC
    RX8xxxManager   &rtc;
#endif
    Adafruit_GFX    &gfx;
    bool    isInisialized;
    int     mode;
    int     currentYear, currentMonth, currentDay, currentDayMax;
    int     currentHour, currentMinute, currentSecond;
    int     displayYear, displayMonth, displayCounter;
};
