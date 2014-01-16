#include "main.h"
#include "fifobufs.h"
/* 	All statically allocated memory for fifoLib is defined in
		fifobufs_iram1.c and fifobufs_iram2.c, and externs are only in
		fifobufs_mem.h which only this file should include, buffers should
		only be accessed in code via ColorFrame	pointer	*/
#include "fifobufs_mem.h"
#include "fifobufs_macros.h"
#include <string.h>
 
uint8_t fifo_new_index = 0;

/**
  * @brief  Creates new ColorFrameFIFO
  * @param  id: FIFO ID set by user
  * @param  type: ColorFrame type to accept
  * @param  permissions: ColorFrame statuses to accept
  * @param  exclude: ColorFrame statuses to reject
  * @retval pointer to new ColorFrameFIFO struct
  */
ColorFrameFIFO* fifoCreateFifo(uint8_t id, uint8_t type, uint8_t permissions, uint8_t exclude)
{
	ColorFrameFIFO* fifo;
	assert_param(id != 0);
	assert_param(fifo_new_index < NUM_FIFO_BUFFERS);
	fifo = &(fifo_buffer[fifo_new_index]);
	fifo_new_index++;
	fifo->id = id;
	fifo->type = type;
	fifo->permissions = permissions;
	fifo->exclude = exclude;
	fifo->first = 0;
	return fifo;
}

/**
  * @brief  Gets last frame in FIFO
  * @param  fifo: pointer to FIFO
  * @retval pointer to last ColorFrame in FIFO
  */
ColorFrame* fifoGetLastFrame(ColorFrameFIFO* fifo)
{
	ColorFrame* last_frame;
	fbmCHECK_FIFO(fifo);
	fifo->profile.query_count++;
	if (fifo->first == 0)
	{
		fifo->profile.query_fail_count++;
		return 0;
	}
	last_frame = fifo->first;
	while (last_frame->next != 0)
		last_frame = last_frame->next;
	if (last_frame != 0)
		fbmCHECK_FRAME(last_frame);
	return last_frame;
}

/**
  * @brief  Pushes ColorFrame to ColorFrameFIFO
  * @param  fifo: pointer to ColoFrameFIFO
  * @param  frame: pointer to ColorFrame
  * @retval pointer to next ColorFrame or same ColorFrame if push fails
  */
ColorFrame* fifoPushFrame(ColorFrameFIFO* fifo, ColorFrame* frame)
{
	ColorFrame* last_frame;
	fbmCHECK_FIFO(fifo);
	fbmCHECK_FRAME(frame);
	fifo->profile.push_count++;
	if ((frame->status & fifo->permissions) > 0 && (frame->status & fifo->exclude) == 0) {
		last_frame = fifoGetLastFrame(fifo);
		if (last_frame != 0) {
			last_frame->next = frame;
		} else {
			fifo->first = frame;
		}
		frame->last_time = LocalTime;
		if (frame->next == 0)
			return 0;
		last_frame = frame->next;
		frame->next = 0;
		return last_frame;
	}
	fifo->profile.push_fail_count++;
	return frame;
}

/**
  * @brief  Pops the first ColorFrame from ColorFrameFIFO
  * @param  fifo: pointer to ColorFrameFIFO
  * @retval pointer to popped ColorFrame or 0 if there is none
  */
ColorFrame* fifoPopFrame(ColorFrameFIFO* fifo)
{
	ColorFrame* frame = fifo->first;
	fbmCHECK_FIFO(fifo);
	fifo->profile.pop_count++;
	if (frame == 0)
	{
		fifo->profile.pop_fail_count++;
		return 0;
	}
	fifo->first = frame->next;
	fbmCHECK_FRAME(frame);
	frame->next = 0;
	return frame;
}

/**
  * @brief  Pulls ColorFrame from anywhere in ColorFrameFIFO
  * @param  fifo: pointer to ColorFrameFIFO
  * @param 	frame: pointer to ColorFrame
  * @retval pointer to pulled ColorFrame or 0 if failed
  */
ColorFrame* fifoPullFrame(ColorFrameFIFO* fifo, ColorFrame* frame)
{
	ColorFrame* last_frame = fifo->first;
	fbmCHECK_FIFO(fifo);
	fbmCHECK_FRAME(frame);
	fifo->profile.pull_count++;
	if (last_frame == 0) 
		return 0;
	if (last_frame == frame) {
		fifo->first = frame->next;
		frame->next = 0;
		return frame;
	}
	while (last_frame->next != 0) {
		if (last_frame->next == frame) {
			last_frame->next = frame->next;
			frame->next = 0;
			return frame;
		}
		last_frame = last_frame->next;
	}
	fifo->profile.pull_fail_count++;
	return 0;
}

