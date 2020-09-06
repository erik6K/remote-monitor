/*
  AWS IoT WiFi

  This sketch securely connects to an AWS IoT using MQTT over WiFi.
  It uses a private key stored in the ATECC508A and a public
  certificate for SSL/TLS authetication.

  It publishes a message every 5 seconds to arduino/outgoing
  topic and subscribes to messages on the arduino/incoming
  topic.

  The circuit:
  - Arduino MKR WiFi 1010 or MKR1000

  The following tutorial on Arduino Project Hub can be used
  to setup your AWS account and the MKR board:

  https://create.arduino.cc/projecthub/132016/securely-connecting-an-arduino-mkr-wifi-1010-to-aws-iot-core-a9f365

  This example code is in the public domain.
*/

#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h> // change to #include <WiFi101.h> for MKR1000
#include <string.h>


#include "arduino_secrets.h"

/////// Enter your sensitive data in arduino_secrets.h
const char ssid[]        = SECRET_SSID;
const char pass[]        = SECRET_PASS;
const char broker[]      = SECRET_BROKER; //endpoint
const char* certificate  = SECRET_CERTIFICATE;
String thing_name    = THING_NAME;
String shadow_name   = SHADOW_NAME;
String pubTopic = "$aws/things/"+thing_name+"/shadow/name/"+shadow_name+"/update"; //Path to send shadow updates

//Initialise requied class instances
WiFiClient    wifiClient;            // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECC508
MqttClient    mqttClient(sslClient);

//Initialise variables 
int lastMillis = millis();
const int sensorPin = A1;
float sensorValue = 0;
const int buttonPin = 1;
float battery_voltage = 0;
bool mains_status = true;


void setup() {
  Serial.begin(115200);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500); 
  
  pinMode(buttonPin, INPUT);

  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1);
  }

  // Set a callback to get the current time
  // used to validate the servers certificate
  ArduinoBearSSL.onGetTime(getTime);

  // Set the ECCX08 slot to use for the private key
  // and the accompanying public certificate for it
  sslClient.setEccSlot(0, certificate);


  //NOT USED - COULD BE USED IN FUTURE
  // Set the message callback, this function is
  // called when the MQTTClient receives a message
  //mqttClient.onMessage(onMessageReceived);
}

void loop() {
  //Connect (or re-connect) to WIFI
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  //Connnect (or re-connec) to MQTT Client
  if (!mqttClient.connected()) {
    // MQTT client is disconnected, connect
    connectMQTT();
  }

  //NOT USED - COULD BE USED IN FUTURE
  //poll for new MQTT messages and send keep alives
  //mqttClient.poll();

  // publish a message roughly every 1 seconds.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    sensorValue = analogRead(sensorPin);
    battery_voltage = sensorValue*15/1000;
    mains_status = digitalRead(buttonPin);
    publish_all(battery_voltage, mains_status);
    lastMillis =millis();
  }
}

unsigned long getTime() {
  // get the current time from the WiFi module  
  return WiFi.getTime();
}

void connectWiFi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.print(ssid);
  Serial.print(" ");

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the network");
  Serial.println();
}

void connectMQTT() {
  Serial.print("Attempting to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");

  while (!mqttClient.connect(broker, 8883)) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe("arduino/incoming");
}


//Function used to publish the most recent battery and voltages:
void publish_all(float battery_voltage, bool mains_status) {
  Serial.println("Publishing message");

  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage(pubTopic);
  mqttClient.print(R"(
  {
  "state": {
    "reported": {
      "battery_voltage": ")");
      mqttClient.print(battery_voltage);
      mqttClient.print(R"(",
      "mains_status": ")");
      mqttClient.print(mains_status);
      mqttClient.print(R"("
    }
  }
}
  )");
  
  mqttClient.endMessage();
}


//Function not mornally used:
void onMessageReceived(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
  }
  Serial.println();

  Serial.println();
}
