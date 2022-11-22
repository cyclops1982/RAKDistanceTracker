#include "batteryhelper.h"

/**
 * @brief Get RAW Battery Voltage
 * @returns minivolts in the battery.
 */
uint16_t BatteryHelper::readVBAT(void)
{

    uint32_t vbat_pin = PIN_VBAT;

    unsigned int sum = 0, average_value = 0;
    unsigned int read_temp[10] = {0};
    unsigned char i = 0;
    unsigned int adc_max = 0;
    unsigned int adc_min = 4095;

    analogReadResolution(12);
    average_value = analogRead(vbat_pin);
    for (i = 0; i < 10; i++)
    {
        read_temp[i] = analogRead(vbat_pin);
        if (read_temp[i] < adc_min)
        {
            adc_min = read_temp[i];
        }
        if (read_temp[i] > adc_max)
        {
            adc_max = read_temp[i];
        }
        sum = sum + read_temp[i];
    }
    average_value = (sum - adc_max - adc_min) >> 3;

    // Convert the raw value to compensated mv, taking the resistor-
    // divider into account (providing the actual LIPO voltage)
    // ADC range is 0..3300mV and resolution is 12-bit (0..4095)
    uint16_t volt = average_value * REAL_VBAT_MV_PER_LSB;

    return volt;
}

/**
 * @brief Convert from raw mv to percentage
 * @param mvolts
 *    RAW Battery Voltage
 */
uint8_t BatteryHelper::mvToPercent(uint16_t mvolts)
{
    if (mvolts < 3300)
        return 0;

    if (mvolts < 3600)
    {
        mvolts -= 3300;
        return mvolts / 30;
    }

    mvolts -= 3600;
    return 10 + (mvolts * 0.15F); // thats mvolts /6.66666666
}

uint8_t BatteryHelper::GetLoRaWanBattVal()
{
    uint16_t mvolts = BatteryHelper::readVBAT();
    if (mvolts < 3300)
        return 0;

    if (mvolts < 3600)
    {
        mvolts -= 3300;
        return mvolts / 30 * 2.55;
    }

    mvolts -= 3600;
    return (10 + (mvolts * 0.15F)) * 2.55;
}
