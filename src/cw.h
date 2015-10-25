#ifndef __CW__H__
#define __CW__H__

#include "config.h"
#include "cw.h"
#include "Si446x.h"
#include "chip.h"
#include "time.h"
#include "types.h"

void CW_Init(void);
void CW_DeInit(void);
void dah(void);
void dit(void);
void CW_transmit(const char* letter);
void CW_transmit_char(char letter);
void positionToMaidenhead(double lat, double lon, char m[]);

#endif