/**
  * @brief  Finds next ColorFrame with status starting from given frame
  * @param  frame: pointer to first ColorFrame
  * @param  status: status to find
  * @param  exclude: status to exclude
  * @retval pointer to ColorFrame or 0 if no frame found
  */
ColorFrame* fifoFindNextFrameStatus(ColorFrame* frame, uint8_t permissions, uint8_t exclude)
{
	ColorFrame* ret_frame;
	ret_frame = frame;
	while (ret_frame != 0)
	{
		if ((ret_frame->status & permissions) > 0 && (ret_frame->status & ~exclude) == 0)
		{
			fbmCHECK_FRAME(ret_frame);
			return ret_frame;
		}
		ret_frame = ret_frame->next;
	}
	return 0;
}

/**
  * @brief  Finds ColorFrame in ColorFrameFIFO with lowest frame number
  * @param  fifo: pointer to ColorFrameFIFO
  * @retval pointer to ColorFrame or 0 if no frame in FIFO
  */
ColorFrame* fifoFindLowestCount(ColorFrameFIFO* fifo)
{
	ColorFrame* frame = fifo->first;
	ColorFrame* ret_frame = 0;
	uint32_t count = 0xFFFFFFFF;

	fbmCHECK_FIFO(fifo);
	fifo->profile.query_count++;
	while (frame != 0) {
		if (frame->count < count) {
			count = frame->count;
			ret_frame = frame;
		}
		frame = frame->next;
	}
	if (ret_frame == 0)
		fifo->profile.query_fail_count++;
	else
		fbmCHECK_FRAME(ret_frame);
	return ret_frame;
}

/**
  * @brief  Finds ColorFrame in ColorFrameFIFO with highest frame number
  * @param  fifo: pointer to ColorFrameFIFO
  * @retval pointer to ColorFrame or 0 if no frame in FIFO
  */
ColorFrame* fifoFindHighestCount(ColorFrameFIFO* fifo)
{
	ColorFrame* frame = fifo->first;
	ColorFrame* ret_frame = 0;
	uint32_t count = 0x00000000;

	fbmCHECK_FIFO(fifo);
	fifo->profile.query_count++;
	while (frame != 0) {
		if (frame->count > count) {
			count = frame->count;
			ret_frame = frame;
		}
		frame = frame->next;
	}
	if (ret_frame == 0)
		fifo->profile.query_fail_count++;
	else
		fbmCHECK_FRAME(ret_frame);
	return ret_frame;
}

/**
  * @brief  Finds ColorFrame in ColorFrameFIFO with given frame number
  * @param  fifo: pointer to ColorFrameFIFO
  * @param  count: ColorFrame count
  * @retval pointer to ColorFrame or 0 if not found
  */
ColorFrame* fifoFindFrameCount(ColorFrameFIFO* fifo, uint32_t count)
{
	ColorFrame* frame;
	fbmCHECK_FIFO(fifo);
	fifo->profile.query_count++;
	frame = fifo->first;
	while (frame != 0)
	{
		if (frame->count == count)
		{
			fbmCHECK_FRAME(frame);
			return frame;
		}
		frame = frame->next;
	}
	fifo->profile.query_fail_count++;
	return 0;
}

/**
  * @brief  Resets ColorFrame and zeroes its memory
  * @param  frame: pointer to ColorFrame
  * @retval None
  */	
void fifoResetFrame(ColorFrame* frame)
{
	fbmCHECK_FRAME(frame);
	frame->count = 0;
	frame->offset = 0;
	frame->flags = 0;
	//frame->next = 0;
	memset((void *) frame->buffer, 0x00, frame->size);
	frame->status = FRAME_ZEROED;
}

/**
  * @brief  Initializes ColorFrames with type FRAME_TYPE_RGB
  * @param  None
  * @retval pointer to first ColorFrame in list
  */
