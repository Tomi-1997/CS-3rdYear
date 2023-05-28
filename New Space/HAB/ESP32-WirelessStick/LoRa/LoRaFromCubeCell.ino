#include "heltec.h" 

#define BAND    433E6
void setup() 
{
  Heltec.begin(true /* Display */,
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

  Heltec.display->init();
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "LoRa OK");
  Heltec.display->display();
  LoRa.receive();
  Heltec.display->setFont(ArialMT_Plain_10);
}


int start = 0;
int end_x = 60;
int end_y = 20;
int wait_time = 1000;
int max_width = end_x;
void drawMessage(int x, int y, char* m) 
{
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  /* If message is too long (len > max_width), drops a line and continues*/
  Heltec.display->drawStringMaxWidth(x, y, max_width, m);
  Heltec.display->display();
}


char buff[256];
void loop() 
{
  int i = 0;
  
  /* Try to parse packet */
  int packetSize = LoRa.parsePacket();
  if (packetSize) 
  {
    /* Size > 0 ---> Got packet*/
    Serial.print("Received packet '");

    /* Read it, or copy to buffer or both*/
    while (LoRa.available()) 
    {
      char c = (char)LoRa.read();
      Serial.print(c);
      buff[i] = c;
      i++;
    }

    /* Get signal strength */
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
    
    sprintf((buff + i), "' with RSSI %d", LoRa.packetRssi());

    Heltec.display->clear();
    drawMessage(0, 0, buff);
  }
}
