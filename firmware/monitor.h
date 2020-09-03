
#ifndef MONITOR_H
#define MONITOR_H

#include <Arduino.h>
#include <string.h>
#define MEMORY 10


class Monitor {
	public:
		Monitor(int main_sensor_pin, int battery_volts_pin);
		void Init();

		void read_values();

		String get_main_status();
		float get_battery_volts();

	private:

		int main_sensor_pin;
		int battery_volts_pin;

		bool main_value;
		int battery_value;
		bool main_history[MEMORY];

};


#endif
