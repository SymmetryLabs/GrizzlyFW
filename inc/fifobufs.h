/* --------------------------------------------------------------------------*/
/* Define to prevent recursive inclusion ------------------------------------*/
/* --------------------------------------------------------------------------*/
#ifndef __FIFOBUFS_H
#define __FIFOBUFS_H

#include "stm32f4xx.h"
#include "pbuf.h"

typedef struct ColorFrame ColorFrame;

typedef ColorFrame* (*ColorFrameHandler)(ColorFrame* frame);

struct ColorFrame {
	// Frame's index in statically allocated global array (Set at initialization)
	uint8_t index;
	// Which data frame is used for (FRAME_TYPE_RGB or _DMA)
	uint8_t type;
	// Current status of frame (reset to FRAME_ZEROED)
	uint8_t status;
	// Each bit represents the channel that has been received and filled
	uint16_t flags;
	// Frame number (reset to 0)
	uint32_t count;
	
	// Number of bytes the frame takes up in memory (Set at initialization)
	uint32_t size;
	/* Last LocalTime buffer was pushed to FIFO */	
	uint32_t last_time;
	
	uint32_t start_time;
	
	// Interpolation ratio of last frame sent (< 0.01 = none set, > .99 = last frame sent)
	float sent_ratio;
	
	// Offset of operation currently locking the frame (reset to 0)
	uint32_t offset;
	// Pointer to frame in memory (Set at initialization)
	volatile uint8_t* buffer;
	// Current rgb buffer being processed
	uint32_t current_buffer;
	
	// Next frame in circular buffer
	ColorFrame* next;
	// Previous frame in circular buffer
	ColorFrame* prev;
	
	// Pointer to pbuf holding channel packet
	struct pbuf* channels[8];
	// Last unset pbuf pointer in array
	uint8_t last_pbuf;
	// Current pbuf being processed
	uint8_t current_pbuf;
	// Offset of current pbuffer
	uint32_t pbuf_offset;
};

/* FIFO worker callback function definition */
typedef ColorFrame* (*fifoWorkerPtr)(ColorFrame*);

typedef struct ColorFrameFIFO ColorFrameFIFO;

struct ColorFrameFIFO {
	/* Index of struct in statically allocated array */
	uint8_t index;
	/* FIFO id given by user */
	uint8_t id;
	/* Type of ColorFrame in FIFO (FRAME_TYPE_RGB and/or _DMA) */
	uint8_t type;
	/* ColorFrame statuses to accept (FRAME_ZEROED through FRAME_GARBAGE) */
	uint8_t permissions;
	/* ColorFrame statuses to reject (same as permissions) */
	uint8_t exclude;
	/* First ColorFrame in FIFO */
	ColorFrame* first;
	struct {
		/* Number of pushes to FIFO */
		uint32_t push_count;
		/* Number of pops from FIFO */
		uint32_t pop_count;
		/* Number of pulls from FIFO */
		uint32_t pull_count;
		/* Number of times fifo is queried (FindFrame, etc.) */
		uint32_t query_count;
		/* Number of times FIFO rejected push */
		uint32_t push_fail_count;
		/* Number of times FIFO pop failed */
		uint32_t pop_fail_count;
		/* Number of times FIFO pull failed */
		uint32_t pull_fail_count;
		/* Number of times FIFO pull failed */
		uint32_t query_fail_count;
	} profile;
	char* __file;
	char* __line;
};

extern ColorFrameFIFO* fifoCreateFifo(uint8_t id, uint8_t type, uint8_t permissions, uint8_t exclude);
extern ColorFrame* fifoGetLastFrame(ColorFrameFIFO* __fifo);
extern ColorFrame* fifoPushFrame(ColorFrameFIFO* fifo, ColorFrame* frame);
extern ColorFrame* fifoPopFrame(ColorFrameFIFO* fifo);
extern ColorFrame* fifoPullFrame(ColorFrameFIFO* fifo, ColorFrame* frame);
ColorFrame* fifoFindNextFrameStatus(ColorFrame* frame, uint8_t permission, uint8_t exclude);
ColorFrame* fifoFindLowestCount(ColorFrameFIFO* fifo);
ColorFrame* fifoFindHighestCount(ColorFrameFIFO* fifo);
ColorFrame* fifoFindFrameCount(ColorFrameFIFO* fifo, uint32_t count);

extern void fifoResetFrame(ColorFrame* frame);


extern ColorFrame* fifoInitializeRGBFrames(void);
extern ColorFrame* fifoInitializeDMAFrames(void);
extern void fifoInitializeDummyFrames(void);

#endif /* __FIFOBUFS_H */
