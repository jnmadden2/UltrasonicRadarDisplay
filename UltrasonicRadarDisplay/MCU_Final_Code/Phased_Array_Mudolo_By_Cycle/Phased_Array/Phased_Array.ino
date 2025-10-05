#include <math.h>
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include <driver/adc.h>
#include <stdint.h>
#include "esp_timer.h"
#include <WiFi.h>
#include <PubSubClient.h>

const unsigned long cyclesPerSecond = 240000000;
const unsigned long cyclesPerMicroSecond = cyclesPerSecond / 1000000;

float Vsound = 0.343; // Speed of sound (mm/micro s)
float distanceMult = 0.65;

//const int PIN_T = 4;
//const int PIN_R = 15;
const int PIN_R = 7;

float lamda = 0.008575; // Wavelength of a 40 kHz sound wave


//-------------------------------
float lambdaMM = 8.575; // wave length in mm
int degreesMax = 60;
float radiansMax = degreesMax * (PI / 180);

//critical distance (this is the optional spacing assuming 60 degrees of viewing in both directions). 
//This value should be replaced by the true distance between transmitters.
float dcr = lambdaMM / (1 + sin(radiansMax));

float dTrue = 10.0; // true distance of 10 mm due to transmitter size

const int PIN_TR0 = 35; //place holder for transmitter 0
const int PIN_TR1 = 36; //place holder for transmitter 1
const int PIN_TR2 = 37; //place holder for transmitter 2
const int PIN_TR3 = 38; //place holder for transmitter 3
const int PIN_TR4 = 39; //place holder for transmitter 4
const int PIN_TR5 = 40; //place holder for transmitter 5
// const int PIN_TR6 = 8; //place holder for transmitter 6
// const int PIN_TR7 = 9; //place holder for transmitter 7
// const int PIN_TR8 = 10; //place holder for transmitter 8
// const int PIN_TR9 = 11; //place holder for transmitter 9

const int transmitterPins[] = {35,36,37,38,39,40};


const int numTransmitters = 10; //number of transmitters

const int pulseQuantity = 10;

int angles[9] = {-60, -45, -30, -15, 0, 15, 30, 45, 60};  // Static array of angle increments to test

TaskHandle_t receiverTaskHandle = NULL;
TaskHandle_t transmitterTaskHandle = NULL;

SemaphoreHandle_t semReadable;
SemaphoreHandle_t semScannable;

unsigned long startTime;

//flags (maybe make into semaphores)
int flagReadable = 0;
int flagScannable = 1;
bool endCycle = false;
int angle;
String message = "";

//------------Raspberry PI variables-------------------
 const char *ssid = "sdmay25-36"; //name of the MQTT server
 const char *password = "password"; //password for the MQTT server
 const char* mqtt_server = "10."; //RPi IP address, used for the MQTT server
                                  //will always be a 10. on ISU internet

 WiFiClient wifiClient;
 PubSubClient client(mqtt_server, 1883, wifiClient); 

//------------------------------------------------------


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
  //Serial.print(cyclesPerMicroSecond);
  //flagReadable = 0;
    
  while (1) {
    //int signal = analogRead(PIN_R);

    invalid = 0;

    if (xSemaphoreTake(semReadable, portMAX_DELAY) == pdTRUE) {

      //Serial.print("receive");
      //Serial.print(startTime);

      unsigned long initial =  esp_timer_get_time() - startTime;

      while(esp_timer_get_time() - initial < 200);//near field wait

      while(invalid == 0 &&  analogRead(PIN_R) < 800){ //mess with the minimum value 800 ~= 0.761 volts
        if (esp_timer_get_time() - initial > 7300)//too long distance to be valid
        {
          invalid = 1;
        }
      }

      afterTime = esp_timer_get_time();
      TOF = afterTime - startTime;

    
      if (invalid == 0){
        //Serial.print("here");
        distance = (TOF/2) * Vsound;

        Serial.print(angle);
        Serial.print(",");
        Serial.print((int)distance);
        Serial.print(".");

        message += String(angle) + "," + String(distance) + ".";
      }
      else{
        //express failed reading
        Serial.print(angle);
        Serial.print(",");
        Serial.print(0);
        Serial.print(".");
        message += String(0) + "," + String(0) + ".";
      }

      if(endCycle){
        //transmit string to Raspberry PI
        client.publish("esp32/sensor1", message.c_str());

        message = "";
        endCycle = false;
      }
      xSemaphoreGive(semScannable);
    }
  }
}

