/*
Draws several lines on the OLED screen.
Scrolls down if there are too many lines.
*/
#include <Wire.h>  
#include "HT_SSD1306Wire.h"

#define charsPerLine 30
#define linesPerScreen 5

SSD1306Wire display(0x3c, 500000, SDA,
                    SCL, GEOMETRY_128_64, GPIO10); // addr , freq , SDA, SCL, resolution , rst

void VextON(void);
void VextOFF(void);
int wordsCount(char** sentence);
void printBuffer(char* text);
void printBuffer(char** text);

void setup() 
{
  VextON();
  delay(100);

  display.init();
  display.clear();
  display.display();
}


void loop()
{
  char* test1[] = {"this is the", "first sentence", "that I am", "currently testing", "allowing",
  "multiple lines", "per text", NULL};
  printBuffer(test1);
  delay(2000);


  char* test2[] = {"this is the", "second sentence", "that I am", "currently testing", "allowing",
  "multiple lines", "per text", NULL};
  printBuffer(test2);
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
  char* buff[] = {text};
  printBuffer(buff);
}


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
