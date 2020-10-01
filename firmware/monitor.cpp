
#include "monitor.h"



Monitor::Monitor(int main_sensor_pin, int battery_volts_pin) {

	this->main_sensor_pin = main_sensor_pin;
	this->battery_volts_pin = battery_volts_pin;

	battery_value = 0;
	main_value = false;
 
}

void Monitor::Init() {

	pinMode(battery_volts_pin, INPUT);
	pinMode(6, INPUT);
	pinMode(7, INPUT);

	init_ADC();

}

void Monitor::init_ADC() {

	//REG_PAC2_WPCLR |= 0x10000; // clear write protect for adc

	// pins
	PORT->Group[PORTB].DIRCLR.reg = PORT_PB02;

	PORT->Group[PORTB].PINCFG[2].bit.PMUXEN = 1;
	PORT->Group[PORTB].PMUX[1].reg = PORT_PMUX_PMUXE_B; // PORTB(2*[1]) = PORTB02


	// clock
	REG_PM_APBCMASK |= PM_APBCMASK_ADC;

	REG_GCLK_GENDIV |= GCLK_GENDIV_DIV(1) | GCLK_GENDIV_ID(3);
	while (GCLK->STATUS.bit.SYNCBUSY);

	REG_GCLK_GENCTRL |= GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_ID(3);
	while (GCLK->STATUS.bit.SYNCBUSY);

	REG_GCLK_CLKCTRL |= GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(3) | GCLK_CLKCTRL_ID(30); // 30 is ADC
	while (GCLK->STATUS.bit.SYNCBUSY);

	// adc

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

}

void Monitor::trig_ADC() {
	REG_ADC_SWTRIG |= ADC_SWTRIG_START;
}

void Monitor::read_values() {
	static uint16_t main_filter = 0;
	int current_mval = 0;

	battery_value = analogRead(battery_volts_pin);

	// a low signal on the input pin means mains is present
	if (main_value) current_mval = !digitalRead(main_sensor_pin);
	else current_mval = digitalRead(main_sensor_pin);

	// 0xec00 leaves 10 bits for filter
	main_filter = (main_filter<<1) | current_mval | 0xec00;

	// mains status toggles when signal has been stable for 10 iterations
	if (main_filter == 0xfc00) {
		main_filter = 0;
		main_value = !main_value;
	}

	//SerialUSB.print("FILTER: "); SerialUSB.println(main_filter, BIN);
}


float Monitor::get_battery_volts() {
	return (float)battery_value / 1240.91;
}

String Monitor::get_main_status() {
	return main_value ? "ON" : "OFF";
}
