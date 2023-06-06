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

#define RF_FREQUENCY 437250000  // Hz

#define TX_OUTPUT_POWER 14  // dBm

#define LORA_BANDWIDTH 3          // [0: 125 kHz, \
                                  //  1: 250 kHz, \
                                  //  2: 500 kHz, \
                                  //  3: Reserved]
#define LORA_SPREADING_FACTOR 10  // [SF7..SF12]
#define LORA_CODINGRATE 1         // [1: 4/5, \
                                  //  2: 4/6, \
                                  //  3: 4/7, \
                                  //  4: 4/8]
#define crcON false
#define LORA_SYNCWORD 0x2A
#define LORA_PREAMBLE_LENGTH 8  // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0   // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 30  // Payload

int SEC_1 = 1000;
char txpacket[BUFFER_SIZE];  // Data to transmit
char rxpacket[BUFFER_SIZE];  // Data to receive (unused here)
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

void setup() {
  VextON();
  Serial.begin(115200);

  txNumber = 0;

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    crcON, 0, 0, LORA_IQ_INVERSION_ON, 3000);
  Radio.SetSyncWord(LORA_SYNCWORD);
  // Radio.SetSymbTime(3,10);
  GPS.begin();
  Serial.println("Starting GPS Tracker Sender");
}


void loop() {
  if (lora_idle == true) {
    // updateGPS();
    smart_delay(5 * SEC_1);
    // if (GPS.location.isValid())
    // {
    preparePacket();
    sendPacket();
    // }
  }
}


void updateGPS() {
  uint32_t starttime = millis();
  while ((millis() - starttime) < 1000) {
    while (GPS.available() > 0) {
      GPS.encode(GPS.read());
    }
  }
}


void preparePacket() {
  /* Prepare largest packet of gps info */
  int index = 0;
  if (GPS.date.isValid())
    index += sprintf((txpacket + index), "%d/%02d/%02d\n", GPS.date.year(), GPS.date.day(), GPS.date.month());

  if (GPS.time.isValid())
    index += sprintf((txpacket + index), "%02d:%02d:%02d.%02d\n", GPS.time.hour(), GPS.time.minute(), GPS.time.second(), GPS.time.centisecond());

  index += sprintf((txpacket + index), "alt:%d.%d\n", (int)GPS.altitude.meters(), fracPart(GPS.altitude.meters(), 2));
  index += sprintf((txpacket + index), "hdop:%d.%d\n", (int)GPS.hdop.hdop(), fracPart(GPS.hdop.hdop(), 2));
  index += sprintf((txpacket + index), "lat :%d.%d\n", (int)GPS.location.lat(), fracPart(GPS.location.lat(), 4));
  index += sprintf((txpacket + index), "lon:%d.%d\n", (int)GPS.location.lng(), fracPart(GPS.location.lng(), 4));
  index += sprintf((txpacket + index), "speed: %d.%d km/h\n", (int)GPS.speed.kmph(), fracPart(GPS.speed.kmph(), 3));
  txpacket[index] = '\0';
}


void sendPacket() {
  /* Increment packets transmitted */
  txNumber++;

  /* Turn colour to let user know data is sent */
  turnOnRGB(COLOR_SEND, 0);
  Radio.Send((uint8_t*)txpacket, strlen(txpacket));
  lora_idle = false;
}


void VextON(void) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
}


void VextOFF(void) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, HIGH);
}

void OnTxDone(void) {
  turnOffRGB();
  Serial.println("TX done......");
  lora_idle = true;
  // delay(10000);
}


void OnTxTimeout(void) {
  turnOffRGB();
  Radio.Sleep();
  Serial.println("TX Timeout......");
  lora_idle = true;
}


int fracPart(double val, int n) {
  return (int)((val - (int)(val)) * pow(10, n));
}

static void smart_delay(const unsigned long ms) {
  Serial.println("************* START SMART_DELAY *************");
  Serial.print("Func: smart_delay() For:\t");
  Serial.println(ms / SEC_1);

  unsigned long start = millis();

  Serial.print("Secs:\t");
  Serial.println(start / SEC_1);

  // loop smart delay
  while (millis() - start < ms && start) {
    // feed gps
    GPS.encode(GPS.read());
  }
  Serial.println("************* END SMART_DELAY *************");

  // smart delay completed. if message arrived, process.
}

/*
* @brief Manages the entry into ARM cortex deep-sleep mode
 * @param none
 * @retval none
 void lowPowerHandler( void )
*/
