#include <Wire.h>  
#include "HT_SSD1306Wire.h"
#include "GPS_Air530.h"

#include "LoRaWan_APP.h"
#include "Arduino.h"

#define charsPerLine 30
#define linesPerScreen 5

/*
 * set LoraWan_RGB to 1,the RGB active in loraWan
 * RGB red means sending;
 * RGB green means received done;
 */
#ifndef LoraWan_RGB
#define LoraWan_RGB 0
#endif

#define RF_FREQUENCY                                915000000 // Hz

#define TX_OUTPUT_POWER                             14        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       10         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here


char txpacket[BUFFER_SIZE];       // Data to transmit
char rxpacket[BUFFER_SIZE];
static RadioEvents_t RadioEvents;
int txNumber;
bool lora_idle=true;

Air530Class GPS;
extern SSD1306Wire display;

void VextON(void);
void VextOFF(void);
int sens_len(char** sentences);
void printBuffer(char* text);
void printBuffer(char** text);


void setup() 
{
  VextON();
  Serial.begin(115200);
  display.init();
  display.clear();
  display.display();
  txNumber=0;

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                  LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                  LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                  true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 
  Radio.SetSyncWord(0x12);

  display.drawString(0, 0, "Starting LoRa!");
  display.display();
  GPS.begin();
}

void loop()
{
  if(lora_idle == true)
  {

    /*Get GPS data*/
    uint32_t starttime = millis();
    while( (millis() - starttime) < 1000 )
    {
      while (GPS.available() > 0)
      {
        GPS.encode(GPS.read());
      }
    }

    /* GPS Data --> text to be sent*/
    txNumber++;
    int prefixLen = sizeof(int) + strlen("Sending packet #");
    char iterNum[prefixLen];
    char* prefix = "Sending packet #";
    sprintf(iterNum, "%s%d", prefix, txNumber);

    sprintf(txpacket,"%d) Alt - %0.2f", txNumber, GPS.altitude.meters());
    Serial.printf("\r\nsending packet \"%s\" , length %d\r\n", txpacket, strlen(txpacket));


    /* Display data to be sent + iter num */
    display.clear();
    display.drawString(0, 0, iterNum);
    display.drawString(0, 10, txpacket);
    display.display();

    /* Turn colour to let user know data is sent */
    turnOnRGB(COLOR_SEND,0); //change rgb color
    Radio.Send( (uint8_t*)txpacket, strlen(txpacket) ); //send the package out 
    lora_idle = false;

    delay(2000);

  }
}

void VextON(void) // OLED ON
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}


void VextOFF(void) // OLED OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}


int sens_len(char** sentences)
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
  for (uint8_t i = 0; i < sens_len(text); i++) 
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


void OnTxDone( void )
{
  turnOffRGB();
  Serial.println("TX done......");
  lora_idle = true;
}


void OnTxTimeout( void )
{
  turnOffRGB();
  Radio.Sleep( );
  Serial.println("TX Timeout......");
  lora_idle = true;
}
