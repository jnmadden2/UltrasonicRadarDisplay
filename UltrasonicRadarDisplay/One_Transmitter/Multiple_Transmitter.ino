#include <Arduino.h>
#include <esp32-hal-ledc.h>
#include <esp32-hal.h>

#include "soc/soc_caps.h"
float Vsound = 0.343; // Speed of sound (mm/micro s)
float distanceMult = 0.65;
int beginT, endT, timePassed, whileTime, distance;


const int pinTrans[] = {15, 16, 17, 18, 19, 21, 22, 23, 24, 25, 26};
const int numTrans = 5; //sizeof(pulsePins);
const int pulseWidth = 12;
const int pulseInterval = 12;
const int PIN_R = 7;

// Phased-Array variables
float d = 0.65; // The distance between transmitters
float lamda = 0.008575; // Wavelength of a 40 kHz sound wave
float theta = 0; // Desired Angle (in Radians)
float frequency = 40000;
float pi = 3.14159265359;

void setup() {
  Serial.begin(115200);

  //pin seup //se
  for (int i = 0; i < numTrans; i++){
    
    pinMode(pinTrans[i], OUTPUT); //ensure pins used for transmitters are outputs
    pinMode(pinTrans[i], LOW); //ensure pins start at a low state
  }
  pinMode(PIN_R, INPUT); //ensure reciever pin is an input
}

void loop() {
  bool cancelRead = false; // Returns true if we've gone 10 ms without receiving anything (~3.5 meters)
  int returnVal = 0; // AnalogRead(0)
  float distance = 0; // Distance to be calculated (in mm)
 
  for (int i = 0; i < numTrans; i++) {
    digitalWrite(pinTrans[i], HIGH);
    delayMicroseconds(pulseWidth);
    digitalWrite(pinTrans[i], LOW);


    beginT = micros();
//  whileTime = micros();

  while (analogRead(6) < 800 && !cancelRead){ //updated to use right pin input, and updated threshold (this may need additional changes)
 

    if (micros() - whileTime > 10000){
      cancelRead = true;
    }

    returnVal = analogRead(A0);

  }

  endT = micros();

  timePassed = endT - beginT;

  if (!cancelRead){
    theta = asin((Vsound * timePassed/2) / (d * frequency));
    int phaseShift = (2 * pi * d * sin(theta)) / frequency;

    distance = (d * sin(theta)) * Vsound;
//    int angle = 360 *(micros() - beginT) / 40000;

    int angle = (theta / 180 * PI);
    Serial.print(angle);
    Serial.print(",");
    Serial.print(phaseShift);
    Serial.print(".");
  }
  }
}


//code for a percise pulse :)
//
//const int pinTrans[] = {15, 16, 17, 18, 19, 21, 22, 23, 24, 25, 26};
//const int numTrans = 5; //sizeof(pulsePins);
//const int PIN_R = 7;
//
//const int freq = 1000; //frequency in Hz
//const int resolution = 8; //8 bit resolution (0-255)
//const int dutyHigh = 127; //50% duty cycle (127/255)
//const int dutyLow = 0; //0% futy cycle (off)
//const int pulseDelay = 300; //delay between pulses (miliseconds)
//
//void setup() {
//  Serial.begin(115200);
//
//  //configure pwn for each pin
//  for (int i = 0; i < numTrans; i++) {
//    ledcAttach(pinTrans[i], freq, resolution); //setup and attach PWN channel
//        //this should work
//        
//    ledcWrite(i, dutyLow); //start with LEDs off
//  }
//}
//
//void loop(){
//  for (int i = 0; i < numTrans; i++) {
//    Serial.printf("Pulsing pin %d\n", pinTrans[i]);
//
//    ledcWrite(i, dutyHigh); //start pulse
//    delayMicroseconds(12);
//    ledcWrite(i, dutyLow); //turn pulse off
//
//    delay(pulseDelay);
//  }
//
//}
