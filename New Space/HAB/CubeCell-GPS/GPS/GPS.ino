/*
Prints GPS information on the OLED screen every couple of seconds
*/

#include <Wire.h>  
#include "HT_SSD1306Wire.h"
#include "GPS_Air530.h"

#define charsPerLine 30
#define linesPerScreen 5

Air530Class GPS;
SSD1306Wire display(0x3c, 500000, SDA,
                    SCL, GEOMETRY_128_64, GPIO10); // addr , freq , SDA, SCL, resolution , rst


void VextON(void);
void VextOFF(void);
int wordsCount(char** sentence);
void printBuffer(char* text);
void printBuffer(char** text);
int fracPart(double val, int n);


void setup() 
{
  VextON();
  delay(100);

  display.init();
  display.clear();
  display.display();

  GPS.begin();
}


void loop()
{
  uint32_t starttime = millis();
  while( (millis() - starttime) < 1000 )
  {
    while (GPS.available() > 0)
    {
      GPS.encode(GPS.read());
    }
  }

  char str[7][charsPerLine];
  char* info[] = {"-----------", str[0], str[1], str[2], str[3], str[4], str[5], str[6], "-----------", NULL};

  int index = sprintf( str[0], "%02d-%02d-%02d",GPS.date.year(),GPS.date.day(),GPS.date.month() );
  str[0][index] = '\0';

  index = sprintf( str[1], "%02d:%02d:%02d",GPS.time.hour(),GPS.time.minute(),GPS.time.second(),GPS.time.centisecond() );
  str[1][index] = '\0';

  index = sprintf( str[2], "alt: %d.%d",(int)GPS.altitude.meters(),fracPart(GPS.altitude.meters(),2) );
  str[2][index] = '\0';

  index = sprintf( str[3], "hdop: %d.%d",(int)GPS.hdop.hdop(),fracPart(GPS.hdop.hdop(),2));
  str[3][index] = '\0';
 
  index = sprintf( str[4], "lat : %d.%d",(int)GPS.location.lat(),fracPart(GPS.location.lat(),4));
  str[4][index] = '\0';
  
  index = sprintf( str[5], "lon: %d.%d",(int)GPS.location.lng(),fracPart(GPS.location.lng(),4));
  str[5][index] = '\0';

  index = sprintf( str[6], "speed: %d.%d km/h",(int)GPS.speed.kmph(),fracPart(GPS.speed.kmph(),3));
  str[6][index] = '\0';

  printBuffer(info);
  delay(2000);
}


void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}


void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}


int wordsCount(char** sentences)
{
  int ans = 0;
  char** temp = sentences;

  while(*temp != NULL)
  {
    temp++;
    ans++;
  }

  return ans;
}


void printBuffer(char* text) 
{
  char* buff[] = {"---", text, "---"};
  printBuffer(buff);
}


// Print several lines of text, make sure the last element is NULL
void printBuffer(char** text) 
{
  display.setLogBuffer(linesPerScreen, charsPerLine);
  for (uint8_t i = 0; i < wordsCount(text); i++) 
  {
    display.clear();
    // Print to the screen
    display.println(text[i]);
    // Draw it to the internal screen buffer
    display.drawLogBuffer(0, 0);
    // Display it on the screen
    display.display();
    delay(1000);
  }
}


int fracPart(double val, int n)
{
  return (int)((val - (int)(val))*pow(10,n));
}


