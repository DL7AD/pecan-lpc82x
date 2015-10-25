#include "chip.h"

/**
 * Sets microcontroller to 1MHz
 */
void setClockPowerSave(void)
{
	Chip_Clock_SetMainClockSource(SYSCTL_MAINCLKSRC_IRC);		// Change main clock source to IRC
	Chip_Clock_SetUARTClockDiv(12);								// Divide clock to 1MHz
	LPC_SYSCTL->PDRUNCFG |= (1 << 7);							// Power down PLL
	SystemCoreClockUpdate();									// Update system clock variable
	SysTick_Config(SystemCoreClock / 1000);						// Configure 1ms tick timer
}

/**
 * Sets microcontroller to 24MHz
 */
void setClockMaxPerformance(void)
{
	LPC_SYSCTL->PDRUNCFG &= ~(1 << 7);							// Power up PLL
	Chip_Clock_SetMainClockSource(SYSCTL_MAINCLKSRC_PLLOUT);	// Change main clock source to PLL
	Chip_Clock_SetUARTClockDiv(1);								// Disable divider
	SystemCoreClockUpdate();									// Update system clock variable
	SysTick_Config(SystemCoreClock / 1000);						// Configure 1ms tick timer
}
