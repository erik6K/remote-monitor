#include "reporter.h"

Reporter reporter;

// create a WiFi UDP object for NTP to use
WiFiUDP wifiUdp;
// create an NTP object
NTP ntp(wifiUdp);
// Create an rtc object
RTCZero rtc;

Reporter::Reporter() {

}

void Reporter::Connect_Wifi() {
  Serial.print("Connecting to WiFi...");
	int status = WL_IDLE_STATUS;
    while ( status != WL_CONNECTED) {
        //Serial_printf((char*)F("Attempting to connect to Wi-Fi SSID: %s \n"), wifi_ssid);
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
   
    // send telemetry values
    if (mqtt_client->connected()) {
        SerialUSB.println(F("Sending telemetry ..."));
        String topic = (String)IOT_EVENT_TOPIC;
        topic.replace(F("{device_id}"), deviceId);
        char buff[10];
        String payload = F("{\"MainsStatus\": \"{mains}\", \"BatteryVoltage\": {battery}}");
        payload.replace(F("{mains}"), mains_status ? "YES" : "NO");//);
        payload.replace(F("{battery}"), String(battery_avg));
        mqtt_client->publish(topic.c_str(), payload.c_str());
    } else { //MQTT Client has disconnected. Initiate reconnect.
      Connect_Wifi(); 
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


//-----------------------------------


int Reporter::getDPSAuthString(char* scopeId, char* deviceId, char* key, char *buffer, int bufferSize, size_t &outLength) {
  unsigned long expiresSecond = rtc.getEpoch() + 7200;
  assert(expiresSecond > 7200);

  String deviceIdEncoded = urlEncode(deviceId);
  char dataBuffer[AUTH_BUFFER_SIZE] = {0};
  size_t size = snprintf(dataBuffer, AUTH_BUFFER_SIZE, "%s%%2Fregistrations%%2F%s", scopeId, deviceIdEncoded.c_str());
  assert(size < AUTH_BUFFER_SIZE); dataBuffer[size] = 0;
  String sr = dataBuffer;
  size = snprintf(dataBuffer, AUTH_BUFFER_SIZE, "%s\n%lu000", sr.c_str(), expiresSecond);
  const size_t dataBufferLength = size;
  assert(dataBufferLength < AUTH_BUFFER_SIZE); dataBuffer[dataBufferLength] = 0;

  char keyDecoded[AUTH_BUFFER_SIZE] = {0};
  size = base64_decode(keyDecoded, key, strlen(key));
  assert(size < AUTH_BUFFER_SIZE && keyDecoded[size] == 0);
  const size_t keyDecodedLength = size;

  Sha256 *sha256 = new Sha256();
  sha256->initHmac((const uint8_t*)keyDecoded, (size_t)keyDecodedLength);
  sha256->print(dataBuffer);
  char* sign = (char*) sha256->resultHmac();
  int encodedSignLen = base64_enc_len(HASH_LENGTH);
  char encodedSign[encodedSignLen];
  base64_encode(encodedSign, sign, HASH_LENGTH);
  delete(sha256);

  String auth = urlEncode(encodedSign);
  outLength = snprintf(buffer, bufferSize, "authorization: SharedAccessSignature sr=%s&sig=%s&se=%lu000&skn=registration", sr.c_str(), auth.c_str(), expiresSecond);
  buffer[outLength] = 0;

  return 0;
}

int Reporter::_getOperationId(char* scopeId, char* deviceId, char* authHeader, char *operationId) {
  WiFiSSLClient client;
  if (client.connect(AZURE_IOT_CENTRAL_DPS_ENDPOINT, 443)) {
    char tmpBuffer[TEMP_BUFFER_SIZE] = {0};
    String deviceIdEncoded = urlEncode(deviceId);
    size_t size = snprintf(tmpBuffer, TEMP_BUFFER_SIZE,
      "PUT /%s/registrations/%s/register?api-version=2018-11-01 HTTP/1.0", scopeId, deviceIdEncoded.c_str());
    assert(size != 0); tmpBuffer[size] = 0;
    client.println(tmpBuffer);
    client.println("Host: global.azure-devices-provisioning.net");
    client.println("content-type: application/json; charset=utf-8");
    client.println("user-agent: iot-central-client/1.0");
    client.println("Accept: */*");
    size = snprintf(tmpBuffer, TEMP_BUFFER_SIZE,
      "{\"registrationId\":\"%s\"}", deviceId);
    assert(size != 0); tmpBuffer[size] = 0;
    String regMessage = tmpBuffer;
    size = snprintf(tmpBuffer, TEMP_BUFFER_SIZE,
      "Content-Length: %d", regMessage.length());
    assert(size != 0); tmpBuffer[size] = 0;
    client.println(tmpBuffer);

    client.println(authHeader);
    client.println("Connection: close");
    client.println();
    client.println(regMessage.c_str());

    delay(2000); // give 2 secs to server to process
    memset(tmpBuffer, 0, TEMP_BUFFER_SIZE);
    int index = 0;
    while (client.available() && index < TEMP_BUFFER_SIZE - 1) {
      tmpBuffer[index++] = client.read();
    }
    tmpBuffer[index] = 0;
    const char* operationIdString= "{\"operationId\":\"";
    index = indexOf(tmpBuffer, TEMP_BUFFER_SIZE, operationIdString, strlen(operationIdString), 0);
    if (index == -1) {
error_exit:
      Serial.println("ERROR: Error from DPS endpoint");
      Serial.println(tmpBuffer);
      return 1;
    } else {
      index += strlen(operationIdString);
      int index2 = indexOf(tmpBuffer, TEMP_BUFFER_SIZE, "\"", 1, index + 1);
      if (index2 == -1) goto error_exit;
      tmpBuffer[index2] = 0;
      strcpy(operationId, tmpBuffer + index);
      // Serial.print("OperationId:");
      // Serial.println(operationId);
      client.stop();
    }
  } else {
    Serial.println("ERROR: Couldn't connect AzureIOT DPS endpoint.");
    return 1;
  }

  return 0;
}

int Reporter::_getHostName(char *scopeId, char*deviceId, char *authHeader, char*operationId, char* hostName) {
  WiFiSSLClient client;
  if (!client.connect(AZURE_IOT_CENTRAL_DPS_ENDPOINT, 443)) {
    Serial.println("ERROR: DPS endpoint GET call has failed.");
    return 1;
  }
  char tmpBuffer[TEMP_BUFFER_SIZE] = {0};
  String deviceIdEncoded = urlEncode(deviceId);
  size_t size = snprintf(tmpBuffer, TEMP_BUFFER_SIZE,
    "GET /%s/registrations/%s/operations/%s?api-version=2018-11-01 HTTP/1.1", scopeId, deviceIdEncoded.c_str(), operationId);
  assert(size != 0); tmpBuffer[size] = 0;
  client.println(tmpBuffer);
  client.println("Host: global.azure-devices-provisioning.net");
  client.println("content-type: application/json; charset=utf-8");
  client.println("user-agent: iot-central-client/1.0");
  client.println("Accept: */*");
  client.println(authHeader);
  client.println("Connection: close");
  client.println();
  delay(5000); // give 5 secs to server to process
  memset(tmpBuffer, 0, TEMP_BUFFER_SIZE);
  int index = 0;
  while (client.available() && index < TEMP_BUFFER_SIZE - 1) {
    tmpBuffer[index++] = client.read();
  }
  tmpBuffer[index] = 0;
  const char* lookFor = "\"assignedHub\":\"";
  index = indexOf(tmpBuffer, TEMP_BUFFER_SIZE, lookFor, strlen(lookFor), 0);
  if (index == -1) {
    Serial.println("ERROR: couldn't get assignedHub. Trying again..");
    Serial.println(tmpBuffer);
    return 2;
  }
  index += strlen(lookFor);
  int index2 = indexOf(tmpBuffer, TEMP_BUFFER_SIZE, "\"", 1, index + 1);
  memcpy(hostName, tmpBuffer + index, index2 - index);
  hostName[index2-index] = 0;
  client.stop();
  return 0;
}

int Reporter::getHubHostName(char *scopeId, char* deviceId, char* key, char *hostName) {
  char authHeader[AUTH_BUFFER_SIZE] = {0};
  size_t size = 0;
  //Serial.println("- iotc.dps : getting auth...");
  if (getDPSAuthString(scopeId, deviceId, key, (char*)authHeader, AUTH_BUFFER_SIZE, size)) {
    Serial.println("ERROR: getDPSAuthString has failed");
    return 1;
  }
  //Serial.println("- iotc.dps : getting operation id...");
  char operationId[AUTH_BUFFER_SIZE] = {0};
  if (_getOperationId(scopeId, deviceId, authHeader, operationId) == 0) {
    delay(4000);
    //Serial.println("- iotc.dps : getting host name...");
    while( _getHostName(scopeId, deviceId, authHeader, operationId, hostName) == 2) delay(5000);
    return 0;
  }
}
