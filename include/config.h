#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>
#include <lorahelper.h>

enum ConfigType
{
    SleepTime = 0x01,
    LORA_TXPower = 0x40,
    LORA_DataRate = 0x41,
    LORA_ADREnabled = 0x42,
    LORA_RequireConfirmation = 0x43
};

struct ConfigOption
{
    const char *name;
    ConfigType configType;
    size_t sizeOfOption;
    void *value;
    void (*setfunc)(const ConfigOption *opt, uint8_t *arr);
};

struct ConfigurationParameters
{

    // This is basically the configuration options we use.
    // They can be updated remotely, although this might not make sense.
    // After restart, we get back to the defaults.
    uint16_t _sleeptime = 300; // in seconds
    int8_t _loraDataRate = DR_5;
    int8_t _loraTXPower = TX_POWER_7;
    bool _loraADREnabled = false;
    //   uint8_t _loraDevEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x08, 0xDD, 0xB1}; //SheepTracker 1
    //   uint8_t _loraDevEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x08, 0xF5, 0x2B}; //SheepTracker 2
    uint8_t _loraDevEUI[8] = {0xAC, 0x1F, 0x09, 0xFF, 0xFE, 0x06, 0xBE, 0x44}; // Distance thing (RP2040)
    uint8_t _loraNodeAppEUI[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t _loraNodeAppKey[16] = {0xD1, 0xAA, 0xDB, 0x5B, 0x6D, 0xA2, 0x9D, 0x1E, 0x1C, 0x84, 0x6B, 0xBE, 0xEC, 0x25, 0x21, 0x42};
    bool _loraRequireConfirmation = true;

    static void SetUint32(const ConfigOption *option, uint8_t *arr);
    static void SetUint16(const ConfigOption *option, uint8_t *arr);
    static void SetUint8(const ConfigOption *option, uint8_t *arr);
    static void SetInt8(const ConfigOption *option, uint8_t *arr);
    static void SetBool(const ConfigOption *option, uint8_t *arr);

public:
    uint32_t GetSleepTimeInSeconds() { return _sleeptime; }

    uint8_t GetLoraTXPower() { return _loraTXPower; }
    uint8_t GetLoraDataRate() { return _loraDataRate; }
    bool GetLoraADREnabled() { return _loraADREnabled; }
    uint8_t *GetLoraDevEUI() { return _loraDevEUI; }
    uint8_t *GetLoraNodeAppEUI() { return _loraNodeAppEUI; }
    uint8_t *GetLoraAppKey() { return _loraNodeAppKey; }
    bool GetLoraRequireConfirmation() { return _loraRequireConfirmation; }
    void SetConfig(uint8_t *array, uint8_t length);

} g_configParams;

static const ConfigOption g_configs[] = {
    {"Sleep time between updates (in seconds)", ConfigType::SleepTime, sizeof(g_configParams._sleeptime), &g_configParams._sleeptime, ConfigurationParameters::SetUint16},
    {"LoraWAN - TX Power", ConfigType::LORA_TXPower, sizeof(g_configParams._loraTXPower), &g_configParams._loraTXPower, ConfigurationParameters::SetInt8},
    {"LoraWAN - DataRate", ConfigType::LORA_DataRate, sizeof(g_configParams._loraDataRate), &g_configParams._loraDataRate, ConfigurationParameters::SetInt8},
    {"LoraWAN - ADR Enabled", ConfigType::LORA_ADREnabled, sizeof(g_configParams._loraADREnabled), &g_configParams._loraADREnabled, ConfigurationParameters::SetBool},
    {"LoraWAN - Require confirmation message", ConfigType::LORA_RequireConfirmation, sizeof(g_configParams._loraRequireConfirmation), &g_configParams._loraRequireConfirmation, ConfigurationParameters::SetBool},
};

void ConfigurationParameters::SetUint32(const ConfigOption *option, uint8_t *arr)
{
    uint32_t val = 0;
    memcpy(&val, arr, option->sizeOfOption);
    uint32_t *ptr = (uint32_t *)option->value;
    *ptr = __builtin_bswap32(val);
    SERIAL_LOG("Setting '%s' change to %u", option->name, *ptr);
}

void ConfigurationParameters::SetUint16(const ConfigOption *option, uint8_t *arr)
{
    uint16_t val = 0;
    memcpy(&val, arr, option->sizeOfOption);
    uint16_t *ptr = (uint16_t *)option->value;
    *ptr = __builtin_bswap16(val);
    SERIAL_LOG("Setting '%s' change to %u", option->name, *ptr);
}

void ConfigurationParameters::SetUint8(const ConfigOption *option, uint8_t *arr)
{
    uint8_t val = 0;
    memcpy(&val, arr, option->sizeOfOption);
    uint8_t *ptr = (uint8_t *)option->value;
    *ptr = val;
    SERIAL_LOG("Setting '%s' change to %u", option->name, *ptr);
}

void ConfigurationParameters::SetInt8(const ConfigOption *option, uint8_t *arr)
{
    int8_t val = 0;
    memcpy(&val, arr, option->sizeOfOption);
    int8_t *ptr = (int8_t *)option->value;
    *ptr = val;
    SERIAL_LOG("Setting '%s' change to %d", option->name, *ptr);
}

void ConfigurationParameters::SetBool(const ConfigOption *option, uint8_t *arr)
{
    bool *ptr = (bool *)option->value;
    *ptr = false;
    // for boolean, we just expect a value > 0 to be true.
    if (arr[0] > 0)
    {
        *ptr = true;
    }
    SERIAL_LOG("Setting '%s' change to %d", option->name, *ptr);
}

void ConfigurationParameters::SetConfig(uint8_t *arr, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        for (size_t x = 0; x < sizeof(g_configs) / sizeof(ConfigOption); x++)
        {
            const ConfigOption *conf = &g_configs[x];
            if (conf->configType == arr[i])
            {
                conf->setfunc(conf, (arr + i + 1));
                i += conf->sizeOfOption;
                break;
            }
        }
    }
}

#endif