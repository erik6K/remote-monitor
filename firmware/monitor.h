
#ifndef MONITOR_H
#define MONITOR_H

#include <Arduino.h>


class Monitor {
	public:
		Monitor(int main_sensor_pin, int battery_volts_pin);

		int poll_main_sensor();
		float get_battery_volts();

	private:

		int main_sensor_pin;
		int battery_volts_pin;

		bool main_power_on;
		float battery_voltage;
}


#endif