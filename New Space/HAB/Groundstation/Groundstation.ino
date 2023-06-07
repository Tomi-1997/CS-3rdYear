/*
  ESP32 - Wireless stick program to act as a ground station to the CubeCell GPS payload.
  To receive data from payload through LoRa and write to a text file, make sure to: 
  - Validate that the LoRa parameters are the same as the payload.
  - Run this program without opening the serial monitor, then promptly run textify.py
*/

#include "heltec.h" 
#define BAND    433E6

void setup() 
{
  Heltec.begin(false /* Display */,
  true /* LoRa */,
  true /*Serial Enable*/, 
  true /* PABOOST */,
  BAND /* LORA FREQ*/);

  Serial.begin(115200);
  Serial.println("Serial init.");

  LoRa.setSpreadingFactor(8);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(4);
  LoRa.setSyncWord(0x12);
  LoRa.setPreambleLength(8);
  LoRa.receive();
  Serial.println("LoRa init.");
  
  delay(1000);
}

void loop() 
{
  /* Try to parse packet */
  int packetSize = LoRa.parsePacket();
  if (packetSize > 0) 
  {
    /* Size > 0 ---> Got packet*/

    /* Print to serial*/
    while (LoRa.available()) 
    {
      char c = (char)LoRa.read();
      Serial.print(c);
    }
    Serial.print("\n");
  }
  else
  {
    delay(1000);
  }
}
