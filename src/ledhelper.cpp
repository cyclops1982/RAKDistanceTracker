#include "ledhelper.h"

bool LedHelper::isInitialized = false;

void LedHelper::init()
{
    if (!isInitialized)
    {
        pinMode(LED_BLUE, OUTPUT);
        pinMode(LED_GREEN, OUTPUT);
        delay(100);
        digitalWrite(LED_BLUE, LOW);
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_BLUE, HIGH);
        digitalWrite(LED_GREEN, HIGH);
        delay(500);
        digitalWrite(LED_BLUE, LOW);
        digitalWrite(LED_GREEN, LOW);

        isInitialized = true;
    }
}

void LedHelper::BlinkDelay(int ledpin, int waittime)
{
    init();
    if (ledpin != LED_BLUE && ledpin != LED_GREEN)
    {
        delay(waittime * 2);
    }
    digitalWrite(ledpin, HIGH);
    delay(waittime);
    digitalWrite(ledpin, LOW);
    delay(waittime);
}

void LedHelper::BlinkHalt()
{
    init();
    while (1)
    {
        // SOS
        for (short i = 0; i < 3; i++)
        {
            digitalWrite(LED_BLUE, HIGH);
            delay(100);
            digitalWrite(LED_BLUE, LOW);
            delay(100);
        }
        for (short i = 0; i < 3; i++)
        {
            digitalWrite(LED_BLUE, HIGH);
            delay(500);
            digitalWrite(LED_BLUE, LOW);
            delay(500);
        }
        for (short i = 0; i < 3; i++)
        {
            digitalWrite(LED_BLUE, HIGH);
            delay(100);
            digitalWrite(LED_BLUE, LOW);
            delay(100);
        }
        delay(500);
    }
}
