#ifndef MONITOR_H
#define MONITOR_H

#include <Arduino.h>
#include "src/Adafruit_Zero_FFT_Library/Adafruit_ZeroFFT.h"

/* Settings for FFT */
#define SAMPLES 1024
#define FS 13392.857 // sample rate

enum ADC_State { FREERUN, SINGLE };

class Monitor {
	public:
		Monitor();
		void Init();

		void take_battery_sample();
		float get_battery_volts();

		void take_mains_samples();
		bool adc_busy();

		void trig_ADC();

		bool add_mains_sample(int smpl);
		int get_mains_sample(int ind);
		void record_battery_sample(int smpl);

		void remove_DC();
		void compute_fft();
		int verify_50Hz();

		ADC_State adc_state;

	private:

		void init_ADC_Pins();
		void init_ADC_Clock();
		void init_ADC_Freerun();
		void init_ADC_Single();


		int16_t mains_samples[SAMPLES];
		

		int sample_index;
		bool adc_BUSY;

		volatile int battery_value;

};
extern Monitor monitor;

#endif
