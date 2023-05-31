/*
Sends GPS information to an esp32 board via LoRa.
*/

#include <Wire.h>  
#include "HT_SSD1306Wire.h"
#include "GPS_Air530.h"

#include "LoRaWan_APP.h"
#include "Arduino.h"

/*
 * set LoraWan_RGB to 1,the RGB active in loraWan
 * RGB red means sending;
 * RGB green means received done;
 */
#ifndef LoraWan_RGB
#define LoraWan_RGB 0
#endif

#define RF_FREQUENCY                                433000000  // Hz

#define TX_OUTPUT_POWER                             14        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       8         // [SF7..SF12]
#define LORA_CODINGRATE                             4         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30        // Payload size


char txpacket[BUFFER_SIZE];       // Data to transmit
char rxpacket[BUFFER_SIZE];       // Data to receive (unused here)
static RadioEvents_t RadioEvents;
int txNumber = 0;
bool lora_idle = true;
Air530Class GPS;


void VextON(void);
void VextOFF(void);
int fracPart(double val, int n);
void updateGPS();
void preparePacket();
void sendPacket();

void setup() 
{
  VextON();
  Serial.begin(115200);

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
  GPS.begin();
  Serial.println("Starting GPS Tracker Sender");
}


void loop()
{
  if (lora_idle == true)
  {
    updateGPS();
    if (GPS.location.isValid())
    {
      preparePacket();
      sendPacket();
    }
  }
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
}


void preparePacket()
{
  /* Write the altitude with precision of two decimal points to the transmitted buffer */
  int altInt = (int)GPS.altitude.meters();
  int altFrac = fracPart(GPS.altitude.meters(), 2);
  int writtenAmount = sprintf( txpacket, "%d.%d", altInt, altFrac );
  txpacket[writtenAmount] = '\0';
}


void sendPacket()
{
  /* Increment packets transmitted */
  txNumber++;

  /* Turn colour to let user know data is sent */
  turnOnRGB(COLOR_SEND,0);
  Radio.Send( (uint8_t*)txpacket, strlen(txpacket) );
  lora_idle = false;
}


void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}


void VextOFF(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}

void OnTxDone( void )
{
  turnOffRGB();
  Serial.println("TX done......");
  lora_idle = true;
  delay(10000);
}


void OnTxTimeout( void )
{
  turnOffRGB();
  Radio.Sleep( );
  Serial.println("TX Timeout......");
  lora_idle = true;
}


int fracPart(double val, int n)
{
  return (int)((val - (int)(val))*pow(10,n));
}


/*
* @brief Manages the entry into ARM cortex deep-sleep mode
 * @param none
 * @retval none
 void lowPowerHandler( void )
*/

