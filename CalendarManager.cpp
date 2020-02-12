#include "CalendarManager.h"
#include "SpecialDate.h"

enum {
    MODE_NORMAL = 0,
    MODE_SET_YEAR,
    MODE_SET_MONTH,
    MODE_SET_DAY,
    MODE_SET_HOUR,
    MODE_SET_MINUTE,
    MODE_MAX
};

#define YEAR_DISPLAY_MIN    1900
#define DISPLAY_DURATION    30

/*-----------------------------------------------------------------------------------------------*/

#define rgb565(r, g, b) (((r) & 0xf8) << 8 | ((g) & 0xfc) << 3 | ((b) & 0xf8) >> 3)

#define C_BLACK     rgb565(0,   0,   0  )
#define C_DATE      rgb565(255, 255, 192)
#define C_TIME      rgb565(64,  255, 64 )
#define C_SUNDAY    rgb565(255, 160, 160)
#define C_WEEKDAY   rgb565(255, 255, 255)
#define C_SATURDAY  rgb565(192, 192, 255)
#define C_HOLIDAY   rgb565(255, 64,  255)
#define C_MEMORIAL  rgb565(255, 255, 0  )
#define C_TODAY     rgb565(192, 223, 223)
#define C_HIGHLIGHT rgb565(128, 128, 0  )
#define C_DISPLAY   rgb565(192, 192, 192)

#define getColorSelected(z) ((mode == (z)) ? C_HIGHLIGHT : C_BLACK)
#define getColorYoubi(z)    (((z) == 0) ? C_SUNDAY : (((z) == 6) ? C_SATURDAY : C_WEEKDAY))

/*-----------------------------------------------------------------------------------------------*/

PROGMEM static const uint8_t figureLargeBitmap[10][48] = { // 16x24 x10
    {
        0x00, 0x00, 0x0F, 0xC0, 0x1C, 0x70, 0x38, 0x38, 0x38, 0x3C, 0x70, 0x3C, 0x70, 0x1C, 0xF0, 0x1E,
        0xF0, 0x1E, 0xF0, 0x1E, 0xF0, 0x1E, 0xF0, 0x1E, 0xF0, 0x1E, 0xF0, 0x1E, 0xF0, 0x1E, 0xF0, 0x1E,
        0x70, 0x1C, 0x78, 0x3C, 0x78, 0x38, 0x38, 0x38, 0x1C, 0x70, 0x07, 0xC0, 0x00, 0x00, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x01, 0xC0, 0x07, 0xC0, 0x3F, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0,
        0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0,
        0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x3F, 0xFC, 0x00, 0x00, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x07, 0xE0, 0x1C, 0x78, 0x38, 0x3C, 0x70, 0x1E, 0x70, 0x1E, 0x78, 0x1E, 0x7C, 0x1E,
        0x7C, 0x3E, 0x38, 0x3C, 0x00, 0x78, 0x00, 0x70, 0x00, 0xE0, 0x01, 0xC0, 0x03, 0x80, 0x07, 0x00,
        0x0C, 0x06, 0x18, 0x0E, 0x38, 0x0E, 0x7F, 0xFE, 0xFF, 0xFE, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x0F, 0xE0, 0x38, 0x70, 0x30, 0x38, 0x70, 0x3C, 0x78, 0x3C, 0x78, 0x3C, 0x38, 0x3C,
        0x00, 0x38, 0x00, 0x70, 0x0F, 0xC0, 0x00, 0x78, 0x00, 0x3C, 0x00, 0x1C, 0x00, 0x1E, 0x70, 0x1E,
        0xF0, 0x1E, 0xF0, 0x1E, 0xE0, 0x1C, 0x60, 0x3C, 0x70, 0x78, 0x1F, 0xE0, 0x00, 0x00, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x00, 0x18, 0x00, 0x38, 0x00, 0x78, 0x00, 0xF8, 0x00, 0xF8, 0x01, 0xF8, 0x03, 0x78,
        0x03, 0x78, 0x06, 0x78, 0x0C, 0x78, 0x18, 0x78, 0x18, 0x78, 0x30, 0x78, 0x60, 0x78, 0x60, 0x78,
        0xFF, 0xFF, 0x00, 0x78, 0x00, 0x78, 0x00, 0x78, 0x00, 0x78, 0x03, 0xFF, 0x00, 0x00, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x78, 0x0C, 0x7F, 0xF8, 0x7F, 0xF8, 0x6F, 0xE0, 0x60, 0x00, 0x60, 0x00, 0x60, 0x00,
        0x7F, 0xC0, 0x70, 0xF0, 0xE0, 0x38, 0xE0, 0x3C, 0x00, 0x1E, 0x00, 0x1E, 0x70, 0x1E, 0xF0, 0x1E,
        0xF0, 0x1E, 0xF0, 0x1E, 0xE0, 0x1C, 0x60, 0x38, 0x30, 0x70, 0x1F, 0xC0, 0x00, 0x00, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x03, 0xF0, 0x0E, 0x38, 0x1C, 0x3C, 0x38, 0x3C, 0x38, 0x3C, 0x78, 0x38, 0x70, 0x00,
        0xF0, 0x00, 0xF0, 0x00, 0xF3, 0xF0, 0xFE, 0x78, 0xF8, 0x3C, 0xF0, 0x1E, 0xF0, 0x1E, 0xF0, 0x1E,
        0x70, 0x1E, 0x70, 0x1E, 0x70, 0x1C, 0x38, 0x3C, 0x1C, 0x78, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x3F, 0xFE, 0x3F, 0xFE, 0x3F, 0xFC, 0x70, 0x0C, 0x70, 0x18, 0x60, 0x18, 0x60, 0x30,
        0x00, 0x30, 0x00, 0x60, 0x00, 0x60, 0x00, 0x60, 0x00, 0xE0, 0x00, 0xC0, 0x01, 0xC0, 0x01, 0xC0,
        0x01, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x0F, 0xE0, 0x1C, 0x78, 0x38, 0x38, 0x78, 0x3C, 0x78, 0x3C, 0x78, 0x3C, 0x78, 0x38,
        0x7E, 0x78, 0x3F, 0xE0, 0x1F, 0xE0, 0x07, 0xF8, 0x1D, 0xFC, 0x78, 0x3E, 0x70, 0x1E, 0xF0, 0x1E,
        0xF0, 0x1E, 0xF0, 0x1E, 0xF0, 0x1C, 0x78, 0x3C, 0x3C, 0x78, 0x0F, 0xE0, 0x00, 0x00, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x0F, 0xC0, 0x3C, 0x70, 0x78, 0x38, 0x70, 0x3C, 0xF0, 0x1C, 0xF0, 0x1C, 0xF0, 0x1E,
        0xF0, 0x1E, 0xF0, 0x1E, 0x78, 0x3E, 0x3C, 0x7E, 0x1F, 0xDE, 0x00, 0x1E, 0x00, 0x1E, 0x00, 0x1C,
        0x38, 0x1C, 0x78, 0x3C, 0x78, 0x38, 0x78, 0x70, 0x38, 0xE0, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00,
    }
};

