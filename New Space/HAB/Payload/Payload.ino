#include "Constants.h"

Air530Class GPS;
char TX_PACKET[BUFFER_SIZE];      /* Packet to send */
static RadioEvents_t RadioEvents; /* LoRa radio object to send status to GS */
int CONSECUTIVE_DANGER;           /* [UNUSED CURRENTLY] Count consecutive occurences which we are descending and below the required altitude. */
                                  /* If it's large enough maybe the GPS needs a restart or the liquid mechanism is stuck  */

int ALTS[ALT_MEMORY_SIZE];        /* Remember past altitudes, make decision based on it */
int ALTS_POS = 0;
enum STATES { INITIAL_ASCENT,
              FLOATING,
              DANGER,
              STANDBY,
              LIQUID_EMPTY };
/*
  INITIAL_ASCENT - First ascent, goal is to rise with no conditions.
  FLOATING - Altitude is stable, state of buoyancy, can save power.
  DANGER -   Balloon is descending fast and altitude is low.
  STANDBY -  On standby after pumping water, decide if still in danger or not.
*/


void VextON(void);
void VextOFF(void);
void preparePacket();
void sendPacket();
void pump(int seconds);
void log_altitude();
int real_index(int i, int start, int direction);
double variance(int* array, int start, int steps, int direction);
double average_();
double average(int* array, int start, int steps, int direction);
int at_risk();
void test_system();



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
                    CRC, 0, 0, LORA_IQ_INVERSION_ON, 4000);
  Radio.SetSyncWord(LORA_SYNCWORD);

  /* GPS Init */
  GPS.begin();

  /* Pump Init */
  pinMode(PUMP_PIN, OUTPUT);

  /* Init alt memory */
  for (int i = 0 ; i < ALT_MEMORY_SIZE; i++)
    ALTS[i] = 0;
  
  
  Serial.println("Payload init");
}


void loop() {
  smart_delay(30 * SEC_1);  //every 30 secs
  preparePacket();
  sendPacket();
  
  switch (State) {
    case INITIAL_ASCENT:
      {
        State = average_() < (ASCENT_LIM)? INITIAL_ASCENT : FLOATING;
        break;
      }

    case FLOATING:
      {
        // CONSECUTIVE_DANGER = 0;

        // check if in danger, maybe add more delay

        if (GPS.altitude.age() < GPS_TOO_OLD)
        {
          State = at_risk() ? DANGER : FLOATING;
        }
        break;
      }

    case DANGER:
      {
        /* Count how many times we had to pump in a row- if it's too much, must have ran out of liquid.
        Switch state to save on power.*/
        
        
        // CONSECUTIVE_DANGER++;
        // if (CONSECUTIVE_DANGER > MAX_DANGER_ITERS)
        // {
        //   State = LIQUID_EMPTY;
        //   break;
        // }

        /* Danger = Going down, get rid of weight */
        pump(PUMP_ON_SEC * SEC_1);

        /* Wait a bit, check for changes- maybe there is still danger */
        State = STANDBY;
        break;
      }

    case STANDBY:
      {
        /* Wait, then check if still in danger */
        smart_delay(STANDBY_WAIT * SEC_1);
        State = at_risk() ? DANGER : FLOATING;
        break;
      }

      case LIQUID_EMPTY:
      {
        smart_delay(5 * SEC_1);
        break;
      }
  }
}


int real_index(int i, int start, int direction)
{
  int index = ((i * direction) + start);

  /* Same as % */
  if (index < 0)
  {
    index += ALT_MEMORY_SIZE;
  }
  if (index >= ALT_MEMORY_SIZE)
  {
    index -= ALT_MEMORY_SIZE;
  }
  return index;
}


double average_()
{
  return average(ALTS, 0, ALT_MEMORY_SIZE, 1);
}


double average(int* array, int start, int steps, int direction)
{
  double sum = 0;
  for (int i = 0; i < steps; i++)
  {
    int index = real_index(i, start, direction);
    sum = sum + array[index];
  }
  return sum / steps;
}


double variance(int* array, int start, int steps, int direction)
{
  /*
  Starts at a given location, continues some steps in a direction and loops around
  when reaching an index matching the size of the array.

      For example, we get an array with 4 steps, start pos is 2   
               sp
      A0  A1   A2   A3   A4   A5   A6   A7

      That means the sub-array is A2 A1 A0 A7, as we loop around
       l  l    l                        l
      A0  A1   A2   A3   A4   A5   A6   A7
  */

  if (steps < 1)
  {
    return 0;
  }

  float sum = 0;
  float var = 0;
  float avrg = 0;

  /* Calculate average */
  avrg = average(array, start, steps, direction);
  
  /* Calculate variance */
  for (int i = 0; i < steps; i++)
  {
    int index = real_index(i, start, direction);
    float res = (array[index] - avrg) * (array[index] - avrg);
    var = var + res;
  }

  return var / (steps - 1);
}


int at_risk() {
  /* Divide altitude memory into two sub-arrays, A and B. 
     From A or B, choose the one with less spikes \ outliers. */
  int steps = ALT_MEMORY_SIZE / 2;
  int left_var = variance(ALTS, ALTS_POS, steps, -1);
  int right_var = variance(ALTS, ALTS_POS + 1, steps, 1);

  /* Update current sub-array to be A or B- on it perform checks to see if we are descending or ascending. */
  int start = ALTS_POS;
  int dir = -1;
  if (right_var < left_var)
  {
    start = ALTS_POS + 1;
    dir = 1;
  }

  /* Divide the sub-array to two , check if the newest altitude half has a lower average.

    For example, after choosing a sub-array of eight elements by variance, we get
    A0 A1 A2 A3 | A4 A5 A6 A7

    Assuming A0 to A7 are arranged in order of update.
    if avg(A0, A1, A2, A3) < avg(A4, A5, A6, A7), then the latest information has a higher average -> we are rising.
  */
  
  steps = steps / 2;
  int latest_average = average(ALTS, start, steps, -1);
  int oldest_average = average(ALTS, start - steps, steps, -1);

  int descending = latest_average < oldest_average;
  int below_lim = latest_average < LOWER_LIMIT;

  return descending && below_lim;
}


