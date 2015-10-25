#ifndef __TYPES_H__
#define __TYPES_H__

#include "lpc_types.h"

typedef struct GPS {
	uint32_t	time;		// Sekunden seit Mitternacht
	int32_t		lat;		// Latitude in 10µMinuten, >=0 N, <0 S
	int32_t		lon;		// Longitude in 10µMinuten, >=0 E, <0 W
	uint16_t	altitude;	// Positive Meter
	uint8_t		satellites;	// Anzahl der Satelliten
	bool		isValid;	// Gültigkeit
} GPS_t;

typedef struct {
	uint32_t id;
	uint32_t time;
	float latitude;
	float longitude;
	uint16_t altitude;
	uint8_t satellites;
	uint8_t ttff;
	uint8_t vbat;
	uint8_t vsol;
	int8_t temp;
	uint32_t pressure;
} track_t;

typedef enum {
	SLEEP,					// General sleep mode
	SWITCH_ON_GPS,			// Switch on GPS
	SEARCH_GPS,				// Search for GPS
	LOG,					// Log data packet
	TRANSMIT,				// Transmit telemetry and position
	TRANSMIT_CONFIG,		// Transmit telemetry configuration
	TRANSMIT_CW				// Transmit CW (position)
} trackingstate_t;

typedef enum {
	GPS_LOCK,
	GPS_LOSS,
	GPS_LOW_BATT
} gpsstate_t;

typedef enum {
	CONFIG_PARM,
	CONFIG_UNIT,
	CONFIG_EQNS,
	CONFIG_BITS
} config_t;

typedef struct {
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} date_t;

typedef enum {
	MODEM_NONE,
	MODEM_AFSK,
	MODEM_CW
} modem_t;

#endif