PROGMEM static const uint8_t figureMediumBitmap[10][32] = { // 12x16 x10
    {
        0x00, 0x00, 0x0F, 0x00, 0x19, 0x80, 0x31, 0xC0, 0x70, 0xE0, 0x70, 0xE0, 0x70, 0xE0, 0x70, 0xE0,
        0x70, 0xE0, 0x70, 0xE0, 0x70, 0xE0, 0x70, 0xE0, 0x39, 0xC0, 0x39, 0x80, 0x0F, 0x00, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x07, 0x00, 0x3F, 0x00, 0x07, 0x00, 0x07, 0x00, 0x07, 0x00, 0x07, 0x00, 0x07, 0x00,
        0x07, 0x00, 0x07, 0x00, 0x07, 0x00, 0x07, 0x00, 0x07, 0x00, 0x07, 0x00, 0x3F, 0xE0, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x1F, 0x80, 0x31, 0xC0, 0x60, 0xE0, 0x60, 0xE0, 0x78, 0xE0, 0x38, 0xC0, 0x01, 0xC0,
        0x03, 0x80, 0x07, 0x00, 0x0C, 0x00, 0x18, 0x60, 0x30, 0xE0, 0x7F, 0xE0, 0x7F, 0xE0, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x1F, 0x80, 0x71, 0xC0, 0x70, 0xE0, 0x70, 0xE0, 0x00, 0xE0, 0x01, 0xC0, 0x1F, 0x00,
        0x01, 0xC0, 0x00, 0xE0, 0x70, 0xE0, 0x70, 0xE0, 0x60, 0xE0, 0x31, 0xC0, 0x1F, 0x80, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x01, 0x80, 0x03, 0x80, 0x07, 0x80, 0x0F, 0x80, 0x0F, 0x80, 0x1B, 0x80, 0x33, 0x80,
        0x63, 0x80, 0xC3, 0x80, 0xFF, 0xE0, 0x03, 0x80, 0x03, 0x80, 0x03, 0x80, 0x0F, 0xE0, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x30, 0xC0, 0x3F, 0x80, 0x3F, 0x00, 0x30, 0x00, 0x30, 0x00, 0x3F, 0x80, 0x71, 0xC0,
        0x60, 0xE0, 0x00, 0xE0, 0x70, 0xE0, 0x70, 0xE0, 0x60, 0xC0, 0x31, 0xC0, 0x1F, 0x00, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x07, 0xC0, 0x1C, 0xE0, 0x38, 0xE0, 0x30, 0x00, 0x70, 0x00, 0x7F, 0x80, 0x79, 0xC0,
        0x70, 0xE0, 0x70, 0xE0, 0x70, 0xE0, 0x70, 0xE0, 0x30, 0xC0, 0x39, 0xC0, 0x0F, 0x80, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x3F, 0xE0, 0x3F, 0xC0, 0x70, 0xC0, 0x61, 0x80, 0x61, 0x80, 0x03, 0x00, 0x03, 0x00,
        0x03, 0x00, 0x06, 0x00, 0x06, 0x00, 0x0E, 0x00, 0x0E, 0x00, 0x0E, 0x00, 0x0E, 0x00, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x0F, 0x00, 0x19, 0x80, 0x39, 0xC0, 0x39, 0xC0, 0x39, 0xC0, 0x1D, 0x80, 0x0F, 0x80,
        0x1B, 0xC0, 0x31, 0xE0, 0x70, 0xE0, 0x70, 0xE0, 0x70, 0xE0, 0x39, 0xC0, 0x1F, 0x80, 0x00, 0x00,
    }, {
        0x00, 0x00, 0x1F, 0x00, 0x39, 0xC0, 0x30, 0xC0, 0x70, 0xE0, 0x70, 0xE0, 0x70, 0xE0, 0x70, 0xE0,
        0x39, 0xE0, 0x1F, 0xE0, 0x00, 0xE0, 0x00, 0xC0, 0x71, 0xC0, 0x73, 0x80, 0x3F, 0x00, 0x00, 0x00,
    }
};


