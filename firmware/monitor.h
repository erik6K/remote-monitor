#ifndef MONITOR_H
#define MONITOR_H

#include <Arduino.h>
#include <string.h>

/* Settings for FFT */
#define SAMPLES 512
#define FS 13392.857 // sample rate


class Monitor {
	public:
		Monitor(int main_sensor_pin, int battery_volts_pin);
		void Init();


		void sample_battery();
		float get_battery_volts();

		void take_mains_samples();
		bool adc_busy();

		void trig_ADC();

		bool add_sample(int smpl);
		int get_sample(int ind);

	private:

		void init_ADC_Pins();
		void init_ADC_Clock();
		void init_ADC_Freerun();

		int mains_samples[SAMPLES];
		int sample_index;
		bool adc_BUSY;

		int main_sensor_pin;
		int battery_volts_pin;

		int battery_value;

};
extern Monitor monitor;

#endif
