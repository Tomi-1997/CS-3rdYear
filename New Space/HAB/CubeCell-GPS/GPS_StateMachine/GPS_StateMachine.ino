/* 
  Program to display current altitude and state on the display.

  For that, log past altitudes for a fixed amount. 
  If the current altitude is the maximal recent altitude, changes state to rising.
  If the current altitude is the minimal recent altitude, changes state to descending.
  Else- switches state to neutral.
*/

#include <Wire.h>  
#include "HT_SSD1306Wire.h"
#include "GPS_Air530.h"


#define charsPerLine 30     /*  */
#define linesPerScreen 5    /* Max lines displayed */
#define ALT_SIZE 16         /* Previous altitudes to consider */
#define GPS_INIT_WINDOW 32  /* Iterations to log gps data, without changing states */


Air530Class GPS;
SSD1306Wire display(0x3c, 500000, SDA,
                    SCL, GEOMETRY_128_64, GPIO10); // addr , freq , SDA, SCL, resolution , rst


void VextON(void);
void VextOFF(void);
int wordsCount(char** sentence);
void printBuffer(char* text);
void printBuffer(char** text);
int fracPart(double val, int n);


void updateGPS();
void updateState();
void PrintState();
int average(int arr[], int size);
int my_max(int arr[], int size);
int my_min(int arr[], int size);


enum States          { Init,   Neutral,   Rising,   Descending};
char* States_str[] = {"Init", "Neutral", "Rising", "Descending"};
int ALTS[ALT_SIZE];  /* Log previous altitudes */
int STATE;           /*  */
int INIT_GPS = 1;    /* For the first X iterations, just log gps */
int ITER = 0;       /* Iteration number, for init and debugging */
int INDEX = 0;      /* Current index for the altitude array */


void setup() 
{
  STATE = Init;
  VextON();
  delay(100);
  display.init();
  display.clear();
  display.display();
  display.setFont(ArialMT_Plain_16);
  GPS.begin();
}

void loop()
{
  if (INIT_GPS == 1 && ITER >= GPS_INIT_WINDOW)
  {
    INIT_GPS = 0;
  }

  updateGPS();
  updateState();
  printState();
  ITER++;
  INDEX = (INDEX + 1) % ALT_SIZE;
}


void printState()
{
  char alt_str[32]; /*  */
  int writtenAmount = sprintf(alt_str, "Alt: %d, #:%d", ALTS[INDEX], ITER);
  alt_str[writtenAmount] = '\0';
  char* buff[] = {"---", States_str[STATE], alt_str, "---", NULL};
  printBuffer(buff);
}


void updateGPS()
{
  uint32_t starttime = millis();
  while( (millis() - starttime) < 1000 )
  {
    while (GPS.available() > 0)
    {
      GPS.encode(GPS.read());
    }
  }

  if (!GPS.altitude.isValid())
  {
    return;
  }

  int curr_alt = (int) GPS.altitude.meters();
  ALTS[INDEX] = curr_alt;
}


void updateState()
{
  if (INIT_GPS == 1)
    return;

  int curr_alt = ALTS[INDEX];
  if (curr_alt > my_max(ALTS, ALT_SIZE))
    STATE = Rising;
  
  else if (curr_alt < my_min(ALTS, ALT_SIZE))
    STATE = Descending;

  else
    STATE = Neutral;  
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


int wordsCount(char** sentence)
{
  int ans = 0;
  char** temp = sentence;

  while(*temp != NULL)
  {
    temp++;
    ans++;
  }

  return ans;
}

void printBuffer(char* text) 
{
  char* buff[] = {"---", text, "---", NULL};
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
    delay(100);
  }
}


int fracPart(double val, int n)
{
  return (int)((val - (int)(val))*pow(10,n));
}


int my_max(int arr[], int size)
{
  if (size == 0)
    return -1;
  int ans = arr[1];
  for (int i = 1; i < size; i++)
  {
    if (arr[i] > ans)
      ans = arr[i];
  }
  return ans;
}


int my_min(int arr[], int size)
{
  if (size == 0)
    return -1;
  int ans = arr[1];
  for (int i = 1; i < size; i++)
  {
    if (arr[i] < ans)
      ans = arr[i];
  }
  return ans;
}


int average(int arr[], int size)
{
  if (size == 0)
    return -1;
  int ans = 0;
  for (int i = 0; i < size; i++)
  {
    ans += arr[i];
  }
  ans = (int) (ans / size);
  return ans;
}