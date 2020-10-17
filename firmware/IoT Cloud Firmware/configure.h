// AZURE IOT CENTRAL
static char PROGMEM iotc_scopeId[] = "<SECRET>";
static char PROGMEM iotc_deviceId[] = "<SECRET>";
static char PROGMEM iotc_deviceKey[] = "<SECRET>";
static long timetolive = 864000; //Seconds before access key expires. Will reset when device restarts. 864000 = 10 days.

// WI-FI
static char PROGMEM wifi_ssid[] = "<SECRET>";
static char PROGMEM wifi_password[] = "<SECRET>";

// DEVICE
#define TELEMETRY_SEND_INTERVAL 30000 // telemetry data sent every 30000ms = 30sec
#define SENSOR_READ_INTERVAL 1000 //Read every 1000ms = 1 sec
