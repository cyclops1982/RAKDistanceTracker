#include <Arduino.h>
#include <stdio.h>
#include <mbed.h>
#include <rtos.h>
#include <Wire.h>
#include <vl53l0x_class.h> 
#include "lorahelper.h"
#include "ledhelper.h"
#include "serialhelper.h"
#include "main.h"
#include "batteryhelper.h"
#include "config.h"

VL53L0X sensor_vl53l0x(&Wire, WB_IO2);
uint16_t g_msgcount = 0;
EventType g_EventType = EventType::None;
uint8_t g_rcvdLoRaData[LORAWAN_BUFFER_SIZE];
uint8_t g_rcvdDataLen = 0;

mbed::Ticker appTimer;

void timerInterrupt()
{
  appTimer.attach(timerInterrupt, (std::chrono::microseconds)(g_configParams.GetSleepTimeInSeconds() * 1000 * 1000));
  g_EventType = EventType::Timer;
}

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

  // setup pins for the ulrasound
  pinMode(ECHO, INPUT);  // Echo Pin of Ultrasonic Sensor is an Input
  pinMode(TRIG, OUTPUT); // Trigger Pin of Ultrasonic Sensor is an Output
  digitalWrite(TRIG, LOW);

  // Setup/start the wire that we use for the Sensor.
  Wire.begin();

  // Lora stuff
  LoraHelper::InitAndJoin(g_configParams.GetLoraDataRate(), g_configParams.GetLoraTXPower(), g_configParams.GetLoraADREnabled(),
                          g_configParams.GetLoraDevEUI(), g_configParams.GetLoraNodeAppEUI(), g_configParams.GetLoraAppKey());

  // tof Sensor
  sensor_vl53l0x.begin();
  sensor_vl53l0x.VL53L0X_Off();
  int status = sensor_vl53l0x.InitSensor(0x52);
  if (status)
  {
    SERIAL_LOG("Failed to init vl53l0x: %d", status);
  }

  SERIAL_LOG("Starting timerInterrupt to get started.")
  timerInterrupt();
}


bool SendData()
{
  if (!lorawan_joined)
  {
    SERIAL_LOG("SendData called, but we're not joined to the network. Joining now...");
    // Lora stuff
    LoraHelper::InitAndJoin(g_configParams.GetLoraDataRate(), g_configParams.GetLoraTXPower(), g_configParams.GetLoraADREnabled(),
                            g_configParams.GetLoraDevEUI(), g_configParams.GetLoraNodeAppEUI(), g_configParams.GetLoraAppKey());
  }
  if (lorawan_joined)
  {
    lmh_error_status loraSendState = lmh_send(&g_SendLoraData, (lmh_confirm)g_configParams.GetLoraRequireConfirmation());

    if (loraSendState == LMH_SUCCESS)
    {
      SERIAL_LOG("LoRaSend ok");
      return true;
    }
    else
    {
      SERIAL_LOG("LorRaSend failed: %d\n", loraSendState);
      return false;
    }
  }
  else
  {
    SERIAL_LOG("SKIPPING SEND - We are not joined to a network");
  }
  return false;
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
uint16_t readTOF()
{
  uint32_t distance;
  int status = sensor_vl53l0x.GetDistance(&distance);

  if (status != VL53L0X_ERROR_NONE)
  {
    SERIAL_LOG("Failed to get distance: %d", status);
    return 0;
  }
  return (uint16_t)distance;
}

// read RAK12007
long int readUltrasound()
{
  pinMode(TRIG, OUTPUT);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(12); // pull high time need over 10us
  digitalWrite(TRIG, LOW);
  pinMode(ECHO, INPUT);
  long int respondTime = pulseIn(ECHO, HIGH); // microseconds
  delay(33);
  return lround(respondTime * (343.0/1000/2));
}

void doUpdateMessage()
{

  SERIAL_LOG("Doing updateMessage");

  uint16_t vbat_mv = BatteryHelper::readVBAT();
  long int ultrasoundDistance = readUltrasound();
  uint16_t tofDistance = readTOF();

  SERIAL_LOG("Bat value:           %lu", vbat_mv);
  SERIAL_LOG("Message Count:       %lu", g_msgcount);
  SERIAL_LOG("TOF Distance:        %lu", tofDistance)
  SERIAL_LOG("Ultrasound distance: %lu", ultrasoundDistance);
  SERIAL_LOG("=============================");

  // Create the lora message
  memset(g_SendLoraData.buffer, 0, LORAWAN_BUFFER_SIZE);
  int size = 0;
  g_SendLoraData.port = 2;
  g_SendLoraData.buffer[size++] = 0x03;
  g_SendLoraData.buffer[size++] = 0x01;

  g_SendLoraData.buffer[size++] = vbat_mv >> 8;
  g_SendLoraData.buffer[size++] = vbat_mv;

  g_SendLoraData.buffer[size++] = g_msgcount >> 8;
  g_SendLoraData.buffer[size++] = g_msgcount;

  g_SendLoraData.buffer[size++] = tofDistance >> 8;
  g_SendLoraData.buffer[size++] = tofDistance;

  g_SendLoraData.buffer[size++] = ultrasoundDistance >> 8;
  g_SendLoraData.buffer[size++] = ultrasoundDistance;

  g_SendLoraData.buffsize = size;


#if !MAX_SAVE
  SERIAL_LOG("Sending LORA data:");
    for (uint8_t i = 0; i < g_SendLoraData.buffsize; i++)
  {
    char hexstr[3];
    sprintf(hexstr, "%02x", g_SendLoraData.buffer[i]);
    SERIAL_LOG("DATA %d: %s", i, hexstr)
  }
#endif
  SendData();


  g_msgcount++;
}

void loop()
{
  switch (g_EventType) {
    case EventType::Timer:
      doUpdateMessage();
      g_EventType = EventType::None;
    break;
    case EventType::LoraDataReceived:
      handleReceivedMessage();
      g_EventType = EventType::None;
    break;
    case EventType::None:
    default:
      SERIAL_LOG("Nothing to do");
      delay(10000);
    break;
  }
  

}
