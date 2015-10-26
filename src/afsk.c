#include "afsk.h"

#include "config.h"
#include "Si446x.h"
#include "gps.h"
#include "types.h"
#include "clock.h"
#include "chip.h"


#define TX_CPU_CLOCK_1			((1865*(temp)) + 11787492)

#define CLOCK_PER_TICK			256

#define PLAYBACK_RATE_1			(TX_CPU_CLOCK_1 / CLOCK_PER_TICK) // Tickrate 46.875 kHz

#define BAUD_RATE				1200
#define SAMPLES_PER_BAUD		(PLAYBACK_RATE_1 / BAUD_RATE) // 52.083333333 / 26.041666667
#define PHASE_DELTA_1200		(10066329600 / CLOCK_PER_TICK / PLAYBACK_RATE_1) // Fixed point 9.7 // 1258 / 2516
#define PHASE_DELTA_2200		(18454937600 / CLOCK_PER_TICK / PLAYBACK_RATE_1) // 2306 / 4613


// Module globals
static uint16_t current_byte;
static uint16_t current_sample_in_baud;		// 1 bit = SAMPLES_PER_BAUD samples
static uint32_t phase_delta;				// 1200/2200 for standard AX.25
static uint32_t phase;						// Fixed point 9.7 (2PI = TABLE_SIZE)
static uint32_t packet_pos;					// Next bit to be sent out
volatile static bool modem_busy = false;	// Is timer running
static int8_t temp;

// Exported globals
uint16_t modem_packet_size = 0;
uint8_t modem_packet[MODEM_MAX_PACKET];

void AFSK_Init(void)
{
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, 28);

	// Initialize radio
	Si446x_Init(MODEM_AFSK);

	// Sample temperature
	temp = Si446x_getTemperature();

	// Set radio power and frequency
	radioTune(gps_get_region_frequency(), RADIO_POWER_APRS);

	// Setup sampling timer
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SCT);
	Chip_SYSCTL_PeriphReset(RESET_SCT);
	Chip_SCT_Config(LPC_SCT, SCT_CONFIG_32BIT_COUNTER | SCT_CONFIG_CLKMODE_BUSCLK);
	Chip_SCT_SetMatchCount(LPC_SCT, SCT_MATCH_0, CLOCK_PER_TICK);					// Set the match count for match register 0
	Chip_SCT_SetMatchReload(LPC_SCT, SCT_MATCH_0, CLOCK_PER_TICK);					// Set the match reload value for match reload register 0
	LPC_SCT->EV[0].CTRL = (1 << 12);												// Event 0 only happens on a match condition
	LPC_SCT->EV[0].STATE = 0x00000001;												// Event 0 only happens in state 0
	LPC_SCT->LIMIT_U = 0x00000001;													// Event 0 is used as the counter limit
	Chip_SCT_EnableEventInt(LPC_SCT, SCT_EVT_0);									// Enable flag to request an interrupt for Event 0
	modem_busy = true;																// Set modem busy flag
	NVIC_EnableIRQ(SCT_IRQn);														// Enable the interrupt for the SCT
	Chip_SCT_ClearControl(LPC_SCT, SCT_CTRL_HALT_L);								// Start the SCT counter by clearing Halt_L in the SCT control register
}

void modem_flush_frame(void) {
	phase_delta = PHASE_DELTA_1200;
	phase = 0;
	packet_pos = 0;
	current_sample_in_baud = 0;

	if(gpsIsOn())
		GPS_hibernate_uart();				// Hibernate UART because it would interrupt the modulation
	AFSK_Init();							// Initialize timers and radio
	setClockMaxPerformance();				// Set clocking to max performance (24MHz)

	while(modem_busy)						// Wait for radio getting finished
		__WFI();

	setClockPowerSave();					// Return to power save mode (1MHz)
	radioShutdown();						// Shutdown radio
	if(gpsIsOn())
		GPS_wake_uart();					// Init UART again to continue GPS decoding
}

/**
 * Interrupt routine which is called <PLAYBACK_RATE> times per second.
 * This method is supposed to load the next sample into the PWM timer.
 */

void SCT_IRQHandler(void) {
	static bool tog = 0;
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, 28, !tog); // Toggling this pin improves modulation (don't know why SSE)
	tog = !tog;

	// If done sending packet
	if(packet_pos == modem_packet_size) {
		Chip_SCT_SetControl(LPC_SCT, SCT_CTRL_HALT_L);	// Stop the SCT counter by setting Halt_L in the SCT control register
		Chip_SCT_ClearEventFlag(LPC_SCT, SCT_EVT_0);	// Clear interrupt
		modem_busy = false;								// Set modem busy flag
		return;											// Done
	}

	// If sent SAMPLES_PER_BAUD already, go to the next bit
	if (current_sample_in_baud == 0) {    // Load up next bit
		if ((packet_pos & 7) == 0) {          // Load up next byte
			current_byte = modem_packet[packet_pos >> 3];
		} else {
			current_byte = current_byte / 2;  // ">>1" forces int conversion
		}

		if ((current_byte & 1) == 0) {
			// Toggle tone (1200 <> 2200)
			phase_delta ^= (PHASE_DELTA_1200 ^ PHASE_DELTA_2200);
		}
	}

	phase += phase_delta;

	RF_GPIO_SET(((phase >> 7) & 0xFF) > 127);

	if(++current_sample_in_baud == SAMPLES_PER_BAUD) {
		current_sample_in_baud = 0;
		packet_pos++;
	}

	Chip_SCT_ClearEventFlag(LPC_SCT, SCT_EVT_0); // Clear interrupt
}
