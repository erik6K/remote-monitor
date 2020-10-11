
#include <SPI.h>
#include <WiFiNINA.h>
#include <utility/wifi_drv.h>

#include "monitor.h"
#include "defs.h"


enum State { PERIODIC, ANALYSE, DEBUG };

volatile State g_STATE;
volatile bool flag_update = 0;



void setLEDs(uint8_t led, uint8_t green, uint8_t blue) {
  WiFiDrv::analogWrite(25, green);
  WiFiDrv::analogWrite(26, led);
  WiFiDrv::analogWrite(27, blue);
}

void Init_Timer() {

	// generic clock (GCLK4) used to clock timers
	REG_GCLK_GENDIV =	GCLK_GENDIV_DIV(3) |	// Divide the 48MHz clock source by divisor 3: 48MHz/3=16MHz
						GCLK_GENDIV_ID(4);		// Configure Generic Clock (GCLK) 4
	while (GCLK->STATUS.bit.SYNCBUSY);			// Wait for synchronization

	REG_GCLK_GENCTRL =	GCLK_GENCTRL_IDC |           // improved duty cycle for odd div
						GCLK_GENCTRL_GENEN |         // enable GCLK
						GCLK_GENCTRL_SRC_DFLL48M |   // 48MHz clock source
						GCLK_GENCTRL_ID(4);          // configure GCLK4
	while (GCLK->STATUS.bit.SYNCBUSY);

	REG_GCLK_CLKCTRL =	GCLK_CLKCTRL_CLKEN |		// Enable GCLK4
						GCLK_CLKCTRL_GEN_GCLK4 |	// Select GCLK4
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

	SerialUSB.begin(9600);
	while (!SerialUSB) {
	// wait for serial port to connect
	}

	// RGB LED Pin Init
	WiFiDrv::pinMode(25, OUTPUT); //GREEN
	WiFiDrv::pinMode(26, OUTPUT); //lED
	WiFiDrv::pinMode(27, OUTPUT); //BLUE

	// <pin initialisations here>

	// LED Pin is D6 - also connected to mains sensor input on prototype board
	//pinMode(LED_BUILTIN, OUTPUT);



	monitor.Init();

	// initialise timer
	Init_Timer();

	g_STATE = PERIODIC;

}

void loop() {

	switch(g_STATE)
	{
		case PERIODIC:

			if (flag_update) {
				flag_update = false;

				monitor.take_battery_sample();

			//	SerialUSB.print("Vb: ");
			//	SerialUSB.println(monitor.get_battery_volts());
			}

			break;

		case ANALYSE:

			SerialUSB.println("ADC READINGS");

			monitor.take_mains_samples();
			while(monitor.adc_busy());
			/*
			for (int i=0; i<SAMPLES; i++) {
			SerialUSB.print(i);
			SerialUSB.print(',');
			SerialUSB.println(monitor.get_mains_sample(i));
			}*/
			monitor.remove_DC();
			monitor.compute_fft();

			//SerialUSB.print("50Hz: ");
			SerialUSB.println(monitor.verify_50Hz() ? "Yes" : "No");

			// re-enable timer
			g_STATE = PERIODIC;
			ENABLE_TIMER()

			break;

		case DEBUG:

			monitor.take_mains_samples();
			while(monitor.adc_busy());

			monitor.remove_DC();
			
			for (int i=0; i<SAMPLES; i++) {
			SerialUSB.print(i);
			SerialUSB.print(',');
			SerialUSB.println(monitor.get_mains_sample(i));
			}

			while(1);

			break;
	}

}

/* ------Interrupt Service Routines------ */

void TC4_Handler() {	// ISR for timer TC4
	static uint8_t counter = 0;
	//static uint8_t led = 0;

	// Check for overflow (OVF) interrupt
	if (TC4->COUNT8.INTFLAG.bit.OVF && TC4->COUNT8.INTENSET.bit.OVF) {

		// sample battery voltage 10 times before state change
		if (counter < NUM_BATT_SAMPLES) {
			//led = ~led;
	   		//digitalWrite(LED_BUILTIN, led);

	   		flag_update = true;
	   		counter++;
	   	}
	   	else {
	   		counter = 0;
	   		g_STATE = ANALYSE;
	   		DISABLE_TIMER()
	   	}
      

	REG_TC4_INTFLAG = TC_INTFLAG_OVF;	// Clear the OVF interrupt flag
	}

}
