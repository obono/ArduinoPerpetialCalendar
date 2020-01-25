#pragma once
#include <arduino.h>

enum {
    SPDAY_SUN1 = 32,
                SPDAY_MON1, SPDAY_TUE1, SPDAY_WED1, SPDAY_THU1, SPDAY_FRI1, SPDAY_SAT1,
    SPDAY_SUN2, SPDAY_MON2, SPDAY_TUE2, SPDAY_WED2, SPDAY_THU2, SPDAY_FRI2, SPDAY_SAT2,
    SPDAY_SUN3, SPDAY_MON3, SPDAY_TUE3, SPDAY_WED3, SPDAY_THU3, SPDAY_FRI3, SPDAY_SAT3,
    SPDAY_SUN4, SPDAY_MON4, SPDAY_TUE4, SPDAY_WED4, SPDAY_THU4, SPDAY_FRI4, SPDAY_SAT4,
    SPDAY_SUN5, SPDAY_MON5, SPDAY_TUE5, SPDAY_WED5, SPDAY_THU5, SPDAY_FRI5, SPDAY_SAT5,
    SPDAY_HARU, SPDAY_AKI
};

#define SPECIAL_DATE(month, dayInfo)    ((month) << 8 | dayInfo)

PROGMEM static const int holidayTable[] = {
    SPECIAL_DATE(1,  1),            // 元日
    SPECIAL_DATE(1,  SPDAY_MON2),   // 成人の日
    SPECIAL_DATE(2,  11),           // 建国記念の日
    SPECIAL_DATE(2,  23),           // 天皇誕生日
    SPECIAL_DATE(3,  SPDAY_HARU),   // 春分の日
    SPECIAL_DATE(4,  29),           // 昭和の日
    SPECIAL_DATE(5,  3),            // 憲法記念日
    SPECIAL_DATE(5,  4),            // みどりの日
    SPECIAL_DATE(5,  5),            // こどもの日
    SPECIAL_DATE(7,  SPDAY_MON3),   // 海の日
    SPECIAL_DATE(8,  11),           // 山の日
    SPECIAL_DATE(9,  SPDAY_MON3),   // 敬老の日
    SPECIAL_DATE(9,  SPDAY_AKI),    // 秋分の日
    SPECIAL_DATE(10, SPDAY_MON2),   // スポーツの日
    SPECIAL_DATE(11, 3),            // 文化の日
    SPECIAL_DATE(11, 23),           // 勤労感謝の日
};

PROGMEM static const int memorialTable[] = {
    SPECIAL_DATE(2,  3),            // 節分
    SPECIAL_DATE(2,  14),           // バレンタインデー
    SPECIAL_DATE(3,  3),            // 雛祭り
    SPECIAL_DATE(3,  14),           // ホワイトデー
    SPECIAL_DATE(5,  SPDAY_SUN2),   // 母の日
    SPECIAL_DATE(6,  SPDAY_SUN3),   // 父の日
    SPECIAL_DATE(7,  7),            // 七夕
    SPECIAL_DATE(8,  15),           // 終戦記念日
    SPECIAL_DATE(10, 31),           // ハロウィン
    SPECIAL_DATE(11, 15),           // 七五三
    SPECIAL_DATE(12, 25),           // クリスマス
};

#define HOLIDAY_TABLE_SIZE (sizeof(holidayTable) / sizeof(int))
#define MEMORIAL_TABLE_SIZE (sizeof(memorialTable) / sizeof(int))

/*-----------------------------------------------------------------------------------------------*/

static int calculateNthYoubiDay(int dayInfo, int startYoubi)
{
    dayInfo -= SPDAY_SUN1;
    int youbi = dayInfo % 7, week = dayInfo / 7;
    return (youbi + 7 - startYoubi) % 7 + 1 + week * 7;
}

static int calculateVernalEquinoxDay(int year) // 春分の日
{
    int z = year % 4;
    if (year >= 1900 && year <= 1923) return (z <= 2) ? 21 : 22;
    if (year >= 1924 && year <= 1959) return 21;
    if (year >= 1960 && year <= 1991) return (z == 0) ? 20 : 21;
    if (year >= 1992 && year <= 2023) return (z <= 1) ? 20 : 21;
    if (year >= 2024 && year <= 2055) return (z <= 2) ? 20 : 21;
    if (year >= 2056 && year <= 2091) return 20;
    if (year >= 2092 && year <= 2099) return (z == 0) ? 19 : 20;
    return 21;
}

static int calculateAutumnalEquinoxDay(int year) // 秋分の日
{
    int z = year % 4;
    if (year >= 1900 && year <= 1919) return (z == 0) ? 23 : 24;
    if (year >= 1920 && year <= 1947) return (z <= 1) ? 23 : 24;
    if (year >= 1948 && year <= 1979) return (z <= 2) ? 23 : 24;
    if (year >= 1980 && year <= 2011) return 23;
    if (year >= 2012 && year <= 2043) return (z == 0) ? 22 : 23;
    if (year >= 2044 && year <= 2075) return (z <= 1) ? 22 : 23;
    if (year >= 2076 && year <= 2099) return (z <= 2) ? 22 : 23;
    return 23;
}

static int convertSpecialDay(int dayInfo, int year, int startYoubi)
{
    if (dayInfo > 0 && dayInfo < SPDAY_MON1) {
        return dayInfo;
    } else if (dayInfo >= SPDAY_MON1 && dayInfo <= SPDAY_MON5) {
        return calculateNthYoubiDay(dayInfo, startYoubi);
    } else if (dayInfo == SPDAY_HARU) {
        return calculateVernalEquinoxDay(year);
    } else if (dayInfo == SPDAY_AKI) {
        return calculateAutumnalEquinoxDay(year);
    } else {
        return 0;
    }
}
