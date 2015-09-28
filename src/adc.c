#include "config.h"
#include "adc.h"

void ADC_Init(void) {
	//LPC_SYSCON->PDRUNCFG		&= ~(1<<4);		// Power up ADC
	//LPC_SYSCON->SYSAHBCLKCTRL	|= (1<<13);		// Enable ADC clock

	//LPC_IOCON->ADC_PIO_BATT		 = ADC_AD_BATT < AD5 ? 0x02 : 0x01;
	//LPC_IOCON->ADC_PIO_SOLAR	 = ADC_AD_SOLAR < AD5 ? 0x02 : 0x01;

	//LPC_ADC->CR = 0x0B01; // Configure ADC block to max. accuracy
}

void ADC_DeInit(void) {
	//LPC_SYSCON->PDRUNCFG        |= 1<<4;		// Power down ADC
	//LPC_SYSCON->SYSAHBCLKCTRL   &= ~(1<<13);	// Disable ADC clock
}

/**
 * Measures battery voltage in millivolts
 * @return battery voltage
 */
uint32_t getBatteryMV(void)
{
	uint32_t adc = getADC(ADC_BATT_PIN);
	return (adc * REF_MV) >> 10;		// Return battery voltage
}

/**
 * Measures solar voltage in millivolts
 * @return solar voltage
 */
uint32_t getSolarMV(void)
{
	return getADC(ADC_SOLAR_PIN) / REF_MV;
}

/**
 * Measures voltage at specific ADx and returns 10bit value (2^10-1 equals LPC reference voltage)
 * @param ad ADx pin
 */
uint16_t getADC(uint8_t ad)
{
	//LPC_ADC->CR  = (LPC_ADC->CR & 0xFFF0) | (1 << ad);	// Configure active adc port
	//LPC_ADC->CR |= (1 << 24);							// Start conversion
	//while((LPC_ADC->DR[ad] < 0x7FFFFFFF));				// Wait for done bit to be toggled
	//return ((LPC_ADC->DR[ad] & 0xFFC0) >> 6);			// Cut out 10bit value
	return 0;
}
