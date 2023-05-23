#include "heltec.h"
#include <ESP32Servo.h>

Servo myservo;
int pos = 0;

// Pin 26 is not recommended in the manual for the esp32-wireless stick
int servoPin = 25;

void setup() {
  myservo.attach(servoPin);   // attaches the servo on pin 18 to the servo object
                              // using default min/max of 1000us and 2000us
                              // different servos may require different min/max settings
                              // for an accurate 0 to 180 sweep
}

int INC = 5;
void loop() 
{
  for (pos = 0; pos <= 180; pos += INC) 
  {
    myservo.write(pos);
    delay(50);
  }
  for (pos = 180; pos >= 0; pos -= INC) 
  {
    myservo.write(pos);
    delay(50);
  }
}
