#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "types.h"
#include "chip.h"

void SysTick_Handler(void);
uint64_t date2UnixTimestamp(date_t time);
date_t unixTimestamp2Date(uint64_t time);
uint64_t getUnixTimestamp(void);
void setUnixTimestamp(uint64_t time);
void incrementUnixTimestamp(uint32_t ms);
void delay(uint32_t ms);

#endif
