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

#define RF_FREQUENCY                                437250000  // Hz

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
#define LORA_SYNCWORD                               0x2A
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30        // Payload size

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];
char Repacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );
void OnRxTimeout(void);
void displaySendReceive();

typedef enum
{
    STATUS_LOWPOWER,
    STATUS_RX,
    STATUS_TX
}States_t;


int16_t txNumber;
States_t state;
bool sleepMode = false;
int16_t Rssi,rxSize;

// Add your initialization code here
void setup()
{  
  Serial.begin(115200);
  while (!Serial);


  txNumber=0;
  Rssi=0;

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.RxTimeout = OnRxTimeout;

  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                  LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                  LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                  true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

  Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                  LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                  LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                  0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
  
  Radio.SetSyncWord(LORA_SYNCWORD);
  state=STATUS_TX;
}

void loop()
{
  switch(state)
  {
    case STATUS_TX:
      delay(2000);
      txNumber++;
      sprintf(txpacket,"%s","hello");
      sprintf(txpacket+strlen(txpacket),"%d",txNumber);
      sprintf(txpacket+strlen(txpacket),"%s"," Rssi : ");
      sprintf(txpacket+strlen(txpacket),"%d",Rssi);

      Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));

      LoRaWAN.displaySending();
      Radio.Send( (uint8_t *)txpacket, strlen(txpacket) );
      state=STATUS_LOWPOWER;
      break;
    case STATUS_RX:
        Serial.println("into RX mode");
        Radio.Rx( 10000 );
        state=STATUS_LOWPOWER;
        break;
    case STATUS_LOWPOWER:
        LoRaWAN.sleep();
        break;
    default:
        break;
  }
}

void OnTxDone( void )
{
  Serial.print("TX done......");
  sprintf(Repacket,"%s","");
  displaySendReceive();
  state=STATUS_RX;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.print("TX Timeout......");
    sprintf(Repacket,"%s","TX Timeout,Retransmission");
    displaySendReceive();
    state=STATUS_TX;
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Rssi=rssi;
    rxSize=size;
    memcpy(rxpacket, payload, size );
    rxpacket[size]='\0';
    Radio.Sleep( );

    Serial.printf("\r\nreceived packet \"%s\" with Rssi %d , length %d\r\n",rxpacket,Rssi,rxSize);
    Serial.println("wait to send next packet");
    sprintf(Repacket,"%s","");
    displaySendReceive();

    state=STATUS_TX;
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    Serial.print("RX Timeout......");
    sprintf(Repacket,"%s","RX Timeout,Retransmission");
    displaySendReceive();
    state=STATUS_TX;
}

void displaySendReceive()
{
  puts(txpacket);
  puts(Repacket);
  puts(rxpacket);
}
