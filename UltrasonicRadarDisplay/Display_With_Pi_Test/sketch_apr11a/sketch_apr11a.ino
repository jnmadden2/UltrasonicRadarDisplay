#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_wifi.h"


String message = "";
int cycle = 5;

//------------Raspberry PI variables-------------------
 const char *MQTTssid = "sdmay25-36"; //name of the MQTT server
 const char *MQTTpassword = "password"; //password for the MQTT server
 const char* mqtt_server = "10.29.183.189"; //RPi IP address, used for the MQTT server
                                  //will always be a 10. on ISU internet
                                  //
 WiFiClient wifiClient;
 PubSubClient client(mqtt_server, 1883, wifiClient); 
 char ssid[] = "IASTATE";
 IPAddress ip;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //------------- MQTT setup
  connect_MQTT();
  client.setServer(mqtt_server,1883);//1883 is the default port for MQTT server
  client.setCallback(callback);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Serial.print(random(60,120));
  // // Serial.print(90);
  // Serial.print(",");
  // Serial.print(random(0,1000));
  // // Serial.print(0);
  // Serial.print(".");

//  message += String(random(60,120)) + "," + String(random(0,1000)) + ".";
//  cycle++;
  
  message = String(0,0.);
  client.publish("esp32/sensor1", message.c_str());

  
  delay(5000);
//  if(cycle >= 5){
//    String mystring(message); 
//    //transmit string to Raspberry PI message.c_str()
//    client.publish("esp32/sensor1", message.c_str());
//    Serial.println("data sent");
//    Serial.println(message);

message = String("11,495.12,498.10,493.");
client.publish("esp32/sensor1", message.c_str());
    Serial.println("data sent");
    Serial.println(message);
    message = "";
    delay(1000);

  message = String("45,510.46,514.44,507.");
  client.publish("esp32/sensor1", message.c_str());
    Serial.println("data sent");
    Serial.println(message);
      delay(1000);
      message = "";


message = String("68,480.69,483.67,477.");
client.publish("esp32/sensor1", message.c_str());
    Serial.println("data sent");
    Serial.println(message);
        message = "";
  delay(1000);

message = String("83,495.82,491.84,497");
client.publish("esp32/sensor1", message.c_str());
    Serial.println("data sent");
    Serial.println(message);
        message = "";
  delay(1000);

      
message = String("120,505.121,508.119,502");
client.publish("esp32/sensor1", message.c_str());
    Serial.println("data sent");
    Serial.println(message);
        message = "";
  delay(1000);

message = String("136,490.137,494.135,487.");
client.publish("esp32/sensor1", message.c_str());
    Serial.println("data sent");
    Serial.println(message);
        message = "";
  delay(1000);

message = String("150,510.151,513.149,507.");
client.publish("esp32/sensor1", message.c_str());
    Serial.println("data sent");
    Serial.println(message);
        message = "";
  delay(1000);

message = String("165,495.166,498.164,492.");
client.publish("esp32/sensor1", message.c_str());
    Serial.println("data sent");
    Serial.println(message);
    message = "";
  delay(1000);

message = String("175,485.176,488.174,482.");
client.publish("esp32/sensor1", message.c_str());
    Serial.println("data sent");
    Serial.println(message);
    message = "";

message = String("179,500.178,503.180,497.");
client.publish("esp32/sensor1", message.c_str());
    Serial.println("data sent");
    Serial.println(message);
    message = "";

    
//    cycle = 0;
//  }
  delay(5000);
}

void connect_MQTT(){
  Serial.println("Resetting Wi-Fi stack...");
  WiFi.disconnect(true, true);
//  ESP.restart();
  esp_wifi_stop();
  esp_wifi_deinit();
  
  delay(500);

  WiFi.mode(WIFI_STA);
  ip = WiFi.localIP();
  Serial.println(ip);

  // force a new MAC address to avoid lease cache 24:ec:4a:09:c9:71
//  uint8_t newMAC[6] = { 0x24, 0xEC, 0x4a, 0x09, 0xC9, 0x7 }; //(uint8_t)random(0, 255) };
//  esp_wifi_set_mac(WIFI_IF_STA,newMAC );

  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin("IASTATE");
  //This might not be needed
  
//  while (WiFi.localIP() == ip) { //waits for a WiFi connection
//    delay(500);
//    Serial.print(".");
//  }
  
    unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    Serial.print(".");
    delay(1000);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect via DHCP.");
    Serial.print("Status: ");
    Serial.println(WiFi.status());
    connect_MQTT();
    return;
  }


  Serial.print("Attempting MQTT connection...");
  if(client.connect("ESP32_client1", MQTTssid, MQTTpassword)) { //connects to mqtt server
    Serial.println("connected");
    client.subscribe("rpi/broadcast");
    client.publish("esp32/sensor1", "100,100"); 
  } else {
     Serial.print("failed, rc-");
     Serial.print(client.state());
     Serial.println(" trying again in 2 seconds");

     delay(2000);
  }

}

void callback(char* topic, byte* message, unsigned int length) {
}
