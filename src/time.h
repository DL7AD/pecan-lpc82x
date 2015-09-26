#ifndef __GLOBAL_H__
#define __GLOBAL_H__

uint64_t date2UnixTimestamp(date_t time);
void setUnixTimestamp(uint64_t time);
uint64_t getUnixTimestamp(void);
void incrementUnixTimestamp(uint32_t ms);
date_t unixTimestamp2Date(uint64_t time);
void delay(uint32_t ms);

#endif
