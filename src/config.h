/* trackuino copyright (C) 2010  EA5HAV Javi
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "defines.h"
#include "chip.h"

// APRS Source Callsign
#define S_CALLSIGN			"DL7AD"
#define S_CALLSIGN_ID		13

// APRS Symbol
#define APRS_SYMBOL_TABLE	'/'
#define APRS_SYMBOL_ID		'O'

// APRS Digipeating paths (comment this out, if not used)
#define DIGI_PATH1			"WIDE1"
#define DIGI_PATH1_TTL		1
//#define DIGI_PATH2			"WIDE1"
//#define DIGI_PATH2_TTL		1

// APRS comment (comment this out, if not used)
//#define APRS_COMMENT		"Pecan Tracker"

// TX delay in milliseconds
#define TX_DELAY			60


#define TIME_SLEEP_CYCLE	300
#define TIME_MAX_GPS_SEARCH	120

// Radio power:				Radio power (for Si4063 @ VCC=2500mV)
//							Range 1-127
//							127 ~ 50mW
//							20  ~ 5mW
//							7   ~ 1mW
//							Radio power (for Si4060 @ VCC=2500mV)
//							Range 1-127
//							127 ~ 10mW
//							20  ~ 1mW
#define RADIO_POWER			5

// Logging size:
#define LOG_SIZE			0		// Log size (Set to 0 to disable log function)
#define LOG_CYCLE_TIME		7200	// Log point sample cycle in seconds
#define LOG_TRX_NUM			6		// Log messages that are transmitted in one packet

/* ============================================== Target definitions =============================================== */
/* ========================= Pecan Pico 6 specific (applicable only if Pecan Pico 6 used) ========================== */

// Power management: Pecan Pico has two options of power management
// 1. Use hardware switch:	GPS will be switched on by MOSFET each cycle. When GPS has locked (>4Sats, it will be
//							switched off by MOSFET. GPS will remain switched on when GPS has no lock until it locks
//							GPS and UART interface will be reset and reinitialized when GPS does not lock for
//							3 cycles. To use this mode, comment out USE_GPS_POWER_SAVE.
// 1. Use GPS power save:	GPS will be switched on permanently and sent into power save mode when GPS has
//							lock (when >4Sats). GPS and UART interface will be reset and reinitialized when GPS
//							does not lock for 3 cycles. To use this mode, USE_GPS_POWER_SAVE has to be set.
//#define USE_GPS_POWER_SAVE

// Battery type: Pecan Femto has two options of battery types
// 1. PRIMARY				LiFeSe2 Power save modes disabled, battery will be used until completely empty
// 2. LiFePo4				LiFePO4 GPS will be kept off below 2700mV, no transmission is made below 2500mV to keep
//							the accumulator healthy
// 2. LiPo					LiPo GPS will be kept off below 3000mV, no transmission is made below 2500mV to keep the
//							accumulator healthy
#define BATTERY_TYPE		LiPo

// Solar feed available
#define SOLAR_AVAIL

/* ============================================== Target definitions =============================================== */
/* ========================================== Please don't touch anything ========================================== */

#define REF_MV				2500				// Reference voltage (Vcc)
#define OSC_FREQ			26992900			// Oscillator frequency

#define UART_RXD_PIN		IOCON_PIO0			// GPS TXD pin
#define UART_TXD_PIN		IOCON_PIO14			// GPS RXD pin

#define ADC_BATT_PIN		SWM_FIXED_ADC11		// ADC battery (connected to voltage divider, factor 0.5)
#define ADC_SOLAR_PIN		SWM_FIXED_ADC9		// ADC solar panels (directly connected)

#define RADIO_MOSI_PIN		IOCON_PIO27			// MOSI
#define RADIO_MISO_PIN		IOCON_PIO26			// MISO
#define RADIO_SCK_PIN		IOCON_PIO25			// SCK
#define RADIO_CS_PIN		IOCON_PIO16			// CS Si446x
#define RADIO_SDN_PIN		IOCON_PIO10			// Radio shutdown
#define RADIO_GPIO_PIN		IOCON_PIO1			// Radio GPIO (GPIO1 at Si446x)

												// GPS power switch pins (have to be switched both in same state)
#define GPS_PWR_PIN1		IOCON_PIO12			// Power switch pin 1 (connected to GPS_Vcc)
#define GPS_PWR_PIN2		IOCON_PIO13			// Power switch pin 2 (connected to GPS_Vcc)

/* =============================================== Misc definitions ================================================ */
/* ========================================== Please don't touch anything ========================================== */

#if BATTERY_TYPE == LiPo
	#define VOLTAGE_NOGPS		3000			// Don't switch on GPS below this voltage
	#define VOLTAGE_NOTRANSMIT	2500			// Don't transmit below this voltage
	#define VOLTAGE_GPS_MAXDROP 100				// Max. Battery drop voltage until GPS is switched off while acquisition
												// Example: VOLTAGE_NOGPS = 2700 & VOLTAGE_GPS_MAXDROP = 100 => GPS will be switched
												// off at 2600mV, GPS will not be switched on if battery voltage already below 2700mV
#elif BATTERY_TYPE == LiFePo4
	#define VOLTAGE_NOGPS		2700			// Don't switch on GPS below this voltage
	#define VOLTAGE_NOTRANSMIT	2500			// Don't transmit below this voltage
	#define VOLTAGE_GPS_MAXDROP 100				// Max. Battery drop voltage until GPS is switched off while acquisition
												// Example: VOLTAGE_NOGPS = 2700 & VOLTAGE_GPS_MAXDROP = 100 => GPS will be switched
												// off at 2600mV, GPS will not be switched on if battery voltage already below 2700mV
#elif BATTERY_TYPE == PRIMARY
	#define VOLTAGE_NOGPS		0				// All battery saving options are switched off, so battery will be used until completely empty
	#define VOLTAGE_NOTRANSMIT	0
	#define VOLTAGE_GPS_MAXDROP 0
#endif

/* ================================================ Error messages ================================================= */
/* ========================================== Please don't touch anything ========================================== */

// TODO: Rewrite configuration validation again

/* ============================ Constant definitions (which will never change in life) ============================= */
/* ========================================== Please don't touch anything ========================================== */

// Frequency (which is used after reset state)
#define DEFAULT_FREQUENCY			144800000

// In other regions the APRS frequencies are different. Our balloon may travel
// from one region to another, so we may QSY according to GPS defined geographical regions
// Here we set some regional frequencies:

#define RADIO_FREQUENCY_REGION1		144800000 // Europe & Africa
#define RADIO_FREQUENCY_REGION2		144390000 // North and south America (Brazil is different)

#define RADIO_FREQUENCY_JAPAN		144660000
#define RADIO_FREQUENCY_CHINA		144640000
#define RADIO_FREQUENCY_BRAZIL		145570000
#define RADIO_FREQUENCY_AUSTRALIA	145175000
#define RADIO_FREQUENCY_NEWZEALAND	144575000
#define RADIO_FREQUENCY_THAILAND	145525000

// APRS Destination callsign
#define D_CALLSIGN					"APECAN" // APExxx = Pecan device
#define D_CALLSIGN_ID				0

#endif
