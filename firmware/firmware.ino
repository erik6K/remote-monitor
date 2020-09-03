
#include <SPI.h>
#include <WiFiNINA.h>
#include <utility/wifi_drv.h>

#include "monitor.h"

#define RADIO_VOX 2
#define MAIN_SENSOR_PIN 3
#define BATTERY_VOLTS_PIN 4


Monitor monitor(MAIN_SENSOR_PIN, BATTERY_VOLTS_PIN);

bool flag_update = 0;


void TC4_Handler() {	// ISR for timer TC4
	static uint8_t red = 0;

	// Check for overflow (OVF) interrupt
	if (TC4->COUNT8.INTFLAG.bit.OVF && TC4->COUNT8.INTENSET.bit.OVF) {
		red = ~red;
   		//setLEDs(red, 0, 125);
   		digitalWrite(LED_BUILTIN, red);

   		flag_update = true;
      

	REG_TC4_INTFLAG = TC_INTFLAG_OVF;	// Clear the OVF interrupt flag
	}

}

void setLEDs(uint8_t red, uint8_t green, uint8_t blue) {
  WiFiDrv::analogWrite(25, green);
  WiFiDrv::analogWrite(26, red);
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

	REG_GCLK_CLKCTRL =	GCLK_CLKCTRL_CLKEN |		// Enable GCLK4 to TC4 and TC5
						GCLK_CLKCTRL_GEN_GCLK4 |	// Select GCLK4
						GCLK_CLKCTRL_ID_TC4_TC5;	// GCLK4 to TC4 and TC5
	while (GCLK->STATUS.bit.SYNCBUSY);

	REG_TC4_INTFLAG |= TC_INTFLAG_MC1 | TC_INTFLAG_MC0 | TC_INTFLAG_OVF;	// Clear the interrupt flags
	REG_TC4_INTENSET = TC_INTENSET_OVF;										// Enable TC4 interrupts

	REG_TC4_CTRLA |=	TC_CTRLA_PRESCALER_DIV64 |
						TC_CTRLA_ENABLE;			// Enable TC4
	while (TC4->COUNT16.STATUS.bit.SYNCBUSY);

	NVIC_EnableIRQ(TC4_IRQn);         // Connect TC4 to Nested Vector Interrupt Controller (NVIC)
}

void setup() {

	Serial.begin(9600);
	while (!Serial) {
	// wait for serial port to connect. Needed for native USB port only
	}

	// RGB LED Pin Init
	WiFiDrv::pinMode(25, OUTPUT); //GREEN
	WiFiDrv::pinMode(26, OUTPUT); //RED
	WiFiDrv::pinMode(27, OUTPUT); //BLUE

	// <pin initialisations here>
	pinMode(LED_BUILTIN, OUTPUT);

	monitor.Init();

	// set adc resolution to 12 bits for MKR board
	analogReadResolution(12);


	// initialise timer
	Init_Timer();

}

void loop() {

	if (flag_update) {
		flag_update = false;

		monitor.read_values()

		Serial.print("M/S: "); Serial.print(monitor.get_main_status());
		Serial.print(" B: "); Serial.println(monitor.get_battery_volts());
	}

}
