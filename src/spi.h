#ifndef __SPI__H__
#define __SPI__H__

#include "chip.h"

void SPI_Init(void);
void SPI_Transmit(uint16_t *TxBuf, uint16_t *RxBuf, uint32_t len);
void SPI_DeInit(void);

#endif
