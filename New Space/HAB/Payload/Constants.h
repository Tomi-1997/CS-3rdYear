#include <Wire.h>  
#include "HT_SSD1306Wire.h"
#include "GPS_Air530.h"
#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "CubeCellServoTimers.h"

#define UPPER_LIMIT 12000
#define LOWER_LIMIT 8000
#define MARGIN 2000
#define ALT_MEMORY_SIZE 128
#define MAX_DANGER_ITERS 10 // Number of consecutive DANGER states allowed before verifying system components.

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
#define BUFFER_SIZE                                 256        // Payload size

