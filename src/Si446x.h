#ifndef __SI406X__H__
#define __SI406X__H__

#include "types.h"

#define POWER_0DBM		6
#define POWER_8DBM		16
#define POWER_10DBM		20
#define POWER_14DBM		32
#define POWER_17DBM		40
#define POWER_20DBM		127

#define RF_GPIO_SET(Select)	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, RADIO_GPIO_PIN, Select)

void Si446x_Init(modem_t modem_type);
void Si446x_write(uint16_t* txData, uint32_t len);
void sendFrequencyToSi446x(uint32_t freq);
void setModemAFSK(void);
void setModemCW(void);
void setDeviation(uint32_t deviation);
void setPowerLevel(uint8_t level);
void startTx(void);
void stopTx(void);
void radioShutdown(void);
void radioTune(uint32_t frequency, uint8_t level);
int8_t Si446x_getTemperature(void);

#endif
