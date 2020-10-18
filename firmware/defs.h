#ifndef DEFS_H
#define DEFS_H


#define RADIO_VOX_PIN 15

/* 
NUM_CYCLES defines how many times the ANALYSE state is entered before data is
reported. It controls the frequency of messages sent to Azure, and the size of
the mains_status and battery_min_values array which are averaged before sending.
Each cycle takes roughly ~4 secs.
*/
#define NUM_CYCLES_B4_REPORT 20
#define NUM_BATT_SAMPLES 10

/*
Factor for scaling 12 bit ADC reading to actual voltage, 1240.91 scales the 12
bit reading to 0-3.3V, the numerator is determined by the voltage divider in
front of the input pin A2.
*/
#define BATT_SCALING_FACTOR (6.4 / 1240.91)



#define DISABLE_TIMER()	REG_TC4_CTRLA &= ~TC_CTRLA_ENABLE; \
						while (TC4->COUNT16.STATUS.bit.SYNCBUSY);

#define ENABLE_TIMER() 	REG_TC4_CTRLA |= TC_CTRLA_ENABLE; \
						while (TC4->COUNT16.STATUS.bit.SYNCBUSY);

#define DISABLE_ADC()	REG_ADC_CTRLA &= ~ADC_CTRLA_ENABLE; \
						while (ADC->STATUS.bit.SYNCBUSY);

#define ENABLE_ADC()	REG_ADC_CTRLA |= ADC_CTRLA_ENABLE; \
						while (ADC->STATUS.bit.SYNCBUSY);

#endif