
#include "monitor.h"
#include "defs.h"

Monitor monitor;

Monitor::Monitor() {

	battery_value = 0;

}

void Monitor::Init() {

	// pins also connected to A1 on prototype board
	pinMode(6, INPUT);
	pinMode(7, INPUT);

	init_ADC_Pins();
	init_ADC_Clock();
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

	REG_ADC_REFCTRL = ADC_REFCTRL_REFSEL_INTVCC1;

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

	REG_ADC_REFCTRL = ADC_REFCTRL_REFSEL_INTVCC1;

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
	/*
	SerialUSB.print("Min: "); SerialUSB.println(min);
	SerialUSB.print("Max: "); SerialUSB.println(max);
	*/
	int offset = ((max - min) / 2) + min;

	for (int i = 0; i < SAMPLES; i++) {
		mains_samples[i] -= offset;
	}
}

void Monitor::compute_fft() {
	ZeroFFT(mains_samples, SAMPLES);
}

/* 50Hz must be at least 3x larger than any other frequency to be considered present */
int Monitor::verify_50Hz() {

	uint8_t histogram[200] = {0};

	int mag_50Hz = mains_samples[3] + mains_samples[4];


	for (int i = 0; i < (SAMPLES >> 1); i++) {
		int mag = min(mains_samples[i], 200);
		histogram[mag]++;
	}

	int maxh = 0;
	int noise_floor = 0;

	for (int i = 0; i <= 200; i++) {
		if (histogram[i] > maxh) {
			maxh = histogram[i];
			noise_floor = i;
		}
	}

	return (mag_50Hz > noise_floor*3 ? 1 : 0);
}

bool Monitor::add_mains_sample(int smpl) {

	mains_samples[sample_index] = smpl;
	sample_index++;

	if (sample_index == SAMPLES) {
		adc_BUSY = false;
		return false;
	}
	else return true;

}

void Monitor::record_battery_sample(int smpl) {
	battery_value = smpl;
}

int Monitor::get_mains_sample(int ind) {
	return mains_samples[ind];
}

void Monitor::take_mains_samples() {

	adc_BUSY = true;
	sample_index = 0;
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


float Monitor::get_battery_volts() {
	return (float)battery_value / 1240.91;
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