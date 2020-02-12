#include <arduino.h>
#include <Adafruit_ST7735.h>
#include "CalendarManager.h"
#ifdef USE_RTC
#include <avr/sleep.h>
#include "RX8xxxManager.h"
#endif

#define VERSION     "0.03"

#define TFT_CS      10
#define TFT_RST     9
#define TFT_DC      8

#define BTN_UP      7
#define BTN_DOWN    6
#define BTN_SET     5
#define BTN_BITMASK 0b11100000

#ifdef USE_RTC
#define RTC_INTA    2
#else
#define FPS                 20
#define US_PER_FRAME        49978UL
#define US_PER_FRAME_256TH  86
#endif

Adafruit_ST7735 gfx = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST); // SCL=13, SDA=11
#ifdef USE_RTC
RX8xxxManager   rtcMan;
CalendarManager calMan = CalendarManager(rtcMan, gfx);
volatile int    rtcInterruptCounter;
#else
CalendarManager calMan = CalendarManager(gfx);
int             frames;
uint8_t         adjuster;
unsigned long   targetUs;
#endif

/*-----------------------------------------------------------------------------------------------*/

void setup()
{
    /*  Setup pins  */
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_SET, INPUT_PULLUP);
#ifdef USE_RTC
    pinMode(RTC_INTA, INPUT);
#endif

    /*  Initalize display and calendar  */
    gfx.initR(INITR_144GREENTAB);
    gfx.setRotation(3); // 270 degrees
#ifdef USE_RTC
    rtcMan.initialize(PERIODICAL_1HZ);
#endif
    calMan.initialize();

#ifdef USE_RTC
    /*  Enable level trigger interrupt  */
    attachInterrupt(0, rtcInterruptHandler, LOW);

    /*  Enable pin charge interrupts  */
    PCICR  = 0; // Disable all PCI
    PCMSK0 = 0;
    PCMSK1 = 0;
    PCMSK2 = 0b11100000; // Enable PCINT21 - 23
    PCICR  = 0b00000100; // Enable PCI2 only

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
#else
    /*  Timing parameters  */
    frames = 0;
    adjuster = 0;
    targetUs = micros();
#endif
}

void loop()
{
    static uint8_t lastButtonsState = 0;
    uint8_t buttonsState = ~PIND & BTN_BITMASK;
    uint8_t buttonsEdge = (lastButtonsState ^ buttonsState) & buttonsState;
    lastButtonsState = buttonsState;

#ifdef USE_RTC
    int seconds = rtcInterruptCounter;
#else
    int seconds = frames / FPS;
#endif
    if (seconds || buttonsEdge) {
        bool isPressUp   = bitRead(buttonsEdge, BTN_UP);
        bool isPressDown = bitRead(buttonsEdge, BTN_DOWN);
        bool isPressSet  = bitRead(buttonsEdge, BTN_SET);
        calMan.execute(seconds, isPressUp, isPressDown, isPressSet);
#ifdef USE_RTC
        rtcInterruptCounter -= seconds;
#else
        frames -= seconds * FPS;
#endif
    }

#ifdef USE_RTC
    if (rtcInterruptCounter == 0) sleep();
#else
    targetUs += US_PER_FRAME;
    if (adjuster < US_PER_FRAME_256TH) targetUs++;
    adjuster += US_PER_FRAME_256TH;
    unsigned long delayUs = targetUs - micros();
    if (!bitRead(delayUs, 31)) delay(delayUs / 1000UL);
    frames++;
#endif
}

#ifdef USE_RTC
static void sleep(void)
{
    cli();
    sleep_enable();
    sleep_bod_disable();
    sei();
    sleep_cpu();
    /* wake up here */
    sleep_disable();
}

ISR(PCINT2_vect)
{
    // do nothing
}

static void rtcInterruptHandler(void)
{
    rtcInterruptCounter++;
}
#endif
