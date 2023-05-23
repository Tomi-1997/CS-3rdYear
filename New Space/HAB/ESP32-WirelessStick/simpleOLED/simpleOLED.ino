#include "Arduino.h"
#include "heltec.h"


void setup() 
{
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->clear();
}


int start = 0;
int end_x = 60;
int end_y = 20;
int wait_time = 1000;
int max_width = end_x;

void drawMessage(int x, int y, char* m) 
{
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->drawStringMaxWidth(x, y, max_width, m);
  // write the buffer to the display
  Heltec.display->display();
}


void loop() 
{
  drawMessage(start, start, "abcdefghijklmnopqrstuvwxyz");
  delay(wait_time);
  Heltec.display->clear();

  drawMessage(end_x, start, "B");
  delay(wait_time);
  Heltec.display->clear();

  drawMessage(start, end_y, "C");
  delay(wait_time);
  Heltec.display->clear();

  drawMessage(end_x, end_y, "D");
  delay(wait_time);
  Heltec.display->clear();

}
