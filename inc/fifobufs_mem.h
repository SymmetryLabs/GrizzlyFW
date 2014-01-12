/* --------------------------------------------------------------------------*/
/* Define to prevent recursive inclusion ------------------------------------*/
/* --------------------------------------------------------------------------*/
#ifndef __FIFOBUFS_MEM_H
#define __FIFOBUFS_MEM_H

#include "stm32f4xx.h"
#include "fifobufs.h"
#include "grizzly.h"

extern volatile uint8_t 		rgb_framebuffer[NUM_RGB_BUFFERS][NUM_GPIO_CHANNELS * LEDS_PER_GPIO_CHANNEL * RGB_FRAME_BPP];
extern volatile	uint8_t			dma_framebuffer[NUM_DMA_BUFFERS][LEDS_PER_GPIO_CHANNEL * DMA_FRAME_BPP];

extern ColorFrame						rgb_frame[NUM_RGB_BUFFERS];
extern ColorFrame						dma_frame[NUM_DMA_BUFFERS];
extern ColorFrameFIFO				fifo_buffer[NUM_FIFO_BUFFERS];

extern volatile uint16_t clock_rising_buffer;
extern volatile uint16_t clock_falling_buffer;

extern uint8_t fifo_new_index;

/*	
	Frame Buffers:
		last_frame = Oldest frame still in memory
		old_interp_frame = Old interpoalated frame
		current_frame = Last frame filled
		interp_frame = Interpolation frame currently being filled
		next_frame = Frame currently being filled from ethernet
		
		When a packet is received with a frame number higher than Frame 3's,
			Frame 
		
		Example:
			Frame 0 set to all black
			current_frame = Frame 0
			next_frame = Frame 2
			Packet (Frame #1) received to Frame 2 (Frame count > current_frame Frame Count)
				last_frame = Frame 0
				current_frame = Frame 2
				interp_frame = Frame 1
				next_frame = Frame 4
			Packet (Frame #2) received to Frame 4 (Frame count > current_frame Frame Count)
				last_frame = Frame 2
				current_frame = Frame 4
				old_interp_frame = Frame 1
				interp_frame = Frame 3
				next_frame = Frame 0
			Packet (Frame #3) received to Frame 0 (Frame count > current_frame Frame Count)
				last_frame = Frame 4
				current_frame = Frame 0
				old_interp_frame = Frame 3
				interp_frame = Frame 1
				next_frame = Frame 2
		
		Target frame rate interrupt can only send last_frame and old_interp_frame


		Main loop:
			Receive Packet
				If packet->frame_number > next_frame->frame_number
					if old_interp_frame has been sent:
						zero old_interp_frame
						switch old_interp_frame and interp_frame
					
					
*/

/* PSEUDO CODE
	Timer @ Target frame rate
		If last_frame ready and unsent
			send
		else if last frame sent, if old_interp_frame 
		Initiates frame DMA transfer
	Timer @ 4 times target frame rate
		if last_frame == sent:
			mark for new frame
		if old_interp_frame == sent:
			mark for new interp frame

	Main loop:
		for each frame:
			switch frame->lock:
				case unlocked:
					switch frame->status:
						case zeroed:
							if next_receiver_frame is 0:
								set next receiver frame
						case received:
							if another frame is ready
						case interpolated:
						case interleaved:
						case sent:
						case used
							lock = frame cleanup



*/


#endif /* __FIFOBUFS_MEM_H */
