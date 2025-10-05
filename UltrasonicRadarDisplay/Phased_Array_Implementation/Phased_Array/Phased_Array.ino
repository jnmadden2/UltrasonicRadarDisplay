#include <math.h>
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/adc.h>
#include <stdint.h>
#include "esp_timer.h"

const unsigned long cyclesPerSecond = 240000000;
const unsigned long cyclesPerMicroSecond = cyclesPerSecond / 1000000;

float Vsound = 0.343; // Speed of sound (mm/micro s)
float distanceMult = 0.65;

const int PIN_T = 4;
//const int PIN_R = 15;
const int PIN_R = 5;

float lamda = 0.008575; // Wavelength of a 40 kHz sound wave


//-------------------------------
float lambdaMM = 8.575; // wave length in mm
int degreesMax = 60;
float radiansMax = degreesMax * (PI / 180);

//critical distance (this is the optional spacing assuming 60 degrees of viewing in both directions). 
//This value should be replaced by the true distance between transmitters.
float dcr = lambdaMM / (1 + sin(radiansMax));

float dTrue = 10.0; // true distance of 10 mm due to transmitter size

const int PIN_TR0 = 1; //place holder for transmitter 0
const int PIN_TR1 = 2; //place holder for transmitter 1
const int PIN_TR2 = 3; //place holder for transmitter 2
const int PIN_TR3 = 4; //place holder for transmitter 3
const int PIN_TR4 = 5; //place holder for transmitter 4
const int PIN_TR5 = 7; //place holder for transmitter 5
const int PIN_TR6 = 8; //place holder for transmitter 6
const int PIN_TR7 = 9; //place holder for transmitter 7
const int PIN_TR8 = 10; //place holder for transmitter 8
const int PIN_TR9 = 11; //place holder for transmitter 9

const int transmitterPins[] = {1, 2, 3, 4, 5, 7, 8, 9, 10, 11};


const int numTransmitters = 10; //number of transmitters

const int pulseQuantity = 10;

int angles[9] = {-60, -45, -30, -15, 0, 15, 30, 45, 60};  // Static array of angle increments to test

TaskHandle_t receiverTaskHandle = NULL;
TaskHandle_t transmitterTaskHandle = NULL;

unsigned long startTime;

//flags (maybe make into semaphores)
int flagReadable = 0;
int flagScannable = 1;
int angle;

//-------------------------------
//calculates time delay between transmitters
int calculatePhaseDelay(int angle){
  float radians = angle * (PI/180); // angle to radians
  
  float timeDelay = (dTrue * abs(sin(radians))) / Vsound; //note absolute value is used (otherwise steering to the left gives negative time) just steer the opisite direction with same time delay 

  int roundedTimeDelay = round(timeDelay); //have to round because there is no standard way to delay down to nano seconds without additional hardware

  return roundedTimeDelay; //time delay between transmitters
}

// --------------------------------------

void TaskReceiver(void *pvParameters) {
  int invalid = 0;
  unsigned long afterTime = 0;
  unsigned long TOF = 0;
  int distance;
    
  while (1) {
    //int signal = analogRead(PIN_R);

    invalid = 0;

    while(!flagReadable){
      continue;
    }

    unsigned long initial =  esp_timer_get_time() - startTime;
    while(!invalid && flagReadable &&  adc1_get_raw(ADC1_CHANNEL_4) < 800){ //mess with the minimum value 800 ~= 0.761 volts
      if (esp_timer_get_time() - initial > 7300)//too long distance to be valid
      {
        invalid = 1;
      }
    }

    afterTime = esp_timer_get_time();
    TOF = afterTime - startTime;

    if (!invalid){
      distance = (TOF/2) * Vsound;

      Serial.print(angle);
      Serial.print(",");
      Serial.print((int)distance);
      Serial.print(".");
    }
    else{
      //express failed reading
      Serial.print(0);
      Serial.print(",");
      Serial.print(0);
      Serial.print(".");
    }

    flagScannable = 1; //ready to scan another direction
  }
}

//-----------------------------------------------------------
void phasedTransmit(void *params){
  int delayBetween;
  int timePassed;
  float distance;
  //int angle;
  int right;
  
  //alternative loop with 9 sample points for testing
  //for(int i = 0; i < 9; i++){
    //int a = angle[i];
  //
  while(1){
    for(int a = -60; a <= 60; a++){
      delayBetween = calculatePhaseDelay(a); //time delay to use
      //timePassed = 0; // The amount of time that has passed (in microseconds) since the transmission
      distance = 0;

      //not ready to start new scan direction
      while(!flagScannable){
        continue;
      }

      //convert angle to match normal unit circle values from +/- from perpendicular form and pulse
      if(a == 0){
        angle = 90;
        PulseStraight();
      }

      else if(a < 0){
        angle = (a * -1) + 90;
        beamSteerL(delayBetween);
        //right = false;


      }
      else{
        angle = 90 - a;
        beamSteerR(delayBetween);
        //right = true;
      }

      delayMicroseconds(8700); //delay to clear field for scanning.

    }
  }
}

