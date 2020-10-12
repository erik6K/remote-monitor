#ifndef REPORTER_H
#define REPORTER_H

#include <stdarg.h>
#include <time.h>
#include <SPI.h>

#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <RTCZero.h>
#include <PubSubClient.h>

/*
#include "src/WiFiNINA/src/WiFiNINA.h"
#include "src/WiFiNINA/src/WiFiUdp.h"
#include "src/RTCZero/src/RTCZero.h"
#include "src/PubSubClient/src/PubSubClient.h"
*/
#include "./configure.h"

// this is an easy to use NTP Arduino library by Stefan Staub - updates can be found here https://github.com/sstaub/NTP
#include "./ntp.h"
#include "./sha256.h"
#include "./base64.h"
#include "./utils.h"

// create a WiFi UDP object for NTP to use
WiFiUDP wifiUdp;
// create an NTP object
NTP ntp(wifiUdp);
// Create an rtc object
RTCZero rtc;

#include "./iotc_dps.h"

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


};
extern Reporter reporter;

#endif