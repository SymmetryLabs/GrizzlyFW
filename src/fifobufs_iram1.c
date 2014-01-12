#include "fifobufs.h"
#include "grizzly.h"

volatile	uint8_t			dma_framebuffer[NUM_DMA_BUFFERS][LEDS_PER_GPIO_CHANNEL * DMA_FRAME_BPP];

volatile uint16_t clock_rising_buffer = 0xFFFF;
volatile uint16_t clock_falling_buffer = 0x0000;
