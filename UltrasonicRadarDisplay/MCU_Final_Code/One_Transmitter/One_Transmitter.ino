#include <Arduino.h>
#include <WiFi.h>
#include <soc/gpio_struct.h>

float Vsound = 0.343; // Speed of sound (mm/micro s)
float distanceMult = 0.65;
const unsigned long cyclesPerSecond = 240000000;
const unsigned long cyclesPerMicroSecond = cyclesPerSecond / 1000000;


const int PIN_T = 37;
const int PIN_R = 7;

float lamda = 0.008575; // Wavelength of a 40 kHz sound wave

unsigned long beginT, endT, timePassed, whileTime;

int distance;

void setup() {
  Serial.begin(115200);

  //WiFi.mode(WIFI_STA);
  //Serial.println(WiFi.macAddress());
  
  //analogReadResolution(12);  // Set ADC to 12-bit (0-4095)
  //analogSetAttenuation(ADC_11db);  // Extend range to 0-3.3V
  setCpuFrequencyMhz(240);
  // put your setup code here, to run once:'
  pinMode(35, OUTPUT);
  pinMode(36, OUTPUT);
  pinMode(PIN_T, OUTPUT); // PIN_T as output
  pinMode(38, OUTPUT);
  pinMode(39, OUTPUT);
  pinMode(40, OUTPUT);
 
  pinMode(PIN_R, INPUT_PULLDOWN);
 
  digitalWrite(35, HIGH);
  digitalWrite(36, HIGH);
  digitalWrite(PIN_T, HIGH);
  digitalWrite(38, HIGH);
  digitalWrite(39, HIGH);
  digitalWrite(40, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  //pulse();
  bool cancelRead = false; // Returns true if we've gone 10 ms without receiving anything (~3.5 meters)

  int returnVal = 0; // AnalogRead(0)
  float distance = 0; // Distance to be calculated (in mm)

  beginT = micros();
 // Serial.print(beginT);
  //Serial.print("\n");
  pulse();


  whileTime = micros();
  delayMicroseconds(200); //delay for near range readings

  while (analogRead(PIN_R) < 800 && !cancelRead) //updated to use right pin input, and updated threshold (this may need additional changes)
  {

    if (micros() - whileTime > 10000)
    {
      cancelRead = true;
      Serial.print("No Readings");
      break;
    }

    returnVal = analogRead(PIN_R);

  }


  endT = micros();

  timePassed = endT - beginT;

  if (!cancelRead){
    distance = (int)(((timePassed) * Vsound)/2); //for actual distance
    //int angle = (theta / 180 * PI);
    Serial.print(90);
    Serial.print(",");
    Serial.print((int)distance);
    Serial.print(".");
  }
  
  delay(5000);
}

//40khz pulse
void pulse(){
  uint64_t start;
  start = ESP.getCycleCount();
  REG_WRITE(GPIO_OUT1_W1TC_REG, (1 << (PIN_T - 32)));
  while ((ESP.getCycleCount() - start) < (75 * 240));  // Wait 75 µs in cycles
  REG_WRITE(GPIO_OUT1_W1TS_REG, (1 << (PIN_T - 32)));
}

