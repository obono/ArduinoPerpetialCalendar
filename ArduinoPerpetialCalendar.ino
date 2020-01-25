#include <arduino.h>
#include <Adafruit_ST7735.h>

#include "CalendarManager.h"

#define VERSION     "0.01"

#define TFT_CS      10
#define TFT_RST     9
#define TFT_DC      8

#define BTN_UP      7
#define BTN_DOWN    6
#define BTN_SET     5
#define BTN_BITMASK 0b11100000

#define FPS                 20
#define US_PER_FRAME        50041UL
#define US_PER_FRAME_256TH  87

Adafruit_ST7735 gfx = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST); // SCL=13, SDA=11
CalendarManager calMan = CalendarManager(gfx);
int             frames;
uint8_t         adjuster;
unsigned long   targetUs;

/*-----------------------------------------------------------------------------------------------*/

void setup()
{
    /*  Setup pins  */
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_SET, INPUT_PULLUP);

    /*  Initalize display and calendar  */
    gfx.initR(INITR_144GREENTAB);
    gfx.setRotation(3); // 270 degrees
    calMan.initialize(2020, 1, 1);

    /*  Timing parameters  */
    frames = 0;
    adjuster = 0;
    targetUs = micros();
}

void loop()
{
    static uint8_t lastButtonsState = 0;
    uint8_t buttonsState = ~PIND & BTN_BITMASK;
    uint8_t buttonsEdge = (lastButtonsState ^ buttonsState) & buttonsState;
    lastButtonsState = buttonsState;

    int seconds = frames / FPS;
    if (seconds || buttonsEdge) {
        bool isPressUp   = bitRead(buttonsEdge, BTN_UP);
        bool isPressDown = bitRead(buttonsEdge, BTN_DOWN);
        bool isPressSet  = bitRead(buttonsEdge, BTN_SET);
        calMan.execute(seconds, isPressUp, isPressDown, isPressSet);
        frames -= seconds * FPS;
    }

    targetUs += US_PER_FRAME;
    if (adjuster < US_PER_FRAME_256TH) targetUs++;
    adjuster += US_PER_FRAME_256TH;
    unsigned long delayUs = targetUs - micros();
    if (!bitRead(delayUs, 31)) delay(delayUs / 1000UL);
    frames++;
}
