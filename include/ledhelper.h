#ifndef LEDHELPER_H
#define LEDHELPER_H
#include <Arduino.h>

class LedHelper
{
public:
    static void init();
    static void BlinkHalt();
    static void BlinkDelay(int ledpin, int delay);
    static bool isInitialized;
};
#endif