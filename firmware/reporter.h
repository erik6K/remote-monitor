#ifndef REPORTER_H
#define REPORTER_H

#include <Arduino.h>
#include <SPI.h>

#include <stdarg.h>
#include <time.h>
#include <assert.h>

#include <WiFiNINA.h>
#include <utility/wifi_drv.h>
#include <WiFiUdp.h>
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

// MQTT publish topics
static const char PROGMEM IOT_EVENT_TOPIC[] = "devices/{device_id}/messages/events/";

class Reporter {
	public:
		Reporter();

		void Init();
		void report_data(int mains_status, float battery_min);

		void mqtt_loop();

	private:
		bool connect_wifi();
		void try_reconnect();

		void set_RGB_LED(uint8_t r, uint8_t g, uint8_t b);

		struct IoTDetails {
			String deviceId;
			String iothubHost;
			String sasToken;
			String username;
		} iotc;

		WiFiSSLClient wifiClient;
		PubSubClient *mqtt_client = NULL;

		enum Reporter_STATE { MQTT, RADIO_ONLY } reporter_state;


		bool connectMQTT(String deviceId, String username, String password);
		String createIotHubSASToken(const char *key, String url, long expire);

		int getDPSAuthString(const char* scopeId, const char* deviceId, const char* key, char *buffer, int bufferSize, size_t &outLength);
		int _getOperationId(const char* scopeId, const char* deviceId, char* authHeader, char *operationId);
		int _getHostName(const char *scopeId, const char*deviceId, char *authHeader, char*operationId, char* hostName);
		int getHubHostName(const char *scopeId, const char* deviceId, const char* key, char *hostName);

};
extern Reporter reporter;

#endif
