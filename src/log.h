#ifndef __LOG_H__
#define __LOG_H__
#if LOG_SIZE

#include "chip.h"

void logTrackPoint(track_t logPoint);
track_t* getNextLogPoint(void);

#endif
#endif
