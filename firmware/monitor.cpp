
#include "monitor.h"

Monitor::Monitor(int main_sensor_pin, int battery_volts_pin) {

	this->main_sensor_pin = main_sensor_pin;
	this->battery_volts_pin = battery_volts_pin;

	battery_value = 0;
	main_value = 0;
 for (int i=0; i < MEMORY; i++) main_history[i]=false;
 
}

void Monitor::Init() {

	pinMode(main_sensor_pin, INPUT_PULLUP);
	pinMode(battery_volts_pin, INPUT);

}

void Monitor::read_values() {

	battery_value = analogRead(battery_volts_pin);
  
  //"Shuffle" mains historic values down queue
  for (int i=MEMORY-1; i>0; i--) {
    main_history[i] = main_history[i-1];
  }
  //Insert most recent value at front
  main_history[0] = digitalRead(main_sensor_pin);

  //FOR TROUBLE SHOOTING:
  /*SerialUSB.print("History: ");
  for (int i=0; i<MEMORY; i++) {
      SerialUSB.print(main_history[i]);SerialUSB.print("-");
  }*/
  
  //Update main_value only if historic values are steady
  for (int i=0; i<MEMORY-1; i++) {
    if (!main_history[i]) {
      main_value = false;
      break;
    }
    if (i==MEMORY-2) {
      main_value = true;
    }
  }
}

float Monitor::get_battery_volts() {
	return (float)battery_value / 1240.91;
}

String Monitor::get_main_status() {
	return main_value ? "OFF" : "ON";
}