PROGMEM static const uint8_t youbiBitmap[YOUBI_MAX][10] = { // 11x5 x7
    { 0x6A, 0xC0, 0x8A, 0xA0, 0xEA, 0xA0, 0x2A, 0xA0, 0xC6, 0xA0 }, // SUN
    { 0xE6, 0xC0, 0xEA, 0xA0, 0xAA, 0xA0, 0xAA, 0xA0, 0xAC, 0xA0 }, // MON
    { 0xEA, 0xE0, 0x4A, 0x80, 0x4A, 0xC0, 0x4A, 0x80, 0x46, 0xE0 }, // TUE
    { 0xAE, 0xC0, 0xA8, 0xA0, 0xAC, 0xA0, 0xE8, 0xA0, 0xEE, 0xE0 }, // WED
    { 0xEA, 0xA0, 0x4A, 0xA0, 0x4E, 0xA0, 0x4A, 0xA0, 0x4A, 0x60 }, // THU
    { 0xEE, 0xE0, 0x8A, 0x40, 0xCC, 0x40, 0x8A, 0x40, 0x8A, 0xE0 }, // FRI
    { 0x6E, 0xE0, 0x8A, 0x40, 0xEE, 0x40, 0x2A, 0x40, 0xCA, 0x40 }, // SAT
};

/*-----------------------------------------------------------------------------------------------*/

#ifdef USE_RTC
CalendarManager::CalendarManager(RX8xxxManager &rtc, Adafruit_GFX &gfx) : rtc(rtc), gfx(gfx)
#else
CalendarManager::CalendarManager(Adafruit_GFX &gfx) : gfx(gfx)
#endif
{
    isInisialized = false;
}

void CalendarManager::initialize(void)
{
#ifdef USE_RTC
    getCurrentDateTimeFromRtc();
#else
    /*  Initialize parameters  */
    mode = MODE_NORMAL;
    currentYear = YEAR_DEFAULT;
    currentMonth = MONTH_DEFAULT;
    currentDay = DAY_DEFAULT;
    currentDayMax = calculateDayMax(currentYear, currentMonth);
    currentHour = HOUR_DEFAULT;
    currentMinute = MINUTE_DEFAULT;
    currentSecond = SECOND_DEFAULT;
#endif
    displayYear = currentYear;
    displayMonth = currentMonth;

    /*  Setup display  */
    gfx.fillScreen(C_BLACK);
    gfx.setTextSize(1);
    drawCurrentDate(true);
    drawCurrentTime(true);
    drawCalendarHeader();
    drawCalendar(currentYear, currentMonth);

    isInisialized = true;
}

