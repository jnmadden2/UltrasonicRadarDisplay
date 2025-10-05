#include <math.h>
#include <Arduino.h>
#include <AceRoutine.h>

//using namespace std;
using namespace ace_routine;

float Vsound = 0.343; // Speed of sound (mm/micro s)
float distanceMult = 0.65;

const int PIN_T = 6;
const int PIN_R = 15;

float lamda = 0.008575; // Wavelength of a 40 kHz sound wave

int beginT, endT, timePassed, whileTime;

//int distance;

//-------------------------------
float lambdaMM = 8.575; // 0.343/40 lambda in milimeters
int degreesMax = 60;
float radiansMax = degreesMax * (PI / 180);

//critical distance (this is the optional spacing assuming 60 degrees of viewing in both directions). 
//This value should be replaced by the true distance between transmitters.
float dcr = lambdaMM / (1 + sin(radiansMax));

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


const int numTransmitters = 10; //number of transmitters

const int pulseQuantity = 10;

int angles[9] = {-60, -45, -30, -15, 0, 15, 30, 45, 60};  // Static local array


//-------------------------------

int calculatePhaseDelay(int angle){
  float radians = angle * (PI/180);
  
  float timeDelay = (dcr * abs(sin(radians))) / Vsound;

  int roundedTimeDelay = round(timeDelay); //have to round because there is no standard way to delay down to nano seconds without additional hardware

  return roundedTimeDelay;
}

//---------------------------------


void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_T, OUTPUT); // PIN_T as output
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:

  //alternative loop with 9 sample points for testing
  //for(int i = 0; i < 9; i++){
    //int a = angle[i];
  //
  for(int a = -60; a <= 60; a++){
    int delayBetween = calculatePhaseDelay(a);
    int timePassed = 0; // The amount of time that has passed (in microseconds) since the transmission
    bool cancelRead = false; // Used to to signify too long of a wait for a receving the pulse

    //int returnVal = 0; 
    float distance = 0; // Distance to be calculated (in mm)

    int angle;

    bool right;

    //convert angle to match normal unit circle values from +/- from perpendicular form
    if(a <= 0){
      angle = (a * -1) + 90;
      right = false;
    }
    else{
      angle = 90 - a;
      right = true;
    }

    beginT = micros();

    if (a == 0) {
      PulseStraight();
    }
    else if (a > 0){
      beamSteer(delayBetween, true);
    }
    else beamSteer(delayBetween, false);

    unsigned long whileTime = micros();
    while (analogRead(PIN_R) < 800 && !cancelRead)
    {

      // Check if we've gone too long without an input
      if (micros() - whileTime > 6000)
      {
        cancelRead = true;
      }
    }

    endT = micros();
    timePassed = endT - beginT;

    if (!cancelRead){
      distance = (timePassed/2) * Vsound;

      Serial.print(angle);
      Serial.print(",");
      Serial.print((int)distance);
      Serial.print(".");
    }
    else{
      //express failed reading
      Serial.print(angle);
      Serial.print(",");
      Serial.print(1);
      Serial.print(".");
    }

    delayMicroseconds(3000); //allow ample time for pulses to clear to send another (percaution for now this is ~1m of distance traveled)
  }
  
  //delay(200);
}

void pulse(){
  digitalWrite(PIN_T, HIGH); // Turn on pin 13
  delayMicroseconds(13);            // Wait for 13 micro seconds
  digitalWrite(PIN_T, LOW);  // Turn off pin 13
  delayMicroseconds(13);            // Wait for 13 micro seconds
}


//-----------------------------------------
COROUTINE(TR0) // For transmitter 1 (left-most)
{
  digitalWrite(PIN_TR0, HIGH); //turn on transmitter 1 
  delayMicroseconds(13);            // Wait for 13 micro seconds
  digitalWrite(PIN_TR0, LOW);       //turn off transmitter 1
  delayMicroseconds(13);            // Wait for 13 micro seconds
}

COROUTINE(TR1) // For transmitter 2
{
  digitalWrite(PIN_TR1, HIGH);      //turn on transmitter 2 
  delayMicroseconds(13);            // Wait for 13 micro seconds
  digitalWrite(PIN_TR1, LOW);        //turn off transmitter 2
  delayMicroseconds(13);            // Wait for 13 micro seconds
}

COROUTINE(TR2) // For transmitter 3
{
  digitalWrite(PIN_TR2, HIGH);      //turn on transmitter 3 
  delayMicroseconds(13);            // Wait for 13 micro seconds
  digitalWrite(PIN_TR2, LOW);        //Turn off transmitter 3
  delayMicroseconds(13);            // Wait for 13 micro seconds
}

