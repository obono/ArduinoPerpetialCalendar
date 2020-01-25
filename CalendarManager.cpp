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

#define YEAR_MIN    1900
#define YEAR_MAX    2099
#define MONTH_MIN   1
#define MONTH_MAX   12
#define DAY_MIN     1
#define HOUR_MIN    0
#define HOUR_MAX    23
#define MINUTE_MIN  0
#define MINUTE_MAX  59
#define SECOND_MIN  0
#define SECOND_MAX  59

enum {
    YOUBI_SUN = 0,
    YOUBI_MON,
    YOUBI_TUE,
    YOUBI_WED,
    YOUBI_THU,
    YOUBI_FRI,
    YOUBI_SAT,
    YOUBI_MAX
};

#define YEAR_DISPLAY_MIN    1900
#define DISPLAY_DURATION    30

/*-----------------------------------------------------------------------------------------------*/

#define rgb565(r, g, b) (((r) & 0xf8) << 8 | ((g) & 0xfc) << 3 | ((b) & 0xf8) >> 3)

#define C_BLACK     rgb565(0,   0,   0  )
#define C_DATE      rgb565(255, 255, 192)
#define C_TIME      rgb565(64,  255, 64 )
#define C_SUNDAY    rgb565(255, 160, 160)
#define C_WEEKDAY   rgb565(240, 255, 255)
#define C_SATURDAY  rgb565(192, 192, 255)
#define C_HOLIDAY   rgb565(255, 64,  255)
#define C_MEMORIAL  rgb565(255, 255, 0  )
#define C_TODAY     rgb565(192, 223, 223)
#define C_HIGHLIGHT rgb565(128, 128, 0  )
#define C_DISPLAY   rgb565(192, 192, 192)

#define getColorSelected(z) ((mode == (z)) ? C_HIGHLIGHT : C_BLACK)
#define getColorYoubi(z)    (((z) == 0) ? C_SUNDAY : (((z) == 6) ? C_SATURDAY : C_WEEKDAY))

/*-----------------------------------------------------------------------------------------------*/

PROGMEM static const uint8_t youbiBitmap[YOUBI_MAX][10] = { // 11x5 x7
    { 0x6A, 0xC0, 0x8A, 0xA0, 0xEA, 0xA0, 0x2A, 0xA0, 0xC6, 0xA0 }, // SUN
    { 0xEE, 0xC0, 0xEA, 0xA0, 0xAA, 0xA0, 0xAA, 0xA0, 0xAE, 0xA0 }, // MON
    { 0xEA, 0xE0, 0x4A, 0x80, 0x4A, 0xC0, 0x4A, 0x80, 0x46, 0xE0 }, // TUE
    { 0xAE, 0xC0, 0xA8, 0xA0, 0xAC, 0xA0, 0xE8, 0xA0, 0xEE, 0xE0 }, // WED
    { 0xEA, 0xA0, 0x4A, 0xA0, 0x4E, 0xA0, 0x4A, 0xA0, 0x4A, 0x60 }, // THU
    { 0xEE, 0xE0, 0x8A, 0x40, 0xCC, 0x40, 0x8A, 0x40, 0x8A, 0xF0 }, // FRI
    { 0x6E, 0xE0, 0x8A, 0x40, 0xEE, 0x40, 0x2A, 0x40, 0xCA, 0x40 }, // SAT
};

/*-----------------------------------------------------------------------------------------------*/

CalendarManager::CalendarManager(Adafruit_GFX &gfx) : gfx(gfx)
{
    isInisialized = false;
}

