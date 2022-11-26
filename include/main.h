#ifndef MAIN_H
#define MAIN_H
#include <Arduino.h>
#include <SPI.h>


#define RAK11310
 
#define TRIG WB_IO6
#define ECHO WB_IO4
//#define PD   WB_IO5   //power done control （=1 power done，=0 power on） 

enum EventType {
  None = -1,
  Timer = 1,
  LoraDataReceived = 2
};


extern EventType g_EventType;
extern uint8_t g_rcvdLoRaData[];
extern uint8_t g_rcvdDataLen;

#endif