void CalendarManager::execute(int seconds, bool isPressUp, bool isPressDown, bool isPressSet)
{
    if (!isInisialized) return;

    bool isRefreshCurrent = false;
    int velocity = isPressUp - isPressDown;
    if (isPressSet) {
        mode = (mode + 1) % MODE_MAX;
#ifdef USE_RTC
        if (mode == MODE_NORMAL) {
            rtc.setDateTime(currentYear, currentMonth, currentDay,
                    currentHour, currentMinute, currentSecond);
        }
#endif
        isRefreshCurrent = true;
    }

    if (mode == MODE_NORMAL) {
        if (velocity) {
            displayMonth = adjustParameter(displayMonth + velocity, MONTH_MIN, MONTH_MAX);
            if (velocity > 0 && displayMonth == MONTH_MIN ||
                    velocity < 0 && displayMonth == MONTH_MAX) {
                displayYear = adjustParameter(displayYear + velocity, YEAR_DISPLAY_MIN, YEAR_MAX);
            }
            displayCounter = DISPLAY_DURATION;
        }
        if (seconds > 0) {
            displayCounter -= seconds;
#ifdef USE_RTC
            currentSecond += seconds;
            if (currentSecond > SECOND_MAX) getCurrentDateTimeFromRtc();
#else
            do {
                forwardOneSecond();
            } while (--seconds > 0);
#endif
            if (displayCounter <= 0) {
                displayYear = currentYear;
                displayMonth = currentMonth;
                displayCounter = 0;
            }
        }
    } else {
        if (velocity) {
            switch (mode) {
            case MODE_SET_YEAR:
                currentYear = adjustParameter(currentYear + velocity, YEAR_MIN, YEAR_MAX);
                currentDayMax = calculateDayMax(currentYear, currentMonth);
                if (currentDay > currentDayMax) currentDay = currentDayMax;
                break;
            case MODE_SET_MONTH:
                currentMonth = adjustParameter(currentMonth + velocity, MONTH_MIN, MONTH_MAX);
                currentDayMax = calculateDayMax(currentYear, currentMonth);
                if (currentDay > currentDayMax) currentDay = currentDayMax;
                break;
            case MODE_SET_DAY:
                currentDay = adjustParameter(currentDay + velocity, DAY_MIN, currentDayMax);
                break;
            case MODE_SET_HOUR:
                currentHour = adjustParameter(currentHour + velocity, HOUR_MIN, HOUR_MAX);
                currentSecond = 0;
                break;
            case MODE_SET_MINUTE:
                currentMinute = adjustParameter(currentMinute + velocity, MINUTE_MIN, MINUTE_MAX);
                currentSecond = 0;
                break;
            default:
                break;
            }
        }
        displayYear = currentYear;
        displayMonth = currentMonth;
        displayCounter = 0;
    }
    drawCurrentDate(isRefreshCurrent);
    drawCurrentTime(isRefreshCurrent);
    drawCalendar(displayYear, displayMonth);
}

/*-----------------------------------------------------------------------------------------------*/

#ifdef USE_RTC
void CalendarManager::getCurrentDateTimeFromRtc(void)
{
    rtc.getDateTime(currentYear, currentMonth, currentDay,
            currentHour, currentMinute, currentSecond);
    currentDayMax = calculateDayMax(currentYear, currentMonth);
}
#endif

int CalendarManager::zeller(int year, int month, int day) // ツェラーの公式
{
    if (month <= 2) {
        year--;
        month += 12;
    }
    int z = year % 100, c = year / 100;
    return (day + 26 * (month + 1) / 10 + z + z / 4 + c * 5 + c / 4 + 6) % 7;
}

int CalendarManager::calculateDayMax(int year, int month)
{
    return (month == 2) ?
            ((!(year % 400) || !(year % 4) && (year % 100)) ? 29 : 28) :
            ((month == 4 || month == 6 || month == 9 || month == 11) ? 30 : 31);
}

