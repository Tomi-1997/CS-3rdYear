/*
  ESP32-Wireless Stick program to receive LoRaWan packet from a CubCell unit.


  The onboard OLED display is SSD1306 driver and I2C interface. In order to make the
  OLED correctly operation, you should output a high-low-high(1-0-1) signal by soft-
  ware to OLED's reset pin, the low-level signal at least 5ms.

  OLED pins to ESP32 GPIOs via this connecthin:
  OLED_SDA -- GPIO4
  OLED_SCL -- GPIO15
  OLED_RST -- GPIO16
  
  Based on code by Aaron.Lee from HelTec AutoMation, ChengDu, China
  成都惠利特自动化科技有限公司
  www.heltec.cn
*/
#include "heltec.h" 


#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6
String rssi = "RSSI --";
String packSize = "--";
String packet ;

int packetsGot = 0;
void LoRaData();
void cbk(int packetSize) ;


void setup() 
{
  Heltec.begin(true /* Display */,
  true /* LoRa */,
  true /*Serial Enable*/, true /* PABOOST */,
  BAND /* BAND*/);
  Serial.begin(9600);

  /* Define settings same as the CubeCell unit */
  LoRa.setSpreadingFactor(10);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(4);
  LoRa.setSyncWord(0x12);
  LoRa.setPreambleLength(8);
  LoRa.setFrequency(BAND);
  LoRa.setTxPowerMax(14);
  
  Heltec.display->init();
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->clear();
  
  Heltec.display->drawString(0, 0, "LoRa OK");
  Heltec.display->display();
  LoRa.receive();
  Heltec.display->setFont(ArialMT_Plain_10);

}

void loop() 
{
  int packetSize = LoRa.parsePacket();
  if (packetSize) 
  { 
    packetsGot++;
    Heltec.display->drawString(0, 0, (String) packetsGot + ":");
    Heltec.display->drawString(0, 10, "Packet:");
    Serial.println("Got packet");
    cbk(packetSize);
  }
  delay(1000);
}


void LoRaData()
{
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawStringMaxWidth(0 , 0 , 60, packet);
  Heltec.display->display();
}


void cbk(int packetSize) 
{
  packet ="";
  packSize = String(packetSize, DEC);
  for (int i = 0; i < packetSize; i++) { packet += (char) LoRa.read(); }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
  LoRaData();
}
