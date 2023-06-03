/*
Sends message to Ariel GS and listens back.
*/

#include "heltec.h" 
#define FREQ    437250000
#define X 64
#define Y 32

void init();
void sendPacket();
void readPacket();
void blinkNtimes(int n);

void setup() 
{
  init();
}


void loop() 
{  
  sendPacket();
  readPacket();
  delay(2000);
}


void sendPacket()
{
  LoRa.beginPacket();
  LoRa.print("hey :)");
  LoRa.endPacket();
  Heltec.display->clear();
  Heltec.display->drawStringMaxWidth(0, 0, X, "SENT");
  Heltec.display->display();
}


void readPacket()
{

  Heltec.display->clear();
  int packetSize = LoRa.parsePacket();

  uint32_t starttime = millis();
  while( (millis() - starttime) < 1000 && packetSize == 0)
  {
    packetSize = LoRa.parsePacket();
    delay(50);
  };
  if (packetSize > 0) 
  {
    char buff[64];
    /* Size > 0 ---> Got packet*/

    /* Read to buffer */
    int i = 0;
    while (LoRa.available()) 
    {
      char c = (char)LoRa.read();
      buff[i] = c;
      if (i < 63)
        i++;
    }
    buff[i] = '\0';
    /* Display buffer on screen */
    Heltec.display->drawStringMaxWidth(0, 0, X, buff);
    Heltec.display->display();
    delay(5000);
  }
  else
  {
    Heltec.display->drawStringMaxWidth(0, 0, X, "X");
    Heltec.display->display();
  }
}


void init()
{
  Heltec.begin(true /* Display */,
  true /* LoRa */,
  true /*Serial Enable*/, 
  true /* PABOOST */,
  62.5E3 /* BAND*/);
  pinMode(LED_BUILTIN, OUTPUT);

  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->clear();

  LoRa.setFrequency(FREQ);
  Serial.begin(115200);
  LoRa.setSpreadingFactor(10);
  // LoRa.setSignalBandwidth(62.5E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(0x2A);
  LoRa.setPreambleLength(8);
  LoRa.receive();
  Serial.println("Starting Ariel GS");
}


void blinkNtimes(int n)
{
  for (int i = 0; i < n; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(200);                      // wait for a bit
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    delay(200);   
  }
    
}