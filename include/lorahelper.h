#ifndef LORAHELPER_H
#define LORAHELPER_H
#include <Arduino.h>
#include <LoRaWan-Arduino.h>
#include "batteryhelper.h"
#include "serialhelper.h"

//#define SCHED_MAX_EVENT_DATA_SIZE APP_TIMER_SCHED_EVENT_DATA_SIZE /**< Maximum size of scheduler events. */
//#define SCHED_QUEUE_SIZE 60                     /**< Maximum number of events in the scheduler queue. */
#define JOINREQ_NBTRIALS 3
#define LORAWAN_APP_PORT 2
#define LORAWAN_BUFFER_SIZE 64 /**< buffer size of the data to be transmitted. */
#define LORAWAN_CLASS CLASS_A

static uint8_t g_sendLoraDataBuffer[LORAWAN_BUFFER_SIZE];                  //< Lora user application data buffer.
static lmh_app_data_t g_SendLoraData = {g_sendLoraDataBuffer, 0, 0, 0, 0}; //< Lora user application data structure.
static bool lorawan_joined;


struct LoraHelper
{
    static void lorawan_has_joined_handler(void);
    static void lorawan_join_failed_handler(void);
    static void lorawan_rx_handler(lmh_app_data_t *app_data);
    static void lorawan_confirm_class_handler(DeviceClass_t Class);
    static void lorawan_unconf_finished(void);
    static void lorawan_conf_finished(bool result);
    static void InitAndJoin(int8_t datarate, int8_t TXPower, bool adrEnabled, uint8_t* nodeDeviceEUI, uint8_t* nodeAppEUI, uint8_t* nodeAppKey);
    static void SetDataRate(int8_t datarate, bool adrEnabled);
    static void SetTXPower(int8_t TXPower);
};

#endif