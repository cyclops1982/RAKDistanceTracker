#include <Arduino.h>
#include "lorahelper.h"
#include "ledhelper.h"
#include "serialhelper.h"
#include "main.h"
#include "batteryhelper.h"
#include "config.h"

#include <stdio.h>

#include "mbed.h"
#include "rtos.h"
#include "Wire.h"

#include <vl53l0x_class.h> // Click to install library: http://librarymanager/All#stm32duino_vl53l0x
VL53L0X sensor_vl53l0x(&Wire, WB_IO2);

uint16_t g_msgcount = 0;

EventType g_EventType = EventType::None;
uint8_t g_rcvdLoRaData[LORAWAN_BUFFER_SIZE];
uint8_t g_rcvdDataLen = 0;

void setup()
{

  delay(1000); // For whatever reason, some pins/things are not available at startup right away. So we wait 3 seconds for stuff to warm up or something
  LedHelper::init();
  // Initialize serial for output.
#if !MAX_SAVE
  time_t timeout = millis();
  Serial.begin(115200);
  // check if serial has become available and if not, just wait for it.
  while (!Serial)
  {
    if ((millis() - timeout) < 5000)
    {
      delay(100);
    }
    else
    {
      break;
    }
  }
#endif
  SERIAL_LOG("Setup start.");
  delay(1000);

  // Turn on power to sensors
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  delay(100);

  pinMode(ECHO, INPUT);  // Echo Pin of Ultrasonic Sensor is an Input
  pinMode(TRIG, OUTPUT); // Trigger Pin of Ultrasonic Sensor is an Output
  digitalWrite(TRIG, LOW);

  // Setup/start the wire that we use for the Sensor.
  Wire.begin();

  // Lora stuff
  // LoraHelper::InitAndJoin(g_configParams.GetLoraDataRate(), g_configParams.GetLoraTXPower(), g_configParams.GetLoraADREnabled(),
  //                        g_configParams.GetLoraDevEUI(), g_configParams.GetLoraNodeAppEUI(), g_configParams.GetLoraAppKey());
  sensor_vl53l0x.begin();

  // Switch off VL53L0X component.
  sensor_vl53l0x.VL53L0X_Off();

  // Initialize VL53L0X component.
  int status = sensor_vl53l0x.InitSensor(0x52);
  if (status)
  {
    SERIAL_LOG("Failed to init vl53l0x: %d", status);
  }
}

void handleReceivedMessage()
{

  g_configParams.SetConfig(g_rcvdLoRaData, g_rcvdDataLen);

  // Some parameters require some re-initialization, which is what we do here for those cases.
  for (uint8_t i = 0; i < g_rcvdDataLen; i++)
  {
    for (size_t x = 0; x < sizeof(g_configs) / sizeof(ConfigOption); x++)
    {
      const ConfigOption *conf = &g_configs[x];
      if (conf->configType == g_rcvdLoRaData[i])
      {
        switch (g_rcvdLoRaData[i])
        {
        case ConfigType::SleepTime:
          SERIAL_LOG("Resetting sleeptimer to %u", g_configParams.GetSleepTimeInSeconds());
          // TODO: update this

          break;
        case ConfigType::LORA_ADREnabled:
        case ConfigType::LORA_DataRate:
          SERIAL_LOG("Setting Lora DataRate to %u and ARD to %d", g_configParams.GetLoraDataRate(), g_configParams.GetLoraADREnabled());
          LoraHelper::SetDataRate(g_configParams.GetLoraDataRate(), g_configParams.GetLoraADREnabled());
          break;
        case ConfigType::LORA_TXPower:
          SERIAL_LOG("Setting Lora TX Power to %d", g_configParams.GetLoraTXPower());
          LoraHelper::SetTXPower(g_configParams.GetLoraTXPower());
          break;
        }
        i += conf->sizeOfOption; // jump to the next one
        break;
      }
    }
  }
}

// read RAK12014
uint32_t readTOF()
{
  uint32_t distance;
  int status = sensor_vl53l0x.GetDistance(&distance);

  if (status != VL53L0X_ERROR_NONE)
  {
    SERIAL_LOG("Failed to get distance: %d", status);
    return -1;
  }
  return distance;
}

// read RAK12007
long int duration_time()
{
  long int respondTime;
  pinMode(TRIG, OUTPUT);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(12); // pull high time need over 10us
  digitalWrite(TRIG, LOW);
  pinMode(ECHO, INPUT);
  respondTime = pulseIn(ECHO, HIGH); // microseconds
  delay(33);

  Serial.printf("respond time is %d\r\n", respondTime);

  // if((respondTime>0)&&(respondTime < TIME_OUT))  //ECHO pin max timeout is 33000us according it's datasheet
  //{
  return respondTime;
  //}
  // else
  //{
  //  return -1;
  //}
}

void doUpdateMessage()
{
  SERIAL_LOG("Doing updateMessage");

  uint16_t vbat_mv = BatteryHelper::readVBAT();
  long ultrasoundduration = duration_time();
  uint32_t tofDistance = readTOF();

  SERIAL_LOG("Bat value:           %d", vbat_mv);
  SERIAL_LOG("Ultrasound duration: %d", ultrasoundduration);
  SERIAL_LOG("Ultrasound distance: %f", ultrasoundduration * (346.6/1000/2));
  SERIAL_LOG("TOF Distance:        %lu", tofDistance)
  SERIAL_LOG("=============================");

  // Create the lora message
  /*  memset(g_SendLoraData.buffer, 0, LORAWAN_BUFFER_SIZE);
    int size = 0;
    g_SendLoraData.port = 2;
    g_SendLoraData.buffer[size++] = 0x03;
    g_SendLoraData.buffer[size++] = 0x02;

    g_SendLoraData.buffer[size++] = vbat_mv >> 8;
    g_SendLoraData.buffer[size++] = vbat_mv;

    g_SendLoraData.buffer[size++] = g_msgcount >> 8;
    g_SendLoraData.buffer[size++] = g_msgcount;

    // TODO

    g_SendLoraData.buffsize = size;

    lmh_error_status loraSendState = LMH_ERROR;
    loraSendState = lmh_send(&g_SendLoraData, (lmh_confirm)g_configParams.GetLoraRequireConfirmation());
  #if !MAX_SAVE
    if (loraSendState == LMH_SUCCESS)
    {
      Serial.println("lmh_send ok");
    }
    else
    {
      Serial.printf("lmh_send failed: %d\n", loraSendState);
    }
  #endif
  */
  g_msgcount++;
}

void loop()
{

  doUpdateMessage();

  delay(1500);
}
