#include "config.h"
#include "cw.h"
#include "Si446x.h"
#include "chip.h"
#include "time.h"
#include "types.h"
#include "clock.h"
#include <math.h>

void CW_Init(void)
{
	// Initialize radio
	Si446x_Init(MODEM_CW);

	// Set radio power and frequency
	radioTune(RADIO_FREQUENCY_CW, RADIO_POWER_CW);

	setClockMaxPerformance();
}

void CW_DeInit(void)
{
	setClockPowerSave();
	radioShutdown();
}

void dah(void)
{
	RF_GPIO_SET(true);
	delay(180);
	RF_GPIO_SET(false);
	delay(60);
}
void dit(void)
{
	RF_GPIO_SET(true);
	delay(60);
	RF_GPIO_SET(false);
	delay(60);
}

void CW_transmit(const char* letter)
{
	for(uint32_t i=0; letter[i]!=0; i++)
		CW_transmit_char(letter[i]);
}

void CW_transmit_char(char letter)
{
	switch(letter) {
		case 'A':
			dit();
			dah();
		break;
		case 'B':
			dah();
			dit();
			dit();
			dit();
		break;
		case 'C':
			dah();
			dit();
			dah();
			dit();
		break;
		case 'D':
			dah();
			dit();
			dit();
		break;
		case 'E':
			dit();
		break;
		case 'F':
			dit();
			dit();
			dah();
			dit();
		break;
		case 'G':
			dah();
			dah();
			dit();
		break;
		case 'H':
			dit();
			dit();
			dit();
			dit();
		break;
		case 'I':
			dit();
			dit();
		break;
		case 'J':
			dit();
			dah();
			dah();
			dah();
		break;
		case 'K':
			dah();
			dit();
			dah();
		break;
		case 'L':
			dit();
			dah();
			dit();
			dit();
		break;
		case 'M':
			dah();
			dah();
		break;
		case 'N':
			dah();
			dit();
		break;
		case 'O':
			dah();
			dah();
			dah();
		break;
		case 'P':
			dit();
			dah();
			dah();
			dit();
		break;
		case 'Q':
			dah();
			dah();
			dit();
			dah();
		break;
		case 'R':
			dit();
			dah();
			dit();
		break;
		case 'S':
			dit();
			dit();
			dit();
		break;
		case 'T':
			dah();
		break;
		case 'U':
			dit();
			dit();
			dah();
		break;
		case 'V':
			dit();
			dit();
			dit();
			dah();
		break;
		case 'W':
			dit();
			dah();
			dah();
		break;
		case 'X':
			dah();
			dit();
			dit();
			dah();
		break;
		case 'Y':
			dah();
			dit();
			dah();
			dah();
		break;
		case 'Z':
			dah();
			dah();
			dit();
			dit();
		break;
		case '1':
			dit();
			dah();
			dah();
			dah();
			dah();
		break;
		case '2':
			dit();
			dit();
			dah();
			dah();
			dah();
		break;
		case '3':
			dit();
			dit();
			dit();
			dah();
			dah();
		break;
		case '4':
			dit();
			dit();
			dit();
			dit();
			dah();
		break;
		case '5':
			dit();
			dit();
			dit();
			dit();
			dit();
		break;
		case '6':
			dah();
			dit();
			dit();
			dit();
			dit();
		break;
		case '7':
			dah();
			dah();
			dit();
			dit();
			dit();
		break;
		case '8':
			dah();
			dah();
			dah();
			dit();
			dit();
		break;
		case '9':
			dah();
			dah();
			dah();
			dah();
			dit();
		break;
		case '0':
			dah();
			dah();
			dah();
			dah();
			dah();
		break;
		case ' ':
			delay(160);
		break;
		case '.':
			dit();
			dah();
			dit();
			dah();
			dit();
			dah();
		break;
	}
	delay(240);
}

void positionToMaidenhead(double lat, double lon, char m[])
{
	lon = lon + 180;
	lat = lat + 90;

	m[0] = ((uint8_t)'A') + ((uint8_t)(lon / 20));
	m[1] = ((uint8_t)'A') + ((uint8_t)(lat / 10));

	m[2] = ((uint8_t)'0') + ((uint8_t)(fmod(lon, 20)/2));
	m[3] = ((uint8_t)'0') + ((uint8_t)(fmod(lat, 10)/1));

	m[4] = ((uint8_t)'A') + ((uint8_t)((lon - ( ((uint8_t)(lon/2))*2)) / (5.0/60.0)));
	m[5] = ((uint8_t)'A') + ((uint8_t)((lat - ( ((uint8_t)(lat/1))*1)) / (2.5/60.0)));

	m[6] = 0;
}