ColorFrame* fifoInitializeRGBFrames(void)
{
	int i;
	for (i = 0; i < NUM_RGB_BUFFERS; i++)
	{
		rgb_frame[i].index = i;
		rgb_frame[i].type = FRAME_TYPE_RGB;
		rgb_frame[i].size = NUM_GPIO_CHANNELS * LEDS_PER_GPIO_CHANNEL * RGB_FRAME_BPP;
		rgb_frame[i].buffer = &(rgb_framebuffer[i][0]);
		fifoResetFrame(&(rgb_frame[i]));
		if (i + 1 < NUM_RGB_BUFFERS)
			rgb_frame[i].next = &(rgb_frame[i+1]);
		else
			rgb_frame[i].next = &(rgb_frame[0]);
		if (i == 0)
			rgb_frame[i].prev = &(rgb_frame[NUM_RGB_BUFFERS - 1]);
		else
			rgb_frame[i].prev = &(rgb_frame[i - 1]);
	}
	return &(rgb_frame[0]);
}

/**
  * @brief  Initializes ColorFrames with type FRAME_TYPE_DMA
  * @param  None
  * @retval pointer to first ColorFrame in list
  */
ColorFrame* fifoInitializeDMAFrames(void)
{
	int i;
	for (i = 0; i < NUM_DMA_BUFFERS; i++)
	{
		dma_frame[i].index = i;
		dma_frame[i].type = FRAME_TYPE_DMA;
		dma_frame[i].size = LEDS_PER_GPIO_CHANNEL * DMA_FRAME_BPP;
		dma_frame[i].buffer = &(dma_framebuffer[i][0]);
		fifoResetFrame(&(dma_frame[i]));
		if (i + 1 < NUM_DMA_BUFFERS)
			dma_frame[i].next = &(dma_frame[i+1]);
		else
			dma_frame[i].next = &(dma_frame[0]);
		if (i == 0)
			dma_frame[i].prev = &(dma_frame[NUM_DMA_BUFFERS - 1]);
		else
			dma_frame[i].prev = &(dma_frame[i - 1]);
	}
	return &(dma_frame[0]);
}

/**
  * @brief  Initialize ColorFrame (RGB) as a test frame
  * @param  pointer to ColorFrame
  * @retval None
  */
void fifoInitializeDummyFrame(ColorFrame* frame)
{
	uint32_t i = 0, j = 0, k = 0;
	volatile uint8_t * buffer;
	for (i = 0; i < NUM_GPIO_CHANNELS; i++) {
		for (j = 0; j < 4; j++) {
			buffer = &(frame->buffer[i * 3 * LEDS_PER_GPIO_CHANNEL + j*64*3]);
			/* 	LED Face Color Legend
				First LED of first 3 edges is white
				Last LED of first 3 edges is Black
				Rest of LEDs in first 3 strips are R, G, B
				Last strip is yellow (0x00FFFF) except purple for channel NUM
			*/
			buffer--;
			*(++buffer) = 0xFF;
			*(++buffer) = 0xFF;
			*(++buffer) = 0xFF;
			for (k = 0; k < 14; k++) {
				*(++buffer) = 0xFF;
				*(++buffer) = 0x00;
				*(++buffer) = 0x00;
			}
			*(++buffer) = 0x00;
			*(++buffer) = 0x00;
			*(++buffer) = 0x00;
			*(++buffer) = 0xFF;
			*(++buffer) = 0xFF;
			*(++buffer) = 0xFF;
			for (k = 0; k < 14; k++) {
				*(++buffer) = 0x00;
				*(++buffer) = 0xFF;
				*(++buffer) = 0x00;
			}
			*(++buffer) = 0x00;
			*(++buffer) = 0x00;
			*(++buffer) = 0x00;
			*(++buffer) = 0xFF;
			*(++buffer) = 0xFF;
			*(++buffer) = 0xFF;
			for (k = 0; k < 14; k++) {
				*(++buffer) = 0x00;
				*(++buffer) = 0x00;
				*(++buffer) = 0xFF;
			}
			*(++buffer) = 0x00;
			*(++buffer) = 0x00;
			*(++buffer) = 0x00;
			buffer++;
			for (k = 0; k < 16; k++) {
				*(buffer + k * 3) = 0x00;
				*(buffer + k * 3 + 1) = 0xFF;
				*(buffer + k * 3 + 2) = 0xFF;
			}
			for (k = 0; k < i; k++) {
				*(buffer + k * 3) = 0xFF;
				*(buffer + k * 3 + 1) = 0x00;
				*(buffer + k * 3 + 2) = 0xFF;
			}
		}
	}
}
