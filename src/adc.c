#include "config.h"
#include "adc.h"
#include "time.h"

void ADC_Init(void) {
	// Configure pins
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);
	Chip_SWM_EnableFixedPin(ADC_BATT_PIN);
	Chip_SWM_EnableFixedPin(ADC_SOLAR_PIN);
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);

	// Enable ADC clock
	Chip_ADC_Init(LPC_ADC, 0);

	// Start Calibration
	Chip_ADC_StartCalibration(LPC_ADC);
	while(!Chip_ADC_IsCalibrationDone(LPC_ADC));

	// Configure clock
	Chip_ADC_SetClockRate(LPC_ADC, 1000); // Clock 1kHz
}

void ADC_DeInit(void) {
	Chip_ADC_DeInit(LPC_ADC); // Power down ADC
}

/**
 * Measures battery voltage in millivolts
 * @return battery voltage
 */
uint32_t getBatteryMV(void)
{
	uint32_t adc = getADC(ADC_BATT_CH);
	return (adc * REF_MV) >> 11;		// Return battery voltage (voltage divider factor included)
}

/**
 * Measures solar voltage in millivolts
 * @return solar voltage
 */
uint32_t getSolarMV(void)
{
	return (getADC(ADC_SOLAR_CH) * REF_MV) >> 12;		// Return solar voltage
}

/**
 * Measures voltage at specific ADx and returns 12bit value (2^12-1 equals LPC reference voltage)
 * @param ad ADx pin
 */
uint32_t getADC(uint8_t ad)
{
	// Start ADC conversion
	Chip_ADC_SetupSequencer(LPC_ADC, ADC_SEQA_IDX, ADC_SEQ_CTRL_CHANSEL(ad) | ADC_SEQ_CTRL_MODE_EOS | ADC_SEQ_CTRL_HWTRIG_POLPOS);
	Chip_ADC_EnableSequencer(LPC_ADC, ADC_SEQA_IDX);

	uint32_t i = 0;
	uint32_t gdat = 0;
	while((gdat & (ADC_DR_OVERRUN | ADC_SEQ_GDAT_DATAVALID)) == 0)
	{
		// Retrieve sampled data
		gdat = Chip_ADC_GetDataReg(LPC_ADC, ad);

		if(i++ > 1000)
			return 0; // It took too long to sample the data
	}

	return (gdat & ADC_SEQ_GDAT_RESULT_MASK) >> ADC_SEQ_GDAT_RESULT_BITPOS; // Cut out 12bit value
}
