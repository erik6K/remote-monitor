#include "monitor.h"

Monitor monitor;

Monitor::Monitor() {

}

void Monitor::Init() {

	// pins also connected to A1 on prototype board
	pinMode(6, INPUT);
	pinMode(7, INPUT);

	// RGB LED Pin Init
	WiFiDrv::pinMode(25, OUTPUT); //GREEN
	WiFiDrv::pinMode(26, OUTPUT); //RED
	WiFiDrv::pinMode(27, OUTPUT); //BLUE

	init_ADC_Pins();
	init_ADC_Clock();

	//mains_status = 0;
	mains_stat_ind = 0;
	battery_index = 0;

}

void Monitor::init_ADC_Pins() {

	PORT->Group[PORTB].DIRCLR.reg |= PORT_PB02 | PORT_PB03; // set pins A1 and A2 as input

	PORT->Group[PORTB].PINCFG[2].bit.PMUXEN = 1;
	PORT->Group[PORTB].PINCFG[3].bit.PMUXEN = 1;

	// PORTB(2*[1]) = PORTB02, (2*[1] + 1) = PORTB03
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

	//REG_PAC2_WPCLR |= 0x10000; // clear write protect for adc
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
	
	SerialUSB.print("Min: "); SerialUSB.println(min);
	SerialUSB.print("Max: "); SerialUSB.println(max);

	led.r = 0; led.g = 0; led.b = 0;
	if (max - min < 1000) led.r = 80;
	else if (max - min > 4090) led.b = 80;
	
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

	// bins 3 and 4 correspond to ~35 and ~51 Hz respectively
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
//	SerialUSB.print("50HZ: "); SerialUSB.println(mag_50Hz);
//	SerialUSB.print("noise floor: "); SerialUSB.println(noise_floor);

	// check if 50Hz magnitude is at least 5 times larger than noise floor
	mains_status[mains_stat_ind] = (mag_50Hz > noise_floor*5 ? 1 : 0);

	// to be removed
	if (mains_status[mains_stat_ind]) led.g = 80;
	setLEDs();

	// cycle the index for moving average
	mains_stat_ind = (mains_stat_ind + 1) % NUM_CYCLES_B4_REPORT;
}

bool Monitor::add_mains_sample(int smpl) {

	mains_samples[mains_samp_ind] = smpl;
	mains_samp_ind++;

	if (mains_samp_ind == SAMPLES) {
		adc_BUSY = false;
		return false;
	}
	else return true;
}

void Monitor::record_battery_sample(int smpl) {

	battery_samples[battery_index] = smpl;

	// cycle the index for moving average
	battery_index = (battery_index + 1) % NUM_BATT_SAMPLES;
}

int Monitor::get_mains_sample(int ind) {
	return mains_samples[ind];
}

void Monitor::take_mains_samples() {

	adc_BUSY = true;
	mains_samp_ind = 0;
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

/* calculates the average and minimum values of the battery samples taken */
float Monitor::get_battery_volts() {
	int min = 4096;
	int avg = 0;
	float result[2];

	for (int i = 0; i < NUM_BATT_SAMPLES; i++) {
		if (min > battery_samples[i]) min = battery_samples[i];
		avg += battery_samples[i];
	}
	avg = avg / NUM_BATT_SAMPLES;

	result[0] = (float)min / 1240.91 * 6.4;
	result[1] = (float)avg / 1240.91 * 6.4;

	return result[1];
}

int Monitor::get_mains_status() {
	return mains_status[mains_stat_ind];

	uint8_t yes = 0, no = 0;

	for (int i = 0; i < NUM_CYCLES_B4_REPORT; i++) {
		if (mains_status[i] > 0) yes++;
		else no++;
	}
	return (yes >= no ? 1 : 0);
}

void Monitor::setLEDs() {
	WiFiDrv::analogWrite(25, led.g);
	WiFiDrv::analogWrite(26, led.r);
	WiFiDrv::analogWrite(27, led.b);
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
