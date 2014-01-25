#ifndef __DEBUG_H
#define __DEBUG_H
#include "stm32f4xx.h"

#define						EVENT_CHANNEL_RECEIVED				1
#define						EVENT_FRAME_RECEIVED				2
#define						EVENT_FRAME_INTERLEAVED				3
#define						EVENT_FRAME_DMA_SENT				4
#define						EVENT_FRAME_GARBAGE					5

typedef struct {
	uint32_t time;
	uint8_t event;
	uint8_t extra1;
	uint16_t extra2;
} DebugEvent;

#endif /* __DEBUG_H */