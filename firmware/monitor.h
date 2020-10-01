
#ifndef MONITOR_H
#define MONITOR_H

#include <Arduino.h>
#include <string.h>




class Monitor {
	public:
		Monitor(int main_sensor_pin, int battery_volts_pin);
		void Init();

		void read_values();

		String get_main_status();
		float get_battery_volts();

		void trig_ADC();

	private:

		void init_ADC();

		int main_sensor_pin;
		int battery_volts_pin;

		bool main_value;
		int battery_value;

};


#endif
