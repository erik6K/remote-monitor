#ifndef MONITOR_H
#define MONITOR_H

#include <Arduino.h>
#include <WiFiNINA.h>
#include <utility/wifi_drv.h>
#include "src/Adafruit_Zero_FFT_Library/Adafruit_ZeroFFT.h"
#include "defs.h"

/* Settings for FFT */
#define SAMPLES 1024
#define FS 13392.857 // sample rate

enum ADC_State { FREERUN, SINGLE };

class Monitor {
	public:
		Monitor();
		void Init();

		void take_battery_sample();
		void take_mains_samples();

		bool adc_busy();
		void trig_ADC();

		bool add_mains_sample(int smpl);
		int get_mains_sample(int ind);
		void record_battery_sample(int smpl);

		void remove_DC();
		void compute_fft();
		void verify_50Hz();

		void save_min_battery();

		float get_battery_volts();
		float get_latest_battery_volts();
		int get_mains_status();
		int get_latest_mains_status();

		ADC_State adc_state;

	private:

		void init_ADC_Pins();
		void init_ADC_Clock();
		void init_ADC_Freerun();
		void init_ADC_Single();

		int16_t mains_samples[SAMPLES];
		int next_mains_samp;

		uint8_t mains_status[NUM_CYCLES_B4_REPORT] = {0};
		uint8_t next_mains_stat;

		uint16_t battery_samples[NUM_BATT_SAMPLES];
		uint8_t next_battery_samp;

		uint16_t battery_min_values[NUM_CYCLES_B4_REPORT] = {0};
		uint8_t next_battery_min;

		bool adc_BUSY;

};
extern Monitor monitor;

#endif
