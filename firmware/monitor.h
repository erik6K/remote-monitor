
#ifndef MONITOR_H
#define MONITOR_H

#include <Arduino.h>


class Monitor {
	public:
		Monitor(int main_sensor_pin, int battery_volts_pin);
		void Init();

		void read_values();

		bool get_main_status();
		float get_battery_volts();

	private:

		int main_sensor_pin;
		int battery_volts_pin;

		int main_value;
		int battery_value;
};


#endif
