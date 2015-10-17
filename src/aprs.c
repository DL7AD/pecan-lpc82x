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

#include "config.h"
#include "ax25.h"
#include "log.h"
#include "gps.h"
#include "aprs.h"
#include "adc.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "base64.h"
#include "time.h"
//#include "small_printf_code.h"

#define METER_TO_FEET(m) (((m)*26876) / 8192)


static uint16_t loss_of_gps_counter = 0;
static char temp[22];

const s_address_t addresses[] =
{ 
	{D_CALLSIGN, D_CALLSIGN_ID},  // Destination callsign
	{S_CALLSIGN, S_CALLSIGN_ID},  // Source callsign (-11 = balloon, -9 = car)
	#ifdef DIGI_PATH1
	{DIGI_PATH1, DIGI_PATH1_TTL}, // Digi1 (first digi in the chain)
	#endif
	#ifdef DIGI_PATH2
	{DIGI_PATH2, DIGI_PATH2_TTL}, // Digi2 (second digi in the chain)
	#endif
};

/**
 * Transmit APRS telemetry packet. The packet contain following values:
 * - Battery voltage in mV (has to be multiplied by 16, val=185 => 2960mV)
 * - Temperature in celcius (100 has to be subtracted, val=104 => 4 celcius)
 * - Altitude in feet (has to be multiplied by 1000, val=26 => 26000ft)
 * - Solar voltage in mV (has to be multiplied by 8, val=123 => 984mV)
 * - TTFF in seconds
 * Therafter is bitwise encoding:
 * - [7:4] Number of cycles where GPS has been lost
 * - [3:0] unused
 */
void transmit_telemetry(track_t *trackPoint)
{
	// Encode telemetry header
	ax25_send_header(addresses, sizeof(addresses)/sizeof(s_address_t));
	ax25_send_string("T#");
	ax25_send_string(fitoa(trackPoint->id % 255, temp, 3));
	ax25_send_byte(',');

	// Encode battery voltage
	ax25_send_string(fitoa(trackPoint->vbat, temp, 3));
	ax25_send_byte(',');

	// Encode temperature
	ax25_send_string(fitoa(trackPoint->temp + 100, temp, 3));
	ax25_send_byte(',');

	// Encode altitude
	uint32_t alt = METER_TO_FEET(trackPoint->altitude) / 1000;
	ax25_send_string(fitoa(alt, temp, 3));
	ax25_send_byte(',');

	// Encode solar voltage
	ax25_send_string(fitoa(trackPoint->vsol, temp, 3));
	ax25_send_byte(',');

	// Encode TTFF (time to first fix in seconds)
	ax25_send_string(fitoa(trackPoint->ttff, temp, 3));
	ax25_send_byte(',');

	// Encode bitwise
	// [7:4] Number of cycles where GPS has been lost
	// [3:0] unused

	// Encode count of GPS losses
	int z;
	for (z = 8; z > 0; z >>= 1) {
		if ((loss_of_gps_counter & z) == z) {
			ax25_send_byte('1');
		} else {
			ax25_send_byte('0');
		}
	}

	// Filling up unused bits
	ax25_send_byte('0');
	ax25_send_byte('0');
	ax25_send_byte('0');
	ax25_send_byte('0');

	ax25_send_footer();

	// Transmit
	ax25_flush_frame();
}

/**
 * Transmit APRS position packet. The comments are filled with:
 * - Static comment (can be set in config.h)
 * - Battery voltage in mV
 * - Solar voltage in mW (if tracker is solar-enabled)
 * - Temperature in Celcius
 * - Air pressure in Pascal
 * - Number of satellites being used
 * - Number of cycles where GPS has been lost (if applicable in cycle)
 */