void log_altitude() {
  if ( !GPS.altitude.isValid() )
    return;

  ALTS[ALTS_POS] = (int) GPS.altitude.meters();
  ALTS_POS++;
  if (ALTS_POS >= ALT_MEMORY_SIZE)
  {
    ALTS_POS = 0;
  }
}


void pump(int seconds) {
  digitalWrite(PUMP_PIN, HIGH);   
  delay(seconds * SEC_1);              
  digitalWrite(PUMP_PIN, LOW);   
}


void preparePacket() {
  /* Prepare packet to send to ground station, consisting of current state, and GPS info */  
  int index = 0;
  char state_c;
  switch (State)
  {
    case INITIAL_ASCENT: { state_c = 'I'; break; }
    case FLOATING:       { state_c = 'F'; break; }
    case DANGER:         { state_c = 'D'; break; }
    case STANDBY:        { state_c = 'S'; break; }
    case LIQUID_EMPTY:   { state_c = 'E'; break; }
    default:             { state_c = '-'; } // Shouldn't really happen
  }

  index += sprintf((TX_PACKET + index), "%c\n", state_c);
  index += sprintf((TX_PACKET + index), "%02d:%02d:%02d.%02d\n", GPS.time.hour(), GPS.time.minute(), GPS.time.second(), GPS.time.centisecond());  
  index += sprintf((TX_PACKET + index), "alt:%d\n", (int)GPS.altitude.meters());
  index += sprintf((TX_PACKET + index), "hdop:%d\n", (int)GPS.hdop.hdop());
  index += sprintf((TX_PACKET + index), "lat:%.4f\n", GPS.location.lat());
  index += sprintf((TX_PACKET + index), "lon:%.4f\n", GPS.location.lng());
  index += sprintf((TX_PACKET + index), "speed:%dkm/h\n", (int)GPS.speed.kmph());
  TX_PACKET[index] = '\0';
}


void sendPacket() {
  /* Turn colour to let user know data is sent */
  turnOnRGB(COLOR_SEND, 0);
  Radio.Send((uint8_t*)TX_PACKET, strlen(TX_PACKET));
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
  // Serial.println("TX done......");
  // smart_delay(10 * SEC_1);
}


void OnTxTimeout(void) {
  turnOffRGB();
  Radio.Sleep();
  Serial.println("TX Timeout......");
}


static void smart_delay(const unsigned long ms) 
{
  Serial.println("************* START SMART_DELAY *************");
  Serial.print("Func: smart_delay() For:\t");
  Serial.println(ms / SEC_1);

  unsigned long start = millis();

  Serial.print("Secs:\t");
  Serial.println(start / SEC_1);

  // loop smart delay
  while (millis() - start < ms && start) 
  {
    // feed gps
    while (GPS.available() > 0)
    {
      GPS.encode(GPS.read());
    }
    // add gps info to memory
    log_altitude();
  }
  Serial.println("************* END SMART_DELAY *************");

  // smart delay completed. if message arrived, process.
}


void assert_(int condition)
{
  if (condition)
    return;

  Serial.println("Assertion failed");
  while(1) {};
}


void test_system()
{
  // Serial.println("Testing GPS");
  // smart_delay(100 * SEC_1);
  // assert_(GPS.altitude.isUpdated());
  // assert_(GPS.altitude.meters() > 0);

  Serial.println("Testing LoRa");
  preparePacket();
  sendPacket();

  Serial.println("Testing Pump");
  pump(1);
  
  Serial.println("Testing At Risk function");

  /* "Simulate" altitude starting at 6000 and increasing with occasional spikes */
  ALTS[0] = 6000; 
  for (int i = 0; i < 1000; i++)
  {
    int previous = ALTS[ (ALTS_POS - 1) % ALT_MEMORY_SIZE ];
    ALTS[ALTS_POS] = previous + (rand() % 20) ;
    ALTS_POS = (ALTS_POS + 1) % ALT_MEMORY_SIZE;
  }

  /* Randomly subtract, simulate spikes */
  for (int i = 0; i < ALT_MEMORY_SIZE; i++)
  {
    if (rand() % 5 == 1) ALTS[i] -= 50;
  }

  Serial.println("Testing incrase in altitude.");
  assert_(at_risk() == 0);


  /* "Simulate" altitude starting at 8000 and descending with occasional positive spikes */
  ALTS[0] = 8000; 
  for (int i = 0; i < 500; i++)
  {
    int previous = ALTS[ (ALTS_POS - 1) % ALT_MEMORY_SIZE ];
    ALTS[ALTS_POS] = previous - (rand() % 20) ;
    ALTS_POS = (ALTS_POS + 1) % ALT_MEMORY_SIZE;
  }

  /* Randomly subtract, simulate spikes */
  for (int i = 0; i < ALT_MEMORY_SIZE; i++)
  {
    if (rand() % 5 == 1) ALTS[i] += 100;
  }

  Serial.println("Testing decrease at risky altitude.");
  assert_(at_risk() == 1);

  Serial.println("Tests OK");
}