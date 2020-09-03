
#include "monitor.h"

Monitor::Monitor(int main_sensor_pin, int battery_volts_pin) {

	this->main_sensor_pin = main_sensor_pin;
	this->battery_volts_pin = battery_volts_pin;

	battery_value = 0;
	main_value = 0;

}

void Monitor::Init() {

	pinMode(main_sensor_pin, INPUT);
	pinMode(battery_volts_pin, INPUT);


}

void Monitor::read_values() {

	battery_value = analogRead(battery_volts_pin);
	main_value = digitalRead(main_sensor_pin);

}

float Monitor::get_battery_volts() {
	return (float)battery_value / 1240.91;
}

bool Monitor::get_main_status() {
	return main_value ? true : false;
}