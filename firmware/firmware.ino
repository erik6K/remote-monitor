
#include <SPI.h>

#include "defs.h"
#include "reporter.h"
#include "monitor.h"


enum State { PERIODIC, ANALYSE };

volatile State global_STATE;
volatile bool flag_update = 0;


/* main timer initialisation */
void Init_Timer() {

	// generic clock (GCLK4) used to clock timers
	REG_GCLK_GENDIV =	GCLK_GENDIV_DIV(3) |	// divide the 48MHz clock source by divisor 3: 48MHz/3=16MHz
						GCLK_GENDIV_ID(4);		// configure Generic Clock (GCLK) 4
	while (GCLK->STATUS.bit.SYNCBUSY);			// wait for synchronization

	REG_GCLK_GENCTRL =	GCLK_GENCTRL_IDC |           // improved duty cycle for odd div
						GCLK_GENCTRL_GENEN |         // enable GCLK
						GCLK_GENCTRL_SRC_DFLL48M |   // 48MHz clock source
						GCLK_GENCTRL_ID(4);          // configure GCLK4
	while (GCLK->STATUS.bit.SYNCBUSY);

	REG_GCLK_CLKCTRL =	GCLK_CLKCTRL_CLKEN |		// enable GCLK4
						GCLK_CLKCTRL_GEN_GCLK4 |	// select GCLK4
						GCLK_CLKCTRL_ID_TC4_TC5;	// GCLK4 to TC4 and TC5
	while (GCLK->STATUS.bit.SYNCBUSY);

	// timer setup
	REG_TC4_INTFLAG |= TC_INTFLAG_MC1 | TC_INTFLAG_MC0 | TC_INTFLAG_OVF;	// Clear the interrupt flags
	REG_TC4_INTENSET = TC_INTENSET_OVF;										// Enable TC4 interrupts

	REG_TC4_CTRLA |=	TC_CTRLA_PRESCALER_DIV64 |
						TC_CTRLA_ENABLE;			// Enable TC4
	while (TC4->COUNT16.STATUS.bit.SYNCBUSY);

	NVIC_EnableIRQ(TC4_IRQn);         // Connect TC4 to Nested Vector Interrupt Controller (NVIC)
}

void setup() {

	// for debugging purposes
	/*SerialUSB.begin(9600);
	while (!SerialUSB) {
	// wait for serial port to connect
	}*/
 	
 	// init connection to azure IoT hub / radio only mode
	reporter.Init();

	// init peripherals for monitoring battery and mains
	monitor.Init();

	// initialise timer
	Init_Timer();

	// enter battery sampling state
	global_STATE = PERIODIC;
}

void loop() {
	static int report_counter = 0;

	// give the MQTT handler time to do its thing - does nothing if RADIO_ONLY
	reporter.mqtt_loop();

	switch(global_STATE)
	{
		case PERIODIC:

			if (flag_update) {
				flag_update = false;

				monitor.take_battery_sample();

				//SerialUSB.print("Vb: "); // debugging
				//SerialUSB.println(monitor.get_latest_battery_volts());
			}

			break;

		case ANALYSE:

			monitor.take_mains_samples();
			while(monitor.adc_busy()); // wait for samples to be taken

			/*
			// dump mains samples
			for (int i=0; i<SAMPLES; i++) {
			SerialUSB.print(i);
			SerialUSB.print(',');
			SerialUSB.println(monitor.get_mains_sample(i));
			}*/

			monitor.remove_DC();
			monitor.compute_fft();

			monitor.verify_50Hz();

			//SerialUSB.print("50Hz: "); // debugging
			//SerialUSB.println(monitor.get_latest_mains_status() ? "ON" : "OFF");

			monitor.save_min_battery();

			// after 10 mains checks we report data to web
			report_counter++;
			if (report_counter >= NUM_CYCLES_B4_REPORT) {
				report_counter = 0;
				reporter.report_data(monitor.get_mains_status(), monitor.get_battery_volts());
			}
			
			global_STATE = PERIODIC;
			ENABLE_TIMER()

			break;

		default:
			// should never end up here - restart system
			NVIC_SystemReset();
			break;
	}
}

/* ------Interrupt Service Routines------ */

void TC4_Handler() {	// ISR for timer TC4
	static uint8_t counter = 0;

	// check for overflow (OVF) interrupt
	if (TC4->COUNT8.INTFLAG.bit.OVF && TC4->COUNT8.INTENSET.bit.OVF) {

		// sample battery voltage 10 times before state change
		if (counter < NUM_BATT_SAMPLES) {

	   		flag_update = true;
	   		counter++;
	   	}
	   	else {
	   		counter = 0;
	   		global_STATE = ANALYSE;
	   		DISABLE_TIMER()
	   	}
      

	REG_TC4_INTFLAG = TC_INTFLAG_OVF;	// clear the overflow interrupt flag
	}

}
