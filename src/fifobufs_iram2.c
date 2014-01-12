#include "fifobufs.h"
#include "grizzly.h"

volatile uint8_t 		rgb_framebuffer[NUM_RGB_BUFFERS][NUM_GPIO_CHANNELS * LEDS_PER_GPIO_CHANNEL * RGB_FRAME_BPP];

ColorFrame					rgb_frame[NUM_RGB_BUFFERS];
ColorFrame					dma_frame[NUM_DMA_BUFFERS];
ColorFrameFIFO			fifo_buffer[NUM_FIFO_BUFFERS];

tDebugStats debugStats;

ColorFrame* rgbFramePtr;
ColorFrame* dmaFramePtr;
ColorFrame* dmaFillingPtr;
