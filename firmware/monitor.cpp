#include "monitor.h"

Monitor monitor;

Monitor::Monitor() {

}

void Monitor::Init() {

	// mains indicator led pin init
	pinMode(LED_BUILTIN, OUTPUT);

	init_ADC_Pins();
	init_ADC_Clock();

	// init indexes
	next_mains_stat = 0;
	next_battery_samp = 0;
	next_battery_min = 0;
}

void Monitor::init_ADC_Pins() {

	PORT->Group[PORTB].DIRCLR.reg |= PORT_PB02 | PORT_PB03; // set pins A1 and A2 as input

	PORT->Group[PORTB].PINCFG[2].bit.PMUXEN = 1;
	PORT->Group[PORTB].PINCFG[3].bit.PMUXEN = 1;

	// PORTB(2*[1]) = PORTB02, PORTB(2*[1] + 1) = PORTB03
	PORT->Group[PORTB].PMUX[1].reg = PORT_PMUX_PMUXE_B | PORT_PMUX_PMUXO_B;
}

void Monitor::init_ADC_Clock() {

	// clock
	REG_PM_APBCMASK |= PM_APBCMASK_ADC;

	REG_GCLK_GENDIV |= GCLK_GENDIV_DIV(1) | GCLK_GENDIV_ID(3);
	while (GCLK->STATUS.bit.SYNCBUSY);

	REG_GCLK_GENCTRL |= GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_ID(3);
	while (GCLK->STATUS.bit.SYNCBUSY);

	REG_GCLK_CLKCTRL |= GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(3) | GCLK_CLKCTRL_ID(30); // 30 is ADC
	while (GCLK->STATUS.bit.SYNCBUSY);
}

void Monitor::init_ADC_Freerun() {

	REG_ADC_CTRLA |= ADC_CTRLA_SWRST; // reset adc
	while (ADC->STATUS.bit.SYNCBUSY);

	/* 12 bits conversion takes 7 clock cycles 
	   48MHz / (7 * 512) = 13392.857 Hz Fs     */

	REG_ADC_REFCTRL = ADC_REFCTRL_REFSEL_AREFA;

	REG_ADC_AVGCTRL |= ADC_AVGCTRL_SAMPLENUM_1;

	REG_ADC_SAMPCTRL = ADC_SAMPCTRL_SAMPLEN(0);

	// Pin A1 on arduno corresponds to PB02 and AIN10
	REG_ADC_INPUTCTRL = ADC_INPUTCTRL_GAIN_1X | ADC_INPUTCTRL_MUXNEG_GND | ADC_INPUTCTRL_MUXPOS_PIN10;
	while (ADC->STATUS.bit.SYNCBUSY);

	REG_ADC_CTRLB = ADC_CTRLB_RESSEL_12BIT | ADC_CTRLB_PRESCALER_DIV512 | ADC_CTRLB_FREERUN; // Fs = 13392.857
	while (ADC->STATUS.bit.SYNCBUSY);

	REG_ADC_WINCTRL = ADC_WINCTRL_WINMODE_DISABLE;
	while (ADC->STATUS.bit.SYNCBUSY);

	REG_ADC_EVCTRL |= ADC_EVCTRL_STARTEI;
	while (ADC->STATUS.bit.SYNCBUSY);

	REG_ADC_CTRLA |= ADC_CTRLA_ENABLE;
	while (ADC->STATUS.bit.SYNCBUSY);

	// interrupt setup
	REG_ADC_INTENSET |= ADC_INTENSET_RESRDY; // result ready int
	while (ADC->STATUS.bit.SYNCBUSY);

	NVIC_EnableIRQ(ADC_IRQn); // enable adc interrupts
	NVIC_SetPriority(ADC_IRQn, 0); // highest priority

	adc_state = FREERUN;
}

void Monitor::init_ADC_Single() {

	REG_ADC_CTRLA |= ADC_CTRLA_SWRST; // reset adc
	while (ADC->STATUS.bit.SYNCBUSY);

	REG_ADC_REFCTRL = ADC_REFCTRL_REFSEL_AREFA;

	REG_ADC_AVGCTRL |= ADC_AVGCTRL_SAMPLENUM_1;

	REG_ADC_SAMPCTRL = ADC_SAMPCTRL_SAMPLEN(0);

	// Pin A1 on arduino corresponds to PB02 and AIN10
	REG_ADC_INPUTCTRL = ADC_INPUTCTRL_GAIN_1X | ADC_INPUTCTRL_MUXNEG_GND | ADC_INPUTCTRL_MUXPOS_PIN11;
	while (ADC->STATUS.bit.SYNCBUSY);

	REG_ADC_CTRLB = ADC_CTRLB_RESSEL_12BIT | ADC_CTRLB_PRESCALER_DIV512;
	while (ADC->STATUS.bit.SYNCBUSY);

	REG_ADC_WINCTRL = ADC_WINCTRL_WINMODE_DISABLE;
	while (ADC->STATUS.bit.SYNCBUSY);

	REG_ADC_EVCTRL |= ADC_EVCTRL_STARTEI;
	while (ADC->STATUS.bit.SYNCBUSY);

	REG_ADC_CTRLA |= ADC_CTRLA_ENABLE;
	while (ADC->STATUS.bit.SYNCBUSY);

	// interrupt setup
	REG_ADC_INTENSET |= ADC_INTENSET_RESRDY; // result ready int
	while (ADC->STATUS.bit.SYNCBUSY);

	NVIC_EnableIRQ(ADC_IRQn); // enable adc interrupts
	NVIC_SetPriority(ADC_IRQn, 0); // highest priority

	adc_state = SINGLE;
}

