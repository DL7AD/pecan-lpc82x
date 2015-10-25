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

#ifndef __APRS_H__
#define __APRS_H__

#include "log.h"
#include "types.h"
#include "config.h"

#define GSP_FIX_OLD						0x0
#define GSP_FIX_CURRENT					0x1

#define NMEA_SRC_OTHER					0x0
#define NMEA_SRC_GLL					0x1
#define NMEA_SRC_GGA					0x2
#define NMEA_SRC_RMC					0x3

#define ORIGIN_COMPRESSED				0x0
#define ORIGIN_TNC_BTEXT				0x1
#define ORIGIN_SOFTWARE					0x2
#define ORIGIN_RESERVED					0x3
#define ORIGIN_KPC3						0x4
#define ORIGIN_PICO						0x5
#define ORIGIN_OTHER_TRACKER			0x6
#define ORIGIN_DIGIPEATER_CONVERSION	0x7

void transmit_telemetry(track_t *trackPoint);
void transmit_position(track_t *trackPoint, gpsstate_t gpsstate, uint16_t course, uint16_t speed);
void transmit_telemetry_configuration(config_t type);
#if LOG_SIZE
void transmit_log(track_t *trackPoint);
#endif
char* fitoa(uint32_t num, char *buffer, uint32_t min_len);

#endif