uint32_t CalendarManager::generateHolidayBits(int year, int month, int startYoubi)
{
    uint32_t ret = 0;
    for (const int *p = holidayTable; p < holidayTable + HOLIDAY_TABLE_SIZE; p++) {
        int specialDate = pgm_read_word(p);
        if (highByte(specialDate) == month) {
            int day = convertSpecialDay(lowByte(specialDate), year, startYoubi);
            bitSet(ret, day);
        }
    }
    return ret;
}

uint32_t CalendarManager::generateMemorialBits(int year, int month, int startYoubi)
{
    uint32_t ret = 0;
    for (const int *p = memorialTable; p < memorialTable + MEMORIAL_TABLE_SIZE; p++) {
        int specialDate = pgm_read_word(p);
        if (highByte(specialDate) == month) {
            int day = convertSpecialDay(lowByte(specialDate), year, startYoubi);
            bitSet(ret, day);
        }
    }
    return ret;
}

void CalendarManager::forwardOneSecond(void)
{
    if (++currentSecond <= SECOND_MAX) return;
    currentSecond = SECOND_MIN;
    if (++currentMinute <= MINUTE_MAX) return;
    currentMinute = MINUTE_MIN;
    if (++currentHour <= HOUR_MAX) return;
    currentHour = HOUR_MIN;
    if (++currentDay <= currentDayMax) return;
    currentDay = DAY_MIN;
    if (++currentMonth > MONTH_MAX) {
        currentMonth = MONTH_MIN;
        currentYear++;
    }
    currentDayMax = calculateDayMax(currentYear, currentMonth);
    return;
}

int CalendarManager::adjustParameter(int value, int minValue, int maxValue)
{
    if (value < minValue) value = maxValue;
    if (value > maxValue) value = minValue;
    return value;
}

/*-----------------------------------------------------------------------------------------------*/

void CalendarManager::drawCurrentDate(bool isEntire)
{
    static int lastYear, lastMonth, lastDay;
    if (isEntire) {
        gfx.drawLine(52, 25, 63, 14, C_DATE);
        gfx.drawLine(88, 25, 99, 14, C_DATE);
        lastYear = lastMonth = lastDay = 0;
    }
    if (lastYear != currentYear || lastMonth != currentMonth || lastDay != currentDay) {
        int youbi = zeller(currentYear, currentMonth, currentDay);
        gfx.drawBitmap(112, 28, youbiBitmap[youbi], 12, 5, getColorYoubi(youbi), C_BLACK);
    }
    if (lastYear != currentYear) {
        drawNumber(40, 12, currentYear, 4, ' ', &drawMediumChar,
                C_DATE, getColorSelected(MODE_SET_YEAR));
        lastYear = currentYear;
    }
    if (lastMonth != currentMonth) {
        drawNumber(76, 12, currentMonth, 2, ' ', &drawMediumChar,
                C_DATE, getColorSelected(MODE_SET_MONTH));
        lastMonth = currentMonth;
    }
    if (lastDay != currentDay) {
        drawNumber(112, 12, currentDay, 2, ' ', &drawMediumChar,
                C_DATE, getColorSelected(MODE_SET_DAY));
        lastDay = currentDay;
    }
}

void CalendarManager::drawCurrentTime(bool isEntire)
{
    static int lastHour, lastMinute, lastSecond;
    if (isEntire) {
        gfx.fillRect(62, 38, 4, 4, C_TIME);
        gfx.fillRect(62, 46, 4, 4, C_TIME);
        lastHour = lastMinute = -1;
    }
    if (isEntire || lastHour != currentHour) {
        drawNumber(40, 32, currentHour, 2, '0', &drawLargeChar,
                C_TIME, getColorSelected(MODE_SET_HOUR));
        lastHour = currentHour;
    }
    if (isEntire || lastMinute != currentMinute) {
        drawNumber(88, 32, currentMinute, 2, '0', &drawLargeChar,
                C_TIME, getColorSelected(MODE_SET_MINUTE));
        lastMinute = currentMinute;
    }
#if 1
    if (isEntire || lastMinute != currentSecond) {
        drawSmallChar(106, 48, ':', C_TIME, C_BLACK);
        drawNumber(118, 48, currentSecond, 2, '0', &drawSmallChar, C_TIME, C_BLACK);
    }
#endif
}

void CalendarManager::drawCalendarHeader(void)
{
    for (int youbi = YOUBI_SUN; youbi <= YOUBI_SAT; youbi++) {
        gfx.drawBitmap(youbi * 18 + 4, 62, youbiBitmap[youbi], 11, 5, getColorYoubi(youbi));
    }
}

