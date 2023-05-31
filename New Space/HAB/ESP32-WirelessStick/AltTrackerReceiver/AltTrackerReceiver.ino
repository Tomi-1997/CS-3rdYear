/*
Constantly listens to altitude updates and prints them to the monitor.
*/

#include "heltec.h" 
#define BAND    433E6


void init();

void setup() 
{
  init();
}

void loop() 
{  
  /* Try to parse packet */
  int packetSize = LoRa.parsePacket();
  if (packetSize > 0) 
  {
    /* Size > 0 ---> Got packet*/

    /* Print to monitor*/
    while (LoRa.available()) 
    {
      char c = (char)LoRa.read();
      Serial.print(c);
    }
    Serial.print("\n");
  }
}


void init()
{
  Heltec.begin(false /* Display */,
  true /* LoRa */,
  true /*Serial Enable*/, 
  true /* PABOOST */,
  BAND /* LORA FREQ*/);

  Serial.begin(115200);
  LoRa.setSpreadingFactor(8);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(4);
  LoRa.setSyncWord(0x12);
  LoRa.setPreambleLength(8);
  LoRa.receive();
}