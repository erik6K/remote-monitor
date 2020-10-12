/*
// Read temperature and humidity data from an Arduino MKR1000 or MKR1010 device using a DHT11/DHT22 sensor.
// The data is then sent to Azure IoT Central for visualizing via MQTT
//
// See the readme.md for details on connecting the sensor and setting up Azure IoT Central to recieve the data.
*/

#include <stdarg.h>
#include <time.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <RTCZero.h>
#include <PubSubClient.h>

#include "./configure.h"

// this is an easy to use NTP Arduino library by Stefan Staub - updates can be found here https://github.com/sstaub/NTP
#include "./ntp.h"
#include "./sha256.h"
#include "./base64.h"
#include "./utils.h"

int getHubHostName(char *scopeId, char* deviceId, char* key, char *hostName);

String iothubHost;
String deviceId;
String sharedAccessKey;

WiFiSSLClient wifiClient;
PubSubClient *mqtt_client = NULL;

bool timeSet = false;
bool wifiConnected = false;
bool mqttConnected = false;

time_t this_second = 0;
time_t checkTime = 1300000000;

long lastTelemetryMillis = 0;
long lastSensorReadMillis = 0;

String mainsStatus = "OFF";
float batteryVoltage = 0.0;

// MQTT publish topics
static const char PROGMEM IOT_EVENT_TOPIC[] = "devices/{device_id}/messages/events/";

// create a WiFi UDP object for NTP to use
WiFiUDP wifiUdp;
// create an NTP object
NTP ntp(wifiUdp);
// Create an rtc object
RTCZero rtc;

#include "./iotc_dps.h"

// get the time from NTP and set the real-time clock on the MKR10x0
void getTime() {
    Serial.println(F("Getting the time from time service: "));

    ntp.begin();
    ntp.update();
    Serial.print(F("Current time: "));
    Serial.print(ntp.formattedTime("%d. %B %Y - "));
    Serial.println(ntp.formattedTime("%A %T"));

    rtc.begin();
    rtc.setEpoch(ntp.epoch());
    timeSet = true;
} 

// connect to Azure IoT Hub via MQTT
void connectMQTT(String deviceId, String username, String password) {
    mqtt_client->disconnect();

    Serial.println(F("Starting IoT Hub connection"));
    int retry = 0;
    while(retry < 10 && !mqtt_client->connected()) {     
        if (mqtt_client->connect(deviceId.c_str(), username.c_str(), password.c_str())) {
                Serial.println(F("===> mqtt connected"));
                mqttConnected = true;
        } else {
            Serial.print(F("---> mqtt failed, rc="));
            Serial.println(mqtt_client->state());
            delay(2000);
            retry++;
        }
    }
}

// create an IoT Hub SAS token for authentication
String createIotHubSASToken(char *key, String url, long expire){
    url.toLowerCase();
    String stringToSign = url + "\n" + String(expire);
    int keyLength = strlen(key);

    int decodedKeyLength = base64_dec_len(key, keyLength);
    char decodedKey[decodedKeyLength];

    base64_decode(decodedKey, key, keyLength);

    Sha256 *sha256 = new Sha256();
    sha256->initHmac((const uint8_t*)decodedKey, (size_t)decodedKeyLength);
    sha256->print(stringToSign);
    char* sign = (char*) sha256->resultHmac();
    int encodedSignLen = base64_enc_len(HASH_LENGTH);
    char encodedSign[encodedSignLen];
    base64_encode(encodedSign, sign, HASH_LENGTH);
    delete(sha256);

    return (char*)F("SharedAccessSignature sr=") + url + (char*)F("&sig=") + urlEncode((const char*)encodedSign) + (char*)F("&se=") + String(expire);
}

// Generates random values for testing
void readSensors() {
    if (mainsStatus == "OFF") {
      mainsStatus = "ON";
      batteryVoltage = 14.0;
    }
    else {
      mainsStatus = "OFF";
      batteryVoltage = 12.0;
    }
    
}

void setup() {
    Serial.begin(115200);
    delay(2000); //Give serial monitor time to start
 
    Serial_printf((char*)F("Hello, starting up the %s device\n"), "Arduino MKR1010");

    // attempt to connect to Wifi network:
    Serial.print((char*)F("WiFi Firmware version is "));
    Serial.println(WiFi.firmwareVersion());
    int status = WL_IDLE_STATUS;
    while ( status != WL_CONNECTED) {
        Serial_printf((char*)F("Attempting to connect to Wi-Fi SSID: %s \n"), wifi_ssid);
        status = WiFi.begin(wifi_ssid, wifi_password);
        delay(1000);
    }

    // get current UTC time
    getTime();

    Serial.println("Getting IoT Hub host from Azure IoT DPS");
    deviceId = iotc_deviceId;
    sharedAccessKey = iotc_deviceKey;
    char hostName[64] = {0};
    getHubHostName((char*)iotc_scopeId, (char*)iotc_deviceId, (char*)iotc_deviceKey, hostName);
    iothubHost = hostName;

    // create SAS token and user name for connecting to MQTT broker
    String url = iothubHost + urlEncode(String((char*)F("/devices/") + deviceId).c_str());
    char *devKey = (char *)sharedAccessKey.c_str();
    long expire = rtc.getEpoch() + timetolive;
    String sasToken = createIotHubSASToken(devKey, url, expire);
    String username = iothubHost + "/" + deviceId + (char*)F("/api-version=2016-11-14");

    // connect to the IoT Hub MQTT broker
    wifiClient.connect(iothubHost.c_str(), 8883);
    mqtt_client = new PubSubClient(iothubHost.c_str(), 8883, wifiClient);
    connectMQTT(deviceId, username, sasToken);

    // initialize timers
    lastTelemetryMillis = millis();
}

// main processing loop
void loop() {
    // give the MQTT handler time to do it's thing
    mqtt_client->loop(); 
    
    // send telemetry values
    if (mqtt_client->connected() && millis() - lastTelemetryMillis > TELEMETRY_SEND_INTERVAL) {
        readSensors();
        Serial.println(F("Sending telemetry ..."));
        String topic = (String)IOT_EVENT_TOPIC;
        topic.replace(F("{device_id}"), deviceId);
        char buff[10];
        String payload = F("{\"MainsStatus\": \"{mains}\", \"BatteryVoltage\": {battery}}");
        payload.replace(F("{mains}"), mainsStatus);//);
        payload.replace(F("{battery}"), String(batteryVoltage));
        Serial_printf("\t%s\n", payload.c_str());
        mqtt_client->publish(topic.c_str(), payload.c_str());

        lastTelemetryMillis = millis();
    }
}
