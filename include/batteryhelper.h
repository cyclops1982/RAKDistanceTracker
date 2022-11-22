
#ifndef BATTERYHELPER_H
#define BATTERYHELPER_H

#include <Arduino.h>

#define VBAT_MV_PER_LSB (0.806F)   // 3.0V ADC range and 12 - bit ADC resolution = 3300mV / 4096
#define VBAT_DIVIDER (0.6F)        // 1.5M + 1M voltage divider on VBAT = (1.5M / (1M + 1.5M))
#define VBAT_DIVIDER_COMP (1.846F) //  // Compensation factor for the VBAT divider
#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)
#define PIN_VBAT WB_A0

class BatteryHelper
{
public:
    /**
     * @brief Get RAW Battery Voltage
     * @return uint16_t minivolts in the battery.
     */
    static uint16_t readVBAT(void);

    /**
     * @brief Converts milivolts to a percentage
     *
     * @param mvolts
     * @return uint8_t The percentage of charge
     */
    static uint8_t mvToPercent(uint16_t mvolts);

    /**
     * @brief Convert milvolts to 0..255 value.
     *
     * @param mvolts
     * @return uint8_t
     */
    static uint8_t GetLoRaWanBattVal(void);

private:
    BatteryHelper() {}
};

#endif