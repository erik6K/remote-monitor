
#include "reporter.h"

Reporter reporter;

Reporter::Reporter() {

}

void Reporter::Connect_Wifi() {
	int status = WL_IDLE_STATUS;
    while ( status != WL_CONNECTED) {
        Serial_printf((char*)F("Attempting to connect to Wi-Fi SSID: %s \n"), wifi_ssid);
        status = WiFi.begin(wifi_ssid, wifi_password);
        delay(1000);
    }
}

void Reporter::Init() {

    // get current UTC time
    getTime();

    SerialUSB.println("Getting IoT Hub host from Azure IoT DPS");
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

}

void Reporter::report_data(int mains_status, int battery_avg) {
    // give the MQTT handler time to do it's thing
    mqtt_client->loop(); 
    
    // send telemetry values
    if (mqtt_client->connected()) {
        SerialUSB.println(F("Sending telemetry ..."));
        String topic = (String)IOT_EVENT_TOPIC;
        topic.replace(F("{device_id}"), deviceId);
        char buff[10];
        String payload = F("{\"MainsStatus\": \"{mains}\", \"BatteryVoltage\": {battery}}");
        payload.replace(F("{mains}"), mains_status ? "YES" : "NO");//);
        payload.replace(F("{battery}"), String(battery_avg));
        Serial_printf("\t%s\n", payload.c_str());
        mqtt_client->publish(topic.c_str(), payload.c_str());
    }
}

void Reporter::connectMQTT(String deviceId, String username, String password) {
    mqtt_client->disconnect();

    SerialUSB.println(F("Starting IoT Hub connection"));
    int retry = 0;
    while(retry < 10 && !mqtt_client->connected()) {     
        if (mqtt_client->connect(deviceId.c_str(), username.c_str(), password.c_str())) {
                SerialUSB.println(F("===> mqtt connected"));
                mqttConnected = true;
        } else {
            SerialUSB.print(F("---> mqtt failed, rc="));
            SerialUSB.println(mqtt_client->state());
            delay(2000);
            retry++;
        }
    }
}

String Reporter::createIotHubSASToken(char *key, String url, long expire) {
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

    return (char*)F("SharedAccessSignature sr=") + url + (char*)F("&sig=") \
    + urlEncode((const char*)encodedSign) + (char*)F("&se=") + String(expire);
}

void Reporter::getTime() {
    SerialUSB.println(F("Getting the time from time service: "));

    ntp.begin();
    ntp.update();
    SerialUSB.print(F("Current time: "));
    SerialUSB.print(ntp.formattedTime("%d. %B %Y - "));
    SerialUSB.println(ntp.formattedTime("%A %T"));

    rtc.begin();
    rtc.setEpoch(ntp.epoch());
    timeSet = true;
}

