#include "fifobufs.h"
#include "fifobufs_mem.h"

#define		FRAME_TAKE(_owner, _ptr)		assert_param(frame->lock == FRAME_UNLOCKED); \
											frame->lock = _owner; \
											_ptr = frame; \
											frame->next = 0;
#define		CHECK_FIFO()					ColorFrame* next = frame->next; \
											if (frame == 0) \
												return;
#define		CHECK_CALC(_frame)				ColorFrame* next = frame->next; \
											if (_frame != 0) \
												return; \
											if (frame == 0) \
												return;

/* ASSERT MACROS */

#define fbmTOUCH_FRAME(__frame) \
			__frame->__line = (char *) __LINE__; \
			__frame->__file = (char *) __FILE__

#define fbmTOUCH_FIFO(__frame) \
			fbmTOUCH_FRAME(__frame)

__inline void fbmCHECK_RGB_FRAME(ColorFrame* __frame)
{
#ifdef FIFO_DEBUG
			assert_param(__frame >= &(rgb_frame[0]));
			assert_param(__frame <= &(rgb_frame[NUM_RGB_BUFFERS - 1]));
			assert_param(__frame->type == FRAME_TYPE_RGB);
			assert_param(__frame->index < NUM_RGB_BUFFERS);
			assert_param((uint32_t) __frame->buffer == (uint32_t) &(rgb_framebuffer[__frame->index][0]));
#endif
}
__inline void fbmCHECK_DMA_FRAME(ColorFrame* __frame)
{
#ifdef FIFO_DEBUG
	assert_param(__frame >= &(dma_frame[0]));
	assert_param(__frame <= &(dma_frame[NUM_DMA_BUFFERS - 1]));
	assert_param(__frame->type == FRAME_TYPE_DMA);
	assert_param(__frame->index < NUM_DMA_BUFFERS);
	assert_param((uint32_t) __frame->buffer == (uint32_t) &(dma_framebuffer[__frame->index][0]));
#endif
}

__inline void fbmCHECK_FIFO(ColorFrameFIFO* __fifo)
{
#ifdef FIFO_DEBUG
	assert_param(__fifo >= &(fifo_buffer[0]));
	assert_param(__fifo <= &(fifo_buffer[NUM_FIFO_BUFFERS - 1]));
	assert_param(__fifo->index < NUM_FIFO_BUFFERS);
	if (__fifo->first != 0)
	{
		if (__fifo->type == (FRAME_TYPE_DMA | FRAME_TYPE_RGB))
			assert_param(
							(__fifo->first >= &(rgb_frame[0]) && \
							 __fifo->first <= &(rgb_frame[NUM_RGB_BUFFERS - 1])) || \
						 	(__fifo->first >= &(dma_frame[0]) && \
							 __fifo->first <= &(dma_frame[NUM_DMA_BUFFERS - 1])));
		else if (__fifo->type == FRAME_TYPE_RGB)
			assert_param(
							(__fifo->first >= &(rgb_frame[0]) && \
							 __fifo->first <= &(rgb_frame[NUM_RGB_BUFFERS - 1])));
		else if (__fifo->type == FRAME_TYPE_DMA)
			assert_param(
						 	(__fifo->first >= &(dma_frame[0]) && \
							 __fifo->first <= &(dma_frame[NUM_DMA_BUFFERS - 1])));
		else
			assert_param(0 == 1);
	}
#endif
}

__inline void fbmCHECK_FRAME(ColorFrame* __frame)
{
#ifdef FIFO_DEBUG
	assert_param(__frame->type > 0 && __frame->type <= FRAME_TYPE_DMA);
	switch (__frame->type)
	{
		case FRAME_TYPE_RGB: 
			fbmCHECK_RGB_FRAME(__frame);
			break;
		case FRAME_TYPE_DMA:
			fbmCHECK_DMA_FRAME(__frame);
			break;
		default:
		assert_param(0==1);
			break;
	}
#endif
}
