#include "Constants.h"

char TX_PACKET[BUFFER_SIZE];      /* Packet to send */
static RadioEvents_t RadioEvents; /*   */
bool LORA_IDLE = true;            /*   */

const int SEC_1 = 1000;

Air530Class GPS;
Servo SERVO;
int SERVO_POS = 0;
int CONSECUTIVE_DANGER;

int ALTS[ALT_MEMORY_SIZE]; /* Remember altitude of past few minutes */
enum STATES { INITIAL_ASCENT,
              FLOATING,
              DANGER,
              STANDBY };
/*
  INITIAL_ASCENT - First ascent, goal is to rise with no conditions.
  FLOATING - Altitude is stable, state of buoyancy, can save power.
  DANGER -   Balloon is descending fast and altitude is low.
  STANDBY -  On standby after dribble, decide if still in danger or not.
*/


void VextON(void);
void VextOFF(void);
int fracPart(double val, int n);
void updateGPS();
void preparePacket();
void sendPacket();
void activateServo(int degrees);
void dribble();
void verify_system();
void log_altitude();


enum STATES State = INITIAL_ASCENT;
void setup() {
  VextON();

  /* Serial Init */
  Serial.begin(115200);

  /* Lora Init */
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
  Radio.SetSyncWord(0x12);

  /* GPS Init */
  GPS.begin();

  /* Servo Init */
  SERVO.attach(GPIO4);
  Serial.println("Payload init");
}


void loop() {
  smart_delay(30 * SEC_1);  //every 30 secs
  // updateGPS(); //updates on smart delay
  log_altitude(); //pass to smart delay?
  preparePacket();
  sendPacket();
  switch (State) {
    case INITIAL_ASCENT:
      {
        //
        break;
      }

    case FLOATING:
      {
        CONSECUTIVE_DANGER = 0;

        //check floating direction
        break;
      }

    case DANGER:
      {
        //add to danger counter
        CONSECUTIVE_DANGER++;
        if (CONSECUTIVE_DANGER > MAX_DANGER_ITERS)
          verify_system();
        //dribble func
        dribble();
        State = STANDBY;
        WAIT_ITERS = 10;
        break;
      }

    case STANDBY:
      {
        smart_delay(WAIT_ITERS * SEC_1);
        State = is_descending() ? DANGER : FLOATING;
        break;
      }
  }
}
// void updateGPS() {
//   uint32_t starttime = millis();
//   while ((millis() - starttime) < 1000) {
//     while (GPS.available() > 0) {
//       GPS.encode(GPS.read());
//     }
//   }
// }

bool is_descending() {
  return false;
}

void log_altitude() {
}


void dribble() {
  activateServo(90);
}


void verify_system() {
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


void activateServo(int degrees) {
  /* Open */
  for (SERVO_POS = 0; SERVO_POS <= degrees; SERVO_POS += 1) {
    SERVO.write(SERVO_POS);
    smart_delay(30);
  }
  smart_delay(SEC_1);  //hold a sec
  /* Close */
  for (SERVO_POS = degrees; SERVO_POS >= 0; SERVO_POS -= 1) {
    SERVO.write(SERVO_POS);
    smart_delay(30);
  }
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
  smart_delay(10 * SEC_1);
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
    GPS.encode(GPS.read())
  }
  Serial.println("************* END SMART_DELAY *************");

  // smart delay completed. if message arrived, process.
}