void CalendarManager::initialize(int year, int month, int day)
{
    /*  Initialize parameters  */
    mode = MODE_NORMAL;
    currentYear = year;
    currentMonth = month;
    currentDay = day;
    currentDayMax = calculateDayMax(currentYear, currentMonth);
    currentHour = HOUR_MIN;
    currentMinute = MINUTE_MIN;
    currentSecond = SECOND_MIN;
    displayYear = currentYear;
    displayMonth = currentMonth;

    /*  Setup display  */
    gfx.fillScreen(C_BLACK);
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
        isRefreshCurrent = true;
    }

    if (mode == MODE_NORMAL) {
        if (velocity) {
            displayMonth = adjustParameter(displayMonth + velocity, MONTH_MIN, MONTH_MAX);
            if (velocity > 0 && displayMonth == MONTH_MIN ||
                    velocity < 0 && displayMonth == MONTH_MAX) {
                displayYear = adjustParameter(displayYear + velocity, YEAR_MIN, YEAR_MAX);
            }
            displayCounter = DISPLAY_DURATION;
        }
        while (seconds > 0) {
            forwardOneSecond();
            if (displayCounter > 0) displayCounter--;
            seconds--;
        }
        if (displayCounter == 0) {
            displayYear = currentYear;
            displayMonth = currentMonth;
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
        displayCounter = 0;
        displayYear = currentYear;
        displayMonth = currentMonth;
    }
    drawCurrentDate(isRefreshCurrent);
    drawCurrentTime(isRefreshCurrent);
    drawCalendar(displayYear, displayMonth);
}

/*-----------------------------------------------------------------------------------------------*/

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
    gfx.setTextSize(2);
    if (isEntire) {
        gfx.setTextColor(C_DATE, C_BLACK);
        drawChar(52, 12, '/');
        drawChar(88, 12, '/');
        lastYear = lastMonth = lastDay = 0;
    }
    if (lastYear != currentYear || lastMonth != currentMonth || lastDay != currentDay) {
        int youbi = zeller(currentYear, currentMonth, currentDay);
        gfx.drawBitmap(112, 28, youbiBitmap[youbi], 12, 5, getColorYoubi(youbi), C_BLACK);
    }
    if (lastYear != currentYear) {
        gfx.setTextColor(C_DATE, getColorSelected(MODE_SET_YEAR));
        drawFigures(4, 12, currentYear, '\0');
        lastYear = currentYear;
    }
    if (lastMonth != currentMonth) {
        gfx.setTextColor(C_DATE, getColorSelected(MODE_SET_MONTH));
        drawFigures(64, 12, currentMonth, ' ');
        lastMonth = currentMonth;
    }
    if (lastDay != currentDay) {
        gfx.setTextColor(C_DATE, getColorSelected(MODE_SET_DAY));
        drawFigures(100, 12, currentDay, ' ');
        lastDay = currentDay;
    }
}

void CalendarManager::drawCurrentTime(bool isEntire)
{
    static int lastHour, lastMinute, lastSecond;
    gfx.setTextSize(3);
    if (isEntire) {
        gfx.setTextColor(C_TIME, C_BLACK);
        drawChar(55, 32, ':');
        lastHour = lastMinute = -1;
    }
    if (isEntire || lastHour != currentHour) {
        gfx.setTextColor(C_TIME, getColorSelected(MODE_SET_HOUR));
        drawFigures(22, 32, currentHour, '0');
        lastHour = currentHour;
    }
    if (isEntire || lastMinute != currentMinute) {
        gfx.setTextColor(C_TIME, getColorSelected(MODE_SET_MINUTE));
        drawFigures(70, 32, currentMinute, '0');
        lastMinute = currentMinute;
    }
#if 1
    if (isEntire || lastMinute != currentSecond) {
        gfx.setTextSize(1);
        gfx.setTextColor(C_TIME, C_BLACK);
        drawChar(106, 48, ':');
        drawFigures(112, 48, currentSecond, '0');
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
            gfx.setTextColor(color);
            drawFigures(youbi * 18 + 4, y, day, ' ');
            if (++youbi > YOUBI_SAT) {
                youbi = YOUBI_SUN;
                y += 10;
            }
        }
        if (!isCurrent) {
            gfx.setTextColor(C_DISPLAY);
            drawFigures(76, 120, year, '\0');
            drawChar(100, 120, '/');
            drawFigures(106, 120, month, ' ');
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

void CalendarManager::drawFigures(int x, int y, int num, char padding)
{
    gfx.setCursor(x, y);
    if (padding && num < 10) gfx.print(padding);
    gfx.print(num);
}

void CalendarManager::drawChar(int x, int y, char c)
{
    gfx.setCursor(x, y);
    gfx.print(c);
}

void CalendarManager::drawTodayFrame(int day, uint16_t color)
{
    int offset = zeller(currentYear, currentMonth, 1) + day - 1;
    int x = offset % YOUBI_MAX * 18 + 2, y = offset / YOUBI_MAX * 10 + 68;
    gfx.drawRect(x, y, 15, 11, color);
}