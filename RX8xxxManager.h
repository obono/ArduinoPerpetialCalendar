#pragma once

#define YEAR_MIN    2000
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

#define YEAR_DEFAULT    2020
#define MONTH_DEFAULT   1
#define DAY_DEFAULT     1
#define HOUR_DEFAULT    0
#define MINUTE_DEFAULT  0
#define SECOND_DEFAULT  0

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

enum {
    PERIODICAL_OFF = 0,
    PERIODICAL_ALWAYS_ON,
    PERIODICAL_2HZ,
    PERIODICAL_1HZ,
    PERIODICAL_EVERY_SECOND,
    PERIODICAL_EVERY_MINUTE,
    PERIODICAL_EVERY_HOUR,
    PERIODICAL_EVERY_MONTH,
};

class RX8xxxManager
{
public:
    RX8xxxManager(void);
    void    initialize(void);
    void    initialize(int mode);
    void    clearInterrupedFlags(void);
    void    getDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second);
    void    setDateTime(int year, int month, int day, int hour, int minute, int second);

private:
    static int bcd2value(int bcd);
    static int value2bcd(int value);

    bool    isInitialized;
};