void transmit_position(track_t *trackPoint, gpsstate_t gpsstate, uint16_t course, uint16_t speed)
{
	date_t date = unixTimestamp2Date(trackPoint->time * 1000);

	ax25_send_header(addresses, sizeof(addresses)/sizeof(s_address_t));
	ax25_send_byte('/');                // Report w/ timestamp, no APRS messaging. $ = NMEA raw data

	// 170915 = 17h:09m:15s zulu (not allowed in Status Reports)
	ax25_send_string(fitoa(date.hour, temp, 2));
	ax25_send_string(fitoa(date.minute, temp, 2));
	ax25_send_string(fitoa(date.second, temp, 2));
	ax25_send_string("h");

	uint16_t lat_degree = abs((int16_t)trackPoint->latitude);
	uint32_t lat_decimal = abs((int32_t)(trackPoint->latitude*100000))%100000;
	uint8_t lat_minute = lat_decimal * 6 / 10000;
	uint8_t lat_minute_dec = (lat_decimal * 6 / 100) % 100;
	ax25_send_string(fitoa(lat_degree, temp, 2));
	ax25_send_string(fitoa(lat_minute, temp, 2));
	ax25_send_byte('.');
	ax25_send_string(fitoa(lat_minute_dec, temp, 2));
	ax25_send_byte(trackPoint->latitude > 0 ? 'N' : 'S');

	ax25_send_byte(APRS_SYMBOL_TABLE); // Symbol table

	uint16_t lon_degree = abs((int16_t)trackPoint->longitude);
	uint32_t lon_decimal = abs((int32_t)(trackPoint->longitude*100000))%100000;
	uint8_t lon_minute = lon_decimal * 6 / 10000;
	uint8_t lon_minute_dec = (lon_decimal * 6 / 100) % 100;
	ax25_send_string(fitoa(lon_degree, temp, 3));
	ax25_send_string(fitoa(lon_minute, temp, 2));
	ax25_send_byte('.');
	ax25_send_string(fitoa(lon_minute_dec, temp, 2));
	ax25_send_byte(trackPoint->longitude > 0 ? 'E' : 'W');

	ax25_send_byte(APRS_SYMBOL_ID);                // Symbol: /O=balloon, /-=QTH, \N=buoy

	ax25_send_string(fitoa(course, temp, 3));             // Course (degrees)
	ax25_send_byte('/');                // and
	ax25_send_string(fitoa(speed, temp, 3));		// speed (knots)

	ax25_send_string("/A=");            // Altitude (feet). Goes anywhere in the comment area
	ax25_send_string(fitoa(METER_TO_FEET(trackPoint->altitude), temp, 6));

	ax25_send_byte(' ');

	uint16_t vbat = EIGHTBIT_TO_VBAT(trackPoint->vbat);
	ax25_send_string(itoa(vbat/1000, temp, 10));
	ax25_send_byte('.');
	ax25_send_string(fitoa((vbat%1000)/10, temp, 2));
	ax25_send_string("Vb ");


	#ifdef SOLAR_AVAIL
	uint16_t vsol = EIGHTBIT_TO_VSOL(trackPoint->vsol);
	ax25_send_string(itoa(vsol/1000, temp, 10));
	ax25_send_byte('.');
	ax25_send_string(fitoa((vsol%1000)/10, temp, 2));
	ax25_send_string("Vs ");
	#endif

	ax25_send_string(itoa(trackPoint->temp, temp, 10));
	ax25_send_string("C ");

	ax25_send_string("SATS");
	ax25_send_string(fitoa(trackPoint->satellites, temp, 2));

	#ifdef APRS_COMMENT
	ax25_send_byte(' ');
	ax25_send_string(APRS_COMMENT); // Comment
	#endif

	if(gpsstate != GPS_LOCK) {
		if(loss_of_gps_counter >= 5) { // GPS lost 3 times (6min if cycle = 2min) TODO: This is actually not a task of APRS encoding
			loss_of_gps_counter = 0;
		}
		loss_of_gps_counter++;
		ax25_send_string(" GPS loss ");
		ax25_send_string(fitoa(loss_of_gps_counter, temp, 2));
	} else {
		loss_of_gps_counter = 0;
	}

	ax25_send_footer();

	// Transmit
	ax25_flush_frame();
}

/**
 * Transmit APRS log packet
 */
#if LOG_SIZE
void transmit_log(track_t *trackPoint)
{
	// Encode telemetry header
	ax25_send_header(addresses, sizeof(addresses)/sizeof(s_address_t));
	ax25_send_string("{{L");

	// Encode log message
	uint8_t i;
	for(i=0; i<LOG_TRX_NUM; i++) {
		track_t *data = getNextLogPoint();
		uint8_t base64[BASE64LEN(sizeof(track_t))+1];
		base64_encode((uint8_t*)data, base64, sizeof(track_t));
		ax25_send_string((char*)base64);
	}

	// Send footer
	ax25_send_footer();

	// Transmit
	ax25_flush_frame();
}
#endif

/**
 * Formatted itoa
 */
char* fitoa(uint32_t num, char *buffer, uint32_t min_len)
{
	int digits;
	if(num)
	{
		digits = floor(log10(abs(num))) + 1;
	} else {
		digits = 1;
	}
	if(digits > min_len)
		min_len = digits;

	// Leading zeros
	for(uint32_t i=0; i<min_len-digits; i++)
		buffer[i] = '0';

	// Convert number
	itoa(num, &buffer[min_len-digits], 10);

	return buffer;
}




