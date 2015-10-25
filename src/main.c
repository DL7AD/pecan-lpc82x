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

#include "chip.h"
#include "aprs.h"
#include "ax25.h"
#include "config.h"
#include "gps.h"
#include "Si446x.h"
#include "uart.h"
#include "debug.h"
#include "adc.h"
#include "afsk.h"
#include "time.h"
#include "clock.h"
#include "cw.h"
#include <stdlib.h>

/**
 * Enter power save mode for 8 seconds. Power save is disabled and replaced by
 * delay function in debug mode to avoid stopping SWD interface.
 */
void power_save()
{
	//#undef DEBUG
	#ifdef DEBUG
	delay(8000);
	#else
	SetLowCurrentOnGPIO();
	InitDeepSleep(8000);
	EnterDeepSleep();
	// Reinitializing is done by wakeup interrupt routine in sleep.c => On_Wakeup
	#endif
}

int main(void)
{
	setClockPowerSave();									// Setup power save clocking
	Chip_GPIO_Init(LPC_GPIO_PORT);							// Enable GPIO clock

	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);			// Enable clock to switch matrix so we can configure the matrix
	Chip_SWM_DisableFixedPin(SWM_FIXED_CLKIN);				// LPC824 needs the fixed pin ACMP2 pin disabled to use pin as GPIO
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);		// Turn clock to switch matrix back off to save power

	SysTick_Config(SystemCoreClock / 1000);					// Configure 1ms tick timer

	// This delay is necessary to get access again after module fell into a deep sleep state in which the reset pin is disabled !!!
	// To get access again, its necessary to access the chip in active mode. If chip is almost every time in sleep mode, it can be
	// only waked up by the reset pin which is (as mentioned before) disabled.
	delay(3000); // !!! IMPORTANT IMPORTANT IMPORTANT !!! DO NOT REMOVE THIS DELAY UNDER ANY CIRCUMSTANCES !!!

	// The actual program starts here

	trackingstate_t trackingstate = TRANSMIT_CONFIG;
	gpsstate_t gpsstate = GPS_LOSS;

	track_t trackPoint;

	uint64_t timestampPointer = 0;
	#if LOG_SIZE
	uint64_t lastLogPoint = 0;
	#endif
	uint32_t id = 0;

	while(true) {

		// Measure battery voltage
		ADC_Init();
		uint32_t batt_voltage = getBatteryMV();
		uint32_t sol_voltage = getSolarMV();
		ADC_DeInit();

		// Freeze tracker when battery below specific voltage
		if(batt_voltage < VOLTAGE_NOTRANSMIT)
		{
			GPS_PowerOff();
			timestampPointer = getUnixTimestamp(); // Mark timestamp for sleep routine
			trackingstate = SLEEP;
		}

		// Switch states
		switch(trackingstate)
		{
			case SLEEP:
				if(batt_voltage < VOLTAGE_NOGPS && gpsIsOn()) // Tracker has low battery, so switch off GPS
					GPS_PowerOff();

				if(batt_voltage > VOLTAGE_NOCW)
					trackingstate = TRANSMIT_CW;

				if(getUnixTimestamp()-timestampPointer >= TIME_SLEEP_CYCLE*1000) {
					trackingstate = SWITCH_ON_GPS;
					continue;
				}

				power_save();
				break;

			case TRANSMIT_CW:
				CW_Init();

				CW_transmit("BALLOON ");
				CW_transmit(S_CALLSIGN);

				char m[7];
				CW_transmit("  ");
				positionToMaidenhead(lastFix.latitude, lastFix.longitude, m);
				CW_transmit(m);

				CW_transmit(" ALT ");
				CW_transmit(itoa(lastFix.altitude, m, 10));
				CW_transmit("M");

				CW_DeInit();

				trackingstate = SLEEP;
				break;

			case SWITCH_ON_GPS:
				if(batt_voltage < VOLTAGE_NOGPS) { // Tracker has low battery, so switch off GPS
					if(gpsIsOn())
						GPS_PowerOff();
					trackingstate = LOG;
					gpsstate = GPS_LOW_BATT;
					continue;
				}

				// Switch on GPS if switched off
				GPS_Init();

				timestampPointer = getUnixTimestamp(); // Mark timestamp for search_gps routine
				trackingstate = SEARCH_GPS;
				break;

			case SEARCH_GPS:
				// Decide to switch off GPS due to low battery
				if(batt_voltage < VOLTAGE_NOGPS-VOLTAGE_GPS_MAXDROP) { //Battery voltage dropped below specific value while acquisitioning
					GPS_PowerOff(); // Stop consuming power
					trackingstate = LOG;
					gpsstate = GPS_LOW_BATT;
				}

				// Parse NMEA
				if(gpsIsOn()) {
					uint8_t c;
					while(UART_RxByte(&c)) {
						if(gps_decode(c)) { // Lock and 5 sats are used
							#ifdef USE_GPS_POWER_SAVE
							gps_activate_power_save(); // Activate power save mode
							#else
							GPS_PowerOff(); // Switch off GPS
							#endif

							// We have received and decoded our location
							gpsSetTime2lock((getUnixTimestamp() - timestampPointer) / 1000);
							trackingstate = LOG;
							gpsstate = GPS_LOCK;
						}
					}
				}

				uint64_t time = getUnixTimestamp();
				if(time-timestampPointer >= TIME_MAX_GPS_SEARCH*1000) { // Searching for GPS took too long
					gpsSetTime2lock(TIME_MAX_GPS_SEARCH);
					trackingstate = LOG;
					gpsstate = GPS_LOSS;
					continue;
				}

				break;

			case LOG:
				trackPoint.id = ++id;
				trackPoint.time = getUnixTimestamp()/1000;
				trackPoint.latitude = lastFix.latitude;
				trackPoint.longitude = lastFix.longitude;
				trackPoint.altitude = lastFix.altitude;
				trackPoint.satellites = lastFix.satellites;
				trackPoint.ttff = lastFix.ttff;

				trackPoint.vbat = VBAT_TO_EIGHTBIT(batt_voltage);
				#ifdef SOLAR_AVAIL
				trackPoint.vsol = VSOL_TO_EIGHTBIT(sol_voltage);
				#else
				trackPoint.vsol = 0;
				#endif

				Si446x_Init(MODEM_NONE);
				trackPoint.temp = Si446x_getTemperature();
				radioShutdown();

				#if LOG_SIZE
				if(getUnixTimestamp()-lastLogPoint >= LOG_CYCLE_TIME*1000) { // New log point necessary
					logTrackPoint(trackPoint);
					lastLogPoint = getUnixTimestamp();
				}
				#endif

				trackingstate = TRANSMIT;
				break;

			case TRANSMIT:
				// Mark timestamp for sleep routine (which will probably follow after this state)
				timestampPointer = getUnixTimestamp();

				// Transmit APRS telemetry
				transmit_telemetry(&trackPoint);

				// Wait a few seconds (Else aprs.fi reports "[Rate limited (< 5 sec)]")
				power_save(6000);

				// Transmit APRS position
				transmit_position(&trackPoint, gpsstate, lastFix.course, lastFix.speed);

				#if LOG_SIZE
				// Wait a few seconds (Else aprs.fi reports "[Rate limited (< 5 sec)]")
				power_save(6000);

				// Transmit log packet
				transmit_log(&trackPoint);
				#endif

				// Change state depending on GPS status
				if(gpsstate == GPS_LOCK || gpsstate == GPS_LOW_BATT) {
					trackingstate = SLEEP;
				} else { // GPS_LOSS
					trackingstate = SWITCH_ON_GPS;
				}

				break;

			case TRANSMIT_CONFIG:
				transmit_telemetry_configuration(CONFIG_PARM);
				power_save(6000);

				transmit_telemetry_configuration(CONFIG_UNIT);
				power_save(6000);

				transmit_telemetry_configuration(CONFIG_EQNS);
				power_save(6000);

				transmit_telemetry_configuration(CONFIG_BITS);
				power_save(6000);

				trackingstate = LOG;
				break;

			default: // It should actually never reach this state
				trackingstate = LOG;
				break;
		}
	}
}