void Monitor::remove_DC() {
	int min = 4096;
	int max = 0;

	for (int i = 0; i < SAMPLES; i++) {
		if (mains_samples[i] < min) min = mains_samples[i];
		if (mains_samples[i] > max) max = mains_samples[i];
	}
	
	// print the minimum and maximum of the signal sampled for debugging
	//SerialUSB.print("Min: "); SerialUSB.println(min);
	//SerialUSB.print("Max: "); SerialUSB.println(max);
	
	int offset = ((max - min) / 2) + min;

	for (int i = 0; i < SAMPLES; i++) {
		mains_samples[i] -= offset;
	}
}

void Monitor::compute_fft() {
	ZeroFFT(mains_samples, SAMPLES);
}

void Monitor::verify_50Hz() {

	uint8_t histogram[256] = {0}; // histogram for noise floor detection

	// bins 3 and 4 correspond to roughly ~35 and ~51 Hz, so get an average between them
	int mag_50Hz = (mains_samples[3] + mains_samples[4]) / 2;

	// 
	for (int i = 0; i < (SAMPLES >> 1); i++) {
		int mag = min(mains_samples[i], 255);
		histogram[mag] += 1;
	}


	int maxh = 0;
	int noise_floor = 0;

	// element that contains max value in histogram is the noise floor
	for (int i = 0; i < 256; i++) {
		if (maxh < histogram[i]) {
			maxh = histogram[i];
			noise_floor = i + 1; // +1 to prevent noise floor of 0
		}
	}

	// print the magnitude of 50Hz and the noise floor for debugging
	//SerialUSB.print("50HZ: "); SerialUSB.println(mag_50Hz);
	//SerialUSB.print("noise floor: "); SerialUSB.println(noise_floor);

	// check if 50Hz magnitude is at least 5 times larger than noise floor
	mains_status[next_mains_stat] = (mag_50Hz > noise_floor*5 ? 1 : 0);

	// Set LED if 50Hz is detected
	digitalWrite(LED_BUILTIN, mains_status[next_mains_stat] ? HIGH : LOW);

	// increment index for average
	next_mains_stat = (next_mains_stat + 1) % NUM_CYCLES_B4_REPORT;
}

bool Monitor::add_mains_sample(int smpl) {

	mains_samples[next_mains_samp] = smpl;
	next_mains_samp++;

	if (next_mains_samp == SAMPLES) {
		adc_BUSY = false;
		return false;
	}
	else return true;
}

void Monitor::record_battery_sample(int smpl) {

	battery_samples[next_battery_samp] = smpl;

	// increment index
	next_battery_samp = (next_battery_samp + 1) % NUM_BATT_SAMPLES;
}

int Monitor::get_mains_sample(int ind) {
	return mains_samples[ind];
}

void Monitor::take_mains_samples() {

	adc_BUSY = true;
	next_mains_samp = 0;
	init_ADC_Freerun();

	trig_ADC();
}

bool Monitor::adc_busy() {
	return adc_BUSY;
}

void Monitor::trig_ADC() {
	REG_ADC_SWTRIG |= ADC_SWTRIG_START;
}

void Monitor::take_battery_sample() {
	if (adc_state != SINGLE) {
		init_ADC_Single();
	}
	trig_ADC();
}

/* find and save min value for averaging */
void Monitor::save_min_battery() {
	int min = 4096;

	for (int i = 0; i < NUM_BATT_SAMPLES; i++) {
		if (battery_samples[i] < min) min = battery_samples[i];
	}
	battery_min_values[next_battery_min] = min;

	// increment index
	next_battery_min = (next_battery_min + 1) % NUM_CYCLES_B4_REPORT;
}

/* calculate the average of the minimum values and scale to volts */
float Monitor::get_battery_volts() {
	int avg = 0;
	float result;

	for (int i = 0; i < NUM_CYCLES_B4_REPORT; i++) {
		avg += battery_min_values[i];
	}

	avg = avg / NUM_CYCLES_B4_REPORT;
	result = (float)avg * BATT_SCALING_FACTOR; // scale adc value to real voltage

	return result;
}

/* return whichever occured more frequently */
int Monitor::get_mains_status() {
	uint8_t yes = 0, no = 0;

	for (int i = 0; i < NUM_CYCLES_B4_REPORT; i++) {
		if (mains_status[i] > 0) yes++;
		else no++;
	}
	return (yes >= no ? 1 : 0);
}

float Monitor::get_latest_battery_volts() {
	return (float)battery_samples[next_battery_samp ? next_battery_samp - 1 : 9] * BATT_SCALING_FACTOR;
}

int Monitor::get_latest_mains_status() {
	return mains_status[next_mains_stat ? next_mains_stat - 1: 9];
}

/* ------ADC Interrupt Service Routine------ */

void ADC_Handler() {
	switch (monitor.adc_state) {

		case FREERUN:
			// returns false once all samples have been taken
			if (monitor.add_mains_sample(REG_ADC_RESULT));
			else {
				DISABLE_ADC();
			}
			break;

		case SINGLE:
			// single reading
			monitor.record_battery_sample(REG_ADC_RESULT);
			break;
	}


	REG_ADC_INTFLAG = ADC_INTENSET_RESRDY; // reset interrupt
}
