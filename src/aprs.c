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

#define METER_TO_FEET(m) (((m)*26876) / 8192)


static uint16_t loss_of_gps_counter = 0;
static char temp[22];

const s_address_t addresses[] =
{ 
	{D_CALLSIGN, D_CALLSIGN_ID},	// Destination callsign
	{S_CALLSIGN, S_CALLSIGN_ID},	// Source callsign (-11 = balloon, -9 = car)
	#ifdef DIGI_PATH1
	{DIGI_PATH1, DIGI_PATH1_TTL},	// Digi1 (first digi in the chain)
	#endif
	#ifdef DIGI_PATH2
	{DIGI_PATH2, DIGI_PATH2_TTL},	// Digi2 (second digi in the chain)
	#endif
};

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
	date_t date = unixTimestamp2Date(((uint64_t)trackPoint->time) * 1000);

	ax25_send_header(addresses, sizeof(addresses)/sizeof(s_address_t));
	ax25_send_byte('/');                // Report w/ timestamp, no APRS messaging. $ = NMEA raw data

	// 170915 = 17h:09m:15s zulu (not allowed in Status Reports)
	ax25_send_string(fitoa(date.hour, temp, 2));
	ax25_send_string(fitoa(date.minute, temp, 2));
	ax25_send_string(fitoa(date.second, temp, 2));
	ax25_send_string("h");

	{
		// Latitude precalculation
		uint32_t y = 380926 * (90 - trackPoint->latitude);
		uint32_t y3  = y   / 753571;
		uint32_t y3r = y   % 753571;
		uint32_t y2  = y3r / 8281;
		uint32_t y2r = y3r % 8281;
		uint32_t y1  = y2r / 91;
		uint32_t y1r = y2r % 91;

		// Longitude precalculation
		uint32_t x = 190463 * (180 + trackPoint->longitude);
		uint32_t x3  = x   / 753571;
		uint32_t x3r = x   % 753571;
		uint32_t x2  = x3r / 8281;
		uint32_t x2r = x3r % 8281;
		uint32_t x1  = x2r / 91;
		uint32_t x1r = x2r % 91;

		// Altitude precalculation
		uint32_t a = logf(METER_TO_FEET(trackPoint->altitude)) / logf(1.002f);
		uint32_t a1  = a / 91;
		uint32_t a1r = a % 91;

		uint8_t gpsFix = gpsstate == GPS_LOCK ? GSP_FIX_CURRENT : GSP_FIX_OLD;
		uint8_t src = NMEA_SRC_GGA;
		uint8_t origin = ORIGIN_PICO;

		temp[0]  = APRS_SYMBOL_TABLE;
		temp[1]  = y3+33;
		temp[2]  = y2+33;
		temp[3]  = y1+33;
		temp[4]  = y1r+33;
		temp[5]  = x3+33;
		temp[6]  = x2+33;
		temp[7]  = x1+33;
		temp[8]  = x1r+33;
		temp[9]  = APRS_SYMBOL_ID;
		temp[10] = a1+33;
		temp[11] = a1r+33;
		temp[12] = ((gpsFix << 5) | (src << 3) | origin) + 33;
		temp[13] = 0;

		ax25_send_string(temp);
	};
	{
		temp[2] = 0;

		ax25_send_string("|");

		// Sequence ID
		uint32_t t = trackPoint->id & 0x1FFF;
		temp[0] = t/91 + 33;
		temp[1] = t%91 + 33;
		ax25_send_string(temp);

		// Battery voltage
		t = trackPoint->vbat;
		temp[0] = t/91 + 33;
		temp[1] = t%91 + 33;
		ax25_send_string(temp);

		// Solar voltage
		t = trackPoint->vsol;
		temp[0] = t/91 + 33;
		temp[1] = t%91 + 33;
		ax25_send_string(temp);

		// Temperature
		t = trackPoint->temp + 128;
		temp[0] = t/91 + 33;
		temp[1] = t%91 + 33;
		ax25_send_string(temp);

		// Sats
		t = trackPoint->satellites;
		temp[0] = t/91 + 33;
		temp[1] = t%91 + 33;
		ax25_send_string(temp);

		// GPS Loss counter
		if(gpsstate != GPS_LOCK)
		{
			if(loss_of_gps_counter >= 8191) // GPS lost 3 times (6min if cycle = 2min) TODO: This is actually not a task of APRS encoding
				loss_of_gps_counter = 0;
			loss_of_gps_counter++;
		}

		if(loss_of_gps_counter)
		{
			t = loss_of_gps_counter;
			temp[0] = t/91 + 33;
			temp[1] = t%91 + 33;
			ax25_send_string(temp);
		}

		ax25_send_byte('|');
	};

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
	// Encode APRS header
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
 * Transmit APRS telemetry configuration
 */
void transmit_telemetry_configuration(config_t type)
{
	ax25_send_header(addresses, sizeof(addresses)/sizeof(s_address_t)); // Header
	ax25_send_byte(':'); // Message flag

	// Callsign
	ax25_send_string(S_CALLSIGN);
	ax25_send_byte('-');
	ax25_send_string(itoa(S_CALLSIGN_ID, temp, 10));

	ax25_send_string(" :"); // Message separator

	switch(type) {
		case CONFIG_PARM:
			ax25_send_string("PARM.Battery,Solar,Temp,Sats,TTFF");
			break;
		case CONFIG_UNIT:
			ax25_send_string("UNIT.Volt,Volt,degC,,sec");
			break;
		case CONFIG_EQNS:
			ax25_send_string(
				"EQNS."
				"0,.001,0,"
				"0,.001,0,"
				"0,1,-128,"
				"0,1,0,"
				"0,1,0"
			);
			break;
		case CONFIG_BITS:
			ax25_send_string("BITS.11111111,Pecan Balloon");
			break;
	}

	ax25_send_footer(); // Footer
	ax25_flush_frame(); // Transmit
}

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




