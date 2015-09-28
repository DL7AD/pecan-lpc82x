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

// Type of Pecan
//							TARGET_FEMTO_PICO3    for Pecan Femto 3
#define TARGET				TARGET_PECAN_FEMTO3

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


#define TIME_SLEEP_CYCLE	60
#define TIME_MAX_GPS_SEARCH	120

// Radio power:				Radio power (for Si4464)
//							Range 1-127, Radio output power depends on VCC voltage.
//							127 @ VCC=3400mV ~ 100mW
//							20  @ VCC=3400mV ~ 10mW
#define RADIO_POWER			5

// Logging size:
#define LOG_SIZE			0
#define LOG_CYCLE_TIME		7200
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
//#define

// Battery type: Pecan Pico has two options of battery types
// 1. PRIMARY				LiFeSe2 Power save modes disabled, battery will be used until completely empty
// 2. SECONDARY				LiFePO4 GPS will be kept off below 2700mV, no transmission is made below 2500mV to keep
//							the accumulator healthy
#define BATTERY_TYPE		PRIMARY

// Oscillator frequency:	The oscillator is powered by different VCC levels and different PWM levels. So it has to
//							be adjusted/stabilized by software depending on voltage. At the moment there are two
//							different oscillators being used by Thomas (DL4MDW) and Sven (DL7AD)
#define OSC_FREQ			19997700
//#define OSC_FREQ			26992900

/* ============================================== Target definitions =============================================== */
/* ========================================== Please don't touch anything ========================================== */

#if TARGET == TARGET_PECAN_FEMTO3

	#define SOLAR_AVAIL							// Solar feed available
	#define GPS_BAUDRATE		9600			// Baudrate for ublox MAX7 or MAX8
	#define REF_MV				3300

	#define UART_RXD_PIN		0
	#define UART_TXD_PIN		14

	#define ADC_BATT_PIN		4
	#define ADC_SOLAR_PIN		17

	#define RADIO_MOSI_PIN		27
	#define RADIO_MISO_PIN		26
	#define RADIO_SCK_PIN		25
	#define RADIO_CS_PIN		16
	#define RADIO_SDN_PIN		10
	#define RADIO_GPIO_PIN		1

	#define GPS_PWR_PIN1		12
	#define GPS_PWR_PIN2		13

#else
	#error No/incorrect target selected
#endif

#if BATTERY_TYPE == SECONDARY
	#define VOLTAGE_NOGPS		2500			// Don't switch on GPS below this voltage (Telemetry transmission only)
	#define VOLTAGE_NOTRANSMIT	2300			// Don't transmit below this voltage
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


/* =============================================== Misc definitions ================================================ */
/* ========================================== Please don't touch anything ========================================== */

#ifdef BMP180_AVAIL
	#define USE_I2C
#endif
#if GPS_BUS == I2C
	#define USE_I2C
#endif

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
