#ifndef CONFIGURE_H
#define CONFIGURE_H

// AZURE IOT CENTRAL
#define IOTC_SCOPEID "0ne0018A252"
#define IOTC_DEVICEID "deviceA"

#define IOTC_DEVICEKEY "z0bzbWP4PFBZtmt9RSyqHTeoOeuJZchkWWn70baaM0I="

#define TIMETOLIVE 864000 //Seconds before access key expires. Will reset when device restarts. 864000 = 10 days.

// WI-FI -- Note: leaving these empty causes program to hang
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#endif