COROUTINE(TR3) // For transmitter 4
{
  digitalWrite(PIN_TR3, HIGH);       //turn on transmitter 4 
  delayMicroseconds(13);            // Wait for 13 micro seconds
  digitalWrite(PIN_TR3, LOW);        //Turn off transmitter 4
  delayMicroseconds(13);            // Wait for 13 micro seconds
}

COROUTINE(TR4) // For transmitter 5 
{
  digitalWrite(PIN_TR4, HIGH);      //turn on transmitter 5
  delayMicroseconds(13);            // Wait for 13 micro seconds
  digitalWrite(PIN_TR4, LOW);        //Turn off transmitter 5
  delayMicroseconds(13);            // Wait for 13 micro seconds
}

COROUTINE(TR5) // For transmitter 6
{
  digitalWrite(PIN_TR5, HIGH);      //turn on transmitter 6 
  delayMicroseconds(13);            // Wait for 13 micro seconds
  digitalWrite(PIN_TR5, LOW);        //Turn off transmitter 6
  delayMicroseconds(13);            // Wait for 13 micro seconds
}

COROUTINE(TR6) // For transmitter 7
{
  digitalWrite(PIN_TR6, HIGH);      //turn on transmitter 7 
  delayMicroseconds(13);            // Wait for 13 micro seconds
  digitalWrite(PIN_TR6, LOW);        //Turn off transmitter 7
  delayMicroseconds(13);            // Wait for 13 micro seconds
}

COROUTINE(TR7) // For transmitter 8
{
  digitalWrite(PIN_TR7, HIGH);      //turn on transmitter 8
  delayMicroseconds(13);            // Wait for 13 micro seconds
  digitalWrite(PIN_TR7, LOW);        //Turn off transmitter 8
  delayMicroseconds(13);            // Wait for 13 micro seconds
}

COROUTINE(TR8) // For transmitter 9 
{
  digitalWrite(PIN_TR8, HIGH);      //turn on transmitter 9 
  delayMicroseconds(13);            // Wait for 13 micro seconds
  digitalWrite(PIN_TR8, LOW);       //Turn off transmitter 9
  delayMicroseconds(13);            // Wait for 13 micro seconds
}

COROUTINE(TR9) // For transmitter 10 (right-most)
{
  digitalWrite(PIN_TR9, HIGH);      //turn on transmitter 10 
  delayMicroseconds(13);            // Wait for 13 micro seconds
  digitalWrite(PIN_TR9, LOW);        //Turn off transmitter 10
  delayMicroseconds(13);            // Wait for 13 micro seconds
}
//--------------------------------------------
//steering functions

//straight ahead steering
void PulseStraight(){
  digitalWrite(PIN_TR0, HIGH);
  digitalWrite(PIN_TR1, HIGH);
  digitalWrite(PIN_TR2, HIGH);
  digitalWrite(PIN_TR3, HIGH);
  digitalWrite(PIN_TR4, HIGH);
  digitalWrite(PIN_TR5, HIGH);
  digitalWrite(PIN_TR6, HIGH);
  digitalWrite(PIN_TR7, HIGH);
  digitalWrite(PIN_TR8, HIGH);
  digitalWrite(PIN_TR9, HIGH);
  delayMicroseconds(13);
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
  delayMicroseconds(13);

}

//directional steering
void beamSteer(int timeDelay, bool direction){
  int trF, trL;

  if (direction){ //scan right
    trF = 0;
    trL = numTransmitters;
  } 
  else{
    trF = numTransmitters - 1;
    trL = 0;
  }
  
  for (int tr = trF; (direction ? tr < trL : tr >= trL); direction ? tr++ : tr--){
    switch(tr){
      case 0:
      TR0.runCoroutine();
      break;
      case 1:
      TR1.runCoroutine();
      break;
      case 2:
      TR2.runCoroutine();
      break;
      case 3:
      TR3.runCoroutine();
      break;
      case 4:
      TR4.runCoroutine();
      break;
      case 5:
      TR5.runCoroutine();
      break;
      case 6:
      TR6.runCoroutine();
      break;
      case 7:
      TR7.runCoroutine();
      break;
      case 8:
      TR8.runCoroutine();
      break;
      case 9:
      TR9.runCoroutine();
      break;
      default:
      //Console.println("error");
      break;
    }

    delayMicroseconds(timeDelay);
  }

}