void CalendarManager::drawCalendar(int year, int month)
{
    static int lastYear, lastMonth, lastToday;
    bool isCurrent = (year == currentYear && month == currentMonth);
    if (lastYear != year || lastMonth != month) {
        gfx.fillRect(0, 68, 128, 60, C_BLACK);
        gfx.setTextSize(1);
        int youbi = zeller(year, month, 1);
        int dayMax = calculateDayMax(year, month);
        uint32_t holidayBits = generateHolidayBits(year, month, youbi);
        uint32_t memorialBits = generateMemorialBits(year, month, youbi);
        bool isKurikoshi = false;
        int y = 70;
        for (int day = 1; day <= dayMax; day++) {
            int x = youbi * 18 + 4;
            uint16_t color = getColorYoubi(youbi);
            bool isHoliday = bitRead(holidayBits, day);
            if (isHoliday || isKurikoshi) {
                color = (isHoliday) ? C_HOLIDAY : C_SUNDAY;
                isKurikoshi = (youbi == YOUBI_SUN || isHoliday && isKurikoshi);
            }
            if (bitRead(memorialBits, day)) {
                gfx.fillRect(x - 1, y - 1, 13, 9, C_HIGHLIGHT);
                if (color == C_WEEKDAY) color = C_MEMORIAL;
            }
            drawNumber(youbi * 18 + 10, y, day, 2, ' ', &drawSmallChar, color, color);
            if (++youbi > YOUBI_SAT) {
                youbi = YOUBI_SUN;
                y += 10;
            }
        }
        if (!isCurrent) {
            drawNumber(94, 120, year, 4, ' ', &drawSmallChar, C_DISPLAY, C_DISPLAY);
            drawSmallChar(100, 120, '/', C_DISPLAY, C_DISPLAY);
            drawNumber(112, 120, month, 2, ' ', &drawSmallChar, C_DISPLAY, C_DISPLAY);
        }
        lastYear = year;
        lastMonth = month;
        lastToday = 0;
    }
    if (isCurrent) {
        if (lastToday != currentDay) {
            if (lastToday > 0) drawTodayFrame(lastToday, C_BLACK);
            drawTodayFrame(currentDay, C_TODAY);
            lastToday = currentDay;
        }
    } else {
        if (lastToday > 0) drawTodayFrame(lastToday, C_BLACK);
        lastToday = 0;
    }
}

void CalendarManager::drawNumber(int x, int y, int number, int digits, char padding,
        int (CalendarManager::*func)(int, int, char, uint16_t, uint16_t),
        uint16_t fgColor, uint16_t bgColor)
{
    while (digits > 0 || number > 0) {
        char c = (digits > 0 && number == 0) ? padding : '0' + number % 10;
        x -= (this->*func)(x, y, c, fgColor, bgColor);
        digits--;
        number /= 10;
    }
}

int CalendarManager::drawSmallChar(int x, int y, char c, uint16_t fgColor, uint16_t bgColor)
{
    gfx.setTextColor(fgColor, bgColor);
    gfx.setCursor(x, y);
    gfx.print(c);
    return 6;
}

int CalendarManager::drawMediumChar(int x, int y, char c, uint16_t fgColor, uint16_t bgColor)
{
    if (c >= '0' && c <= '9') {
        gfx.drawBitmap(x, y, figureMediumBitmap[c - '0'], 12, 16, fgColor, bgColor);
    } else {
        gfx.fillRect(x, y, 12, 16, bgColor);
    }
    return 12;
}

int CalendarManager::drawLargeChar(int x, int y, char c, uint16_t fgColor, uint16_t bgColor)
{
    if (c >= '0' && c <= '9') {
        gfx.drawFastVLine(x, y, 24, bgColor);
        gfx.drawBitmap(x + 1, y, figureLargeBitmap[c - '0'], 16, 24, fgColor, bgColor);
        gfx.drawFastVLine(x + 17, y, 24, bgColor);
    } else {
        gfx.fillRect(x, y, 18, 24, bgColor);
    }
    return 18;
}

void CalendarManager::drawTodayFrame(int day, uint16_t color)
{
    int offset = zeller(currentYear, currentMonth, 1) + day - 1;
    int x = offset % YOUBI_MAX * 18 + 2, y = offset / YOUBI_MAX * 10 + 68;
    gfx.drawRect(x, y, 15, 11, color);
}
