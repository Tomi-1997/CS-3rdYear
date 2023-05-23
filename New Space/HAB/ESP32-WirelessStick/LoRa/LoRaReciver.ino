#include "Arduino.h"
#include "heltec.h"
#include <time.h>

/* OLED Display constants */
const int start = 0;
const int end_x = 60;
const int end_y = 20;
const int wait_time = 2000;
const int max_width = end_x;


const long frequency = 915E6;  // LoRa Frequency


void drawMessage(int x, int y, char* m) {
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->drawStringMaxWidth(x, y, max_width, m);
  // write the buffer to the display
  Heltec.display->display();
}


void setup() {
  Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Enable*/, true /*Serial Enable*/);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->clear();
  LoRa.setFrequency(frequency);

  Serial.begin(9600);
  while (!Serial)
    ;
}


void loop() {
  int packetSize = LoRa.parsePacket();

  char buffer[1024];
  int i = 0;
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      // drawMessage(start, start, (char*)LoRa.read());
      buffer[i] = (char)LoRa.read();
      i++;
      // Serial.print((char)LoRa.read());
      // Serial.println();
    }
    buffer[i] = '\0';
    drawMessage(start, start, buffer);
    // print RSSI of packet
    // Serial.print("' with RSSI ");
    // Serial.println(LoRa.packetRssi());
    delay(wait_time);
    Heltec.display->clear();
  }
}
