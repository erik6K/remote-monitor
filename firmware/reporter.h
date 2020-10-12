#ifndef REPORTER_H
#define REPORTER_H

#include <stdarg.h>
#include <time.h>
#include <SPI.h>

#include <assert.h>

#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <RTCZero.h>
#include <PubSubClient.h>

#include "configure.h"

// this is an easy to use NTP Arduino library by Stefan Staub - updates can be found here https://github.com/sstaub/NTP
#include "ntp.h"
#include "sha256.h"
#include "base64.h"
#include "utils.h"


#define AZURE_IOT_CENTRAL_DPS_ENDPOINT "global.azure-devices-provisioning.net"
#define TEMP_BUFFER_SIZE 1024
#define AUTH_BUFFER_SIZE 256

//#include "iotc_dps.h"

// MQTT publish topics
static const char PROGMEM IOT_EVENT_TOPIC[] = "devices/{device_id}/messages/events/";

class Reporter {
	public:
		void connectMQTT(String deviceId, String username, String password);
		String createIotHubSASToken(char *key, String url, long expire);

		Reporter();
		void Connect_Wifi();
		void Init();
		void report_data(int mains_status, int battery_voltage);


	private:
		void getTime();

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


		int getDPSAuthString(char* scopeId, char* deviceId, char* key, char *buffer, int bufferSize, size_t &outLength);

		int _getOperationId(char* scopeId, char* deviceId, char* authHeader, char *operationId);

		int _getHostName(char *scopeId, char*deviceId, char *authHeader, char*operationId, char* hostName);

		int getHubHostName(char *scopeId, char* deviceId, char* key, char *hostName);


};
extern Reporter reporter;

#endif