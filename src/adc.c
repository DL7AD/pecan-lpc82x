#include "config.h"
#include "adc.h"

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
	return (getADC(ADC_BATT_PIN) * REF_MV) >> 12;		// Return battery voltage
}

/**
 * Measures solar voltage in millivolts
 * @return solar voltage
 */
uint32_t getSolarMV(void)
{
	return (getADC(ADC_SOLAR_PIN) * REF_MV) >> 12;		// Return solar voltage
}

/**
 * Measures voltage at specific ADx and returns 12bit value (2^12-1 equals LPC reference voltage)
 * @param ad ADx pin
 */
uint16_t getADC(uint8_t ad)
{
	// Start ADC conversion
	Chip_ADC_SetupSequencer(LPC_ADC, ADC_SEQA_IDX, ADC_SEQ_CTRL_CHANSEL(ad) | ADC_SEQ_CTRL_MODE_EOS);
	Chip_ADC_EnableSequencer(LPC_ADC, ADC_SEQA_IDX);
	Chip_ADC_StartBurstSequencer(LPC_ADC, ADC_SEQA_IDX);
	uint32_t seq_gdat = Chip_ADC_GetSequencerDataReg(LPC_ADC, ADC_SEQ_CTRL_CHANSEL(ad));

	return (LPC_ADC->DR[ad] >> 4) & 0xFFF;			// Cut out 12bit value
}