//---------------------------------

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_T, OUTPUT); // PIN_T as output

  // Set transmitter pins as OUTPUT 
  for (int i = 0; i < numTransmitters; i++) {
    pinMode(transmitterPins[i], OUTPUT);
    digitalWrite(transmitterPins[i], LOW); // Ensure they start LOW
  }

  // Set receiver pin as INPUT
  pinMode(PIN_R, INPUT);
  adc1_config_width(ADC_WIDTH_BIT_12);  // 12-bit ADC resolution (0-4095)
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);  // Read up to 3.9V  

  Serial.begin(115200);

  xTaskCreatePinnedToCore(TaskReceiver, "Receiver", 10000, 0, 2, &receiverTaskHandle, 1); //Receiver Task
  xTaskCreatePinnedToCore(phasedTransmit, "Transmit", 10000, 0, 2, &transmitterTaskHandle, 0); //Transmitter Task
}

//---------------------------------------

void loop() {
  // put your main code here, to run repeatedly:
  taskYIELD();
}

//--------------------------------------------
//steering functions

//straight ahead steering (3 cycles pulse)
void PulseStraight(){
  digitalWrite(PIN_TR0, HIGH);
  digitalWrite(PIN_TR1, HIGH);
  digitalWrite(PIN_TR2, HIGH);
  digitalWrite(PIN_TR3, HIGH);
  digitalWrite(PIN_TR4, HIGH);
  startTime = esp_timer_get_time();
  flagReadable = 1;
  digitalWrite(PIN_TR5, HIGH);
  digitalWrite(PIN_TR6, HIGH);
  digitalWrite(PIN_TR7, HIGH);
  digitalWrite(PIN_TR8, HIGH);
  digitalWrite(PIN_TR9, HIGH);
  delayMicroseconds(75);
  digitalWrite(PIN_TR0, LOW);
  digitalWrite(PIN_TR1, LOW);
  digitalWrite(PIN_TR2, LOW);
  digitalWrite(PIN_TR3, LOW);
  digitalWrite(PIN_TR4, LOW);
  digitalWrite(PIN_TR5, LOW);
  digitalWrite(PIN_TR6, LOW);
  digitalWrite(PIN_TR7, LOW);
  digitalWrite(PIN_TR8, LOW);
  digitalWrite(PIN_TR9, LOW);
  delayMicroseconds(75);

}

//break into directions into different functions for efficency

//steer right
void beamSteerR(int timeDelay){
  int stopNext = 0;
  int startNext = 0;

  unsigned long stopT = 75;
  unsigned long fireT = 0;

  uint64_t current;
  uint64_t start = ESP.getCycleCount();

  while(startNext < 10){
    current = ESP.getCycleCount();

    if(((current-start)/cyclesPerMicroSecond) >= fireT){
      digitalWrite(transmitterPins[startNext], HIGH);
      fireT += timeDelay;
      startNext++;
      //between middle transmitters
      if(startNext == 5){
        flagReadable = 1;
        startTime = esp_timer_get_time();
      }
    }

    if(((current-start)/cyclesPerMicroSecond) >= stopT){
      digitalWrite(transmitterPins[stopNext], LOW);
      stopT += timeDelay;
      stopNext++;
    }

  }

  while(stopNext < 10){
    current = ESP.getCycleCount();

    if(((current-start)/cyclesPerMicroSecond) >= stopT){
      digitalWrite(transmitterPins[stopNext], LOW);
      stopT += timeDelay;
      stopNext++;
    }
  }
}

//steer Left
void beamSteerL(int timeDelay){
  int stopNext = 9;
  int startNext = 9;

  unsigned long stopT = 75;
  unsigned long fireT = 0;

  uint64_t current;
  uint64_t start = ESP.getCycleCount();

  while(startNext > -1){
    current = ESP.getCycleCount();

    if(((current-start)/cyclesPerMicroSecond) >= fireT){
      digitalWrite(transmitterPins[startNext], HIGH);
      fireT += timeDelay;
      startNext--;
      //between middle transmitters
      if(startNext == 4){
        flagReadable = 1;
        startTime = esp_timer_get_time();
      }
    }

    if(((current-start)/cyclesPerMicroSecond) >= stopT){
      digitalWrite(transmitterPins[stopNext], LOW);
      stopT += timeDelay;
      stopNext--;
    }

  }

  while(stopNext > -1){
    current = ESP.getCycleCount();
    if(((current-start)/cyclesPerMicroSecond) >= stopT){
      digitalWrite(transmitterPins[stopNext], LOW);
      stopT += timeDelay;
      stopNext--;
    }
  }
}
