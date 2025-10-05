//#include <sys/wait.h>
//
//float Vsound = 0.343; // Speed of sound (mm/micro s)
//float distanceMult = 0.65;
//
//const int PIN_T = 15;
//const int PIN_R = 7;
//
//float lamda = 0.008575; // Wavelength of a 40 kHz sound wave
//
//int beginT, endT, timePassed, whileTime;
//
//int distance;
//
//
//void setup() {
//  // put your setup code here, to run once:
//  pinMode(PIN_T, OUTPUT); // PIN_T as output
//  Serial.begin(115200);
//}
//
//void loop() {
//  // put your main code here, to run repeatedly:
//
//  bool cancelRead = false; // Returns true if we've gone 10 ms without receiving anything (~3.5 meters)
//
//  int returnVal = 0; // AnalogRead(0)
//  float distance = 0; // Distance to be calculated (in mm)
//  int temp = PIN_T;
//  pulse();
////  while(wait(NULL) > 0); // there is no wait command :(
//  delay(100); //going to try a 100ms dealy, this should prevent any issues from multi-threading
//
//  beginT = micros();
//  whileTime = micros();
//
//  while (analogRead(PIN_R) < 800 && !cancelRead) //updated to use right pin input, and updated threshold (this may need additional changes)
//  {
//
//    if (micros() - whileTime > 10000) {
//      cancelRead = true;
//    }
//
//    returnVal = analogRead(A0);
//
//  }
//
//  endT = micros();
//
//  timePassed = endT - beginT;
//
//  if (!cancelRead){
//    distance = (timePassed/2) * Vsound;
//
//    //int angle = (theta / 180 * PI);
//    Serial.print(90);
//    Serial.print(",");
//    Serial.print((int)distance);
//    Serial.print(".");
//  }
//  
////  delay(200);
//}
//
//void pulse(){
//  digitalWrite(PIN_T, HIGH); // Turn on pin 13
//  delay(13);            // Wait for 13 micro sec (update)
//  digitalWrite(PIN_T, LOW);  // Turn off pin 13
//  delay(13);            // Wait for 13 micro sec (update)
//  
//}