//-----------------------------------------------------------
void phasedTransmit(void *params){
  int delayBetween;
  int timePassed;
  float distance;
  //int angle;
  int right;
  //flagScannable = 1;
  
  //alternative loop with 9 sample points for testing
  //for(int i = 0; i < 9; i++){
    //int a = angle[i];
  //
  while(1){
    for(int a = -60; a <= 60; a++){
      delayBetween = calculatePhaseDelay(a); //time delay to use
      //timePassed = 0; // The amount of time that has passed (in microseconds) since the transmission
      distance = 0;

      if (xSemaphoreTake(semScannable, portMAX_DELAY) == pdTRUE) {
        //flagScannable = 0;
        //Serial.print("transmit");
        //Serial.print(angle);

        if(a == 60){
          endCycle = true;
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

        vTaskDelay(pdMS_TO_TICKS(9)); //delay to clear field for scanning.
        //Serial.print("afterScan");
      }
    }
  }
}

//---------------------------------------
void setup() {
  // put your setup code here, to run once:
  //pinMode(PIN_T, OUTPUT); // PIN_T as output

  // Set transmitter pins as OUTPUT 
  for (int i = 0; i < numTransmitters; i++) {
    pinMode(transmitterPins[i], OUTPUT);
    digitalWrite(transmitterPins[i], HIGH); // Ensure they start High (no pulsing)
  }

  // Set receiver pin as INPUT
  //pinMode(PIN_R, INPUT);
  analogReadResolution(12);  // Set ADC to 12-bit (0-4095)
  analogSetAttenuation(ADC_11db);  // Extend range to 0-3.3V  

  Serial.begin(115200);

  semReadable = xSemaphoreCreateBinary();
  semScannable = xSemaphoreCreateBinary();

  xTaskCreatePinnedToCore(phasedTransmit, "Transmit", 10000, 0, 2, &transmitterTaskHandle, 0); //Transmitter Task
  xTaskCreatePinnedToCore(TaskReceiver, "Receiver", 10000, 0, 2, &receiverTaskHandle, 1); //Receiver Task

  xSemaphoreGive(semScannable);
  
  //------------- MQTT setup
  connect_MQTT();
  client.setServer(mqtt_server,1883);//1883 is the default port for MQTT server
  client.setCallback(callback);

}

//---------------------------------------

void loop() {
  // put your main code here, to run repeatedly:
  taskYIELD();
}

//--------------------------------------------
//steering functions

//straight ahead steering (3 cycles pulse)
//straight ahead steering (3 cycles pulse)
void PulseStraight(){
  uint64_t start;

  REG_WRITE(GPIO_OUT1_W1TC_REG, (1 << (PIN_TR0 - 32)));
  REG_WRITE(GPIO_OUT1_W1TC_REG, (1 << (PIN_TR1 - 32)));
  REG_WRITE(GPIO_OUT1_W1TC_REG, (1 << (PIN_TR2 - 32)));
  xSemaphoreGive(semReadable);
  REG_WRITE(GPIO_OUT1_W1TC_REG, (1 << (PIN_TR3 - 32)));
  REG_WRITE(GPIO_OUT1_W1TC_REG, (1 << (PIN_TR4 - 32)));
  REG_WRITE(GPIO_OUT1_W1TC_REG, (1 << (PIN_TR5 - 32)));
  
  start = ESP.getCycleCount();
  while ((ESP.getCycleCount() - start) < (75 * 240));  // Wait 75 µs in cycles
  
  REG_WRITE(GPIO_OUT1_W1TS_REG, (1 << (PIN_TR0 - 32)));
  REG_WRITE(GPIO_OUT1_W1TS_REG, (1 << (PIN_TR1 - 32)));
  REG_WRITE(GPIO_OUT1_W1TS_REG, (1 << (PIN_TR2 - 32)));
  REG_WRITE(GPIO_OUT1_W1TS_REG, (1 << (PIN_TR3 - 32)));
  REG_WRITE(GPIO_OUT1_W1TS_REG, (1 << (PIN_TR4 - 32)));
  REG_WRITE(GPIO_OUT1_W1TS_REG, (1 << (PIN_TR5 - 32)));
  
  //delayMicroseconds(75);
}

//break into directions into different functions for efficency

// steer right
void beamSteerR(int timeDelay){
  const int numTransmitters = 6;
  uint64_t fireT[numTransmitters];
  uint64_t stopT[numTransmitters];
  int order[numTransmitters];

  // Calculate fire times and stop times
  for (int i = 0; i < numTransmitters; i++) {
    fireT[i] = ((int)(i * timeDelay) % 25) * cyclesPerMicroSecond;
    stopT[i] = fireT[i] + 75 * cyclesPerMicroSecond; // Stop time is 75us after fire time
    order[i] = i; // Initialize order array
  }

  // Sort the transmitters by fire time
  for (int i = 0; i < numTransmitters - 1; i++) {
    for (int j = i + 1; j < numTransmitters; j++) {
      if (fireT[order[j]] < fireT[order[i]]) {
        int temp = order[i];
        order[i] = order[j];
        order[j] = temp;
      }
    }
  }

  uint64_t current;
  uint64_t start = ESP.getCycleCount();

  int startNext = 0;
  int stopNext = 0;

  while (startNext < numTransmitters || stopNext < numTransmitters) {
    current = ESP.getCycleCount();

    // Fire transmitter at correct fire time
    if (startNext < numTransmitters) {
      int idx = order[startNext];
      if ((current - start) >= fireT[idx]) {
        REG_WRITE(GPIO_OUT1_W1TC_REG, (1 << (transmitterPins[idx] - 32))); // Fire (set LOW)
        startNext++;

        //After middle transmitter, give semaphore
        if ( idx == 3) {
          xSemaphoreGive(semReadable);
          startTime = esp_timer_get_time();
        }
      }
    }

    // Stop transmitter at correct stop time
    if (stopNext < numTransmitters) {
      int idx = order[stopNext];
      if ((current - start) >= stopT[idx]) {
        REG_WRITE(GPIO_OUT1_W1TS_REG, (1 << (transmitterPins[idx] - 32))); // Stop (set HIGH)
        stopNext++;
      }
    }
  }
}

// steer left
void beamSteerL(int timeDelay){
  const int numTransmitters = 6;
  uint64_t fireT[numTransmitters];
  uint64_t stopT[numTransmitters];
  int order[numTransmitters];

  // Initialize fire and stop times
  for (int i = 0; i < numTransmitters; i++) {
    fireT[i] = (((5 - i) * timeDelay) % 25) * cyclesPerMicroSecond;
    stopT[i] = fireT[i] + 75 * cyclesPerMicroSecond; // Stop 75us after fire
    order[i] = i; // Keep track of which transmitter this is
  }

  // Sort the transmitters based on fire times (ascending order)
  for (int i = 0; i < numTransmitters - 1; i++) {
    for (int j = i + 1; j < numTransmitters; j++) {
      if (fireT[order[j]] < fireT[order[i]]) {
        int temp = order[i];
        order[i] = order[j];
        order[j] = temp;
      }
    }
  }

  uint64_t current;
  uint64_t start = ESP.getCycleCount();

  int startNext = 0;
  int stopNext = 0;

  while (startNext < numTransmitters || stopNext < numTransmitters) {
    current = ESP.getCycleCount();

    // Start transmitter at correct fire time
    if (startNext < numTransmitters) {
      int idx = order[startNext];
      if ((current - start) >= fireT[idx]) {
        REG_WRITE(GPIO_OUT1_W1TC_REG, (1 << (transmitterPins[idx] - 32))); // Fire (LOW)
        startNext++;

        if (idx == 3) { // After 3 transmitters fired
          xSemaphoreGive(semReadable);
          startTime = esp_timer_get_time();
        }
      }
    }

    // Stop transmitter after its stop time
    if (stopNext < numTransmitters) {
      int idx = order[stopNext];
      if ((current - start) >= stopT[idx]) {
        REG_WRITE(GPIO_OUT1_W1TS_REG, (1 << (transmitterPins[idx] - 32))); // Stop (HIGH)
        stopNext++;
      }
    }
  }
}


void connect_MQTT(){

  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  //WiFi.begin("IASTATE", "password, if needed");
  //This might not be needed
  
  while (WiFi.status() != WL_CONNECTED) { //waits for a WiFi connection
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Attempting MQTT connection...");
  if(client.connect("ESP32_client1", ssid, password)) { //connects to mqtt server
    Serial.println("connected");
    client.subscribe("rpi/broadcast"); 
  } else {
     Serial.print("failed, rc-");
     Serial.print(client.state());
     Serial.println(" trying again in 2 seconds");

     delay(2000);
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Check if a message is received on the topic "rpi/broadcast"
  if (String(topic) == "rpi/broadcast") {
      if(messageTemp == "10"){
      }
  }

  //Similarly add more if statements to check for other subscribed topics 
}


