#include "stm32f4x7_eth.h"
#include "netconf.h"
#include "main.h"
#include "udp_echoserver.h"
#include "udp_colorserver.h"
#include "grizzly.h"
#include "fifobufs.h"

extern void Delay(uint32_t nCount);
extern __IO uint32_t LocalTime;

void run_inner_frame_worker(void)
{
	uint8_t *src1, *src2;
	volatile uint8_t *interp1, *interp2;
	volatile uint8_t *dest1, *dest2;
	volatile uint8_t *prev1, *prev2;
	volatile uint32_t *dma;
	ColorFrame* prev_frame = rgbFramePtr->prev;

	uint8_t channel_num;
	uint8_t timeout = 16;
	uint32_t i, ch1, ch2;
	uint8_t interpolate, interleave;
	
	if (rgbFramePtr->current_pbuf != rgbFramePtr->last_pbuf)
	{
		/* Channel source buffer (pbuf from ethernet) */
		src1 = rgbGetPBufFramePtr(rgbFramePtr->channels[rgbFramePtr->current_pbuf]);
		src2 = src1 + LEDS_PER_GPIO_CHANNEL * ETH_FRAME_BPP + rgbFramePtr->pbuf_offset * ETH_FRAME_BPP;
		channel_num = rgbGetPBufChannelNum(rgbFramePtr->channels[rgbFramePtr->current_pbuf]);
		ch1 = 2*channel_num;
		ch2 = 2*channel_num + 1;
		
		/* Frame buffer for newest frame */
		dest1 = rgbFramePtr->buffer + (LEDS_PER_GPIO_CHANNEL * ch1) + \
							rgbFramePtr->pbuf_offset * RGB_FRAME_BPP;
		dest2 = rgbFramePtr->buffer + (LEDS_PER_GPIO_CHANNEL * ch2) + \
							rgbFramePtr->pbuf_offset * RGB_FRAME_BPP;;
		
		/* Buffers containing last received frame */
		prev1 = rgbFramePtr->prev->buffer + (LEDS_PER_GPIO_CHANNEL * ch1) + \
							rgbFramePtr->pbuf_offset * RGB_FRAME_BPP;
		prev2 = rgbFramePtr->prev->buffer + (LEDS_PER_GPIO_CHANNEL * ch2) + \
							rgbFramePtr->pbuf_offset * RGB_FRAME_BPP;
		
		dma = (uint32_t *) dmaFillingPtr->buffer + dmaFillingPtr->offset;
		dma--;
		
		while (timeout > 0 && rgbFramePtr->pbuf_offset < LEDS_PER_GPIO_CHANNEL)
		{
			*(dest1) = *(src1 + 1);
			*(dest1 + 1) = *(src1 + 2);
			*(dest1 + 2) = *(src1 + 3);
			
			*(dest2) = *(src2 + 1);
			*(dest2 + 1) = *(src2 + 2);
			*(dest2 + 2) = *(src2 + 3);
			
			*(++dma) |= (interleaveLUT1[(uint8_t) (*(prev1) + (*(dest1) - *(prev1)) / 2)] << ch1);
			*(++dma) |= (interleaveLUT2[(uint8_t) (*(prev1) + (*(dest1) - *(prev1)) / 2)] << ch1);
			*(++dma) |= (interleaveLUT3[(uint8_t) (*(prev1) + (*(dest1) - *(prev1)) / 2)] << ch1);
			*(++dma) |= (interleaveLUT4[(uint8_t) (*(prev1) + (*(dest1) - *(prev1)) / 2)] << ch1);
			
			*(++dma) |= (interleaveLUT1[(uint8_t) (*(prev2) + (*(dest2) - *(prev2)) / 2)] << ch2);
			*(++dma) |= (interleaveLUT2[(uint8_t) (*(prev2) + (*(dest2) - *(prev2)) / 2)] << ch2);
			*(++dma) |= (interleaveLUT3[(uint8_t) (*(prev2) + (*(dest2) - *(prev2)) / 2)] << ch2);
			*(++dma) |= (interleaveLUT4[(uint8_t) (*(prev2) + (*(dest2) - *(prev2)) / 2)] << ch2);
			
			*(++dma) |= (interleaveLUT1[(uint8_t) (*(prev1 + 1) + (*(dest1 + 1) - *(prev1 + 1)) / 2)] << ch1);
			*(++dma) |= (interleaveLUT2[(uint8_t) (*(prev1 + 1) + (*(dest1 + 1) - *(prev1 + 1)) / 2)] << ch1);
			*(++dma) |= (interleaveLUT3[(uint8_t) (*(prev1 + 1) + (*(dest1 + 1) - *(prev1 + 1)) / 2)] << ch1);
			*(++dma) |= (interleaveLUT4[(uint8_t) (*(prev1 + 1) + (*(dest1 + 1) - *(prev1 + 1)) / 2)] << ch1);
			
			*(++dma) |= (interleaveLUT1[(uint8_t) (*(prev2 + 1) + (*(dest2 + 1) - *(prev2 + 1)) / 2)] << ch2);
			*(++dma) |= (interleaveLUT2[(uint8_t) (*(prev2 + 1) + (*(dest2 + 1) - *(prev2 + 1)) / 2)] << ch2);
			*(++dma) |= (interleaveLUT3[(uint8_t) (*(prev2 + 1) + (*(dest2 + 1) - *(prev2 + 1)) / 2)] << ch2);
			*(++dma) |= (interleaveLUT4[(uint8_t) (*(prev2 + 1) + (*(dest2 + 1) - *(prev2 + 1)) / 2)] << ch2);
			
			*(++dma) |= (interleaveLUT1[(uint8_t) (*(prev1 + 2) + (*(dest1 + 2) - *(prev1 + 2)) / 2)] << ch1);
			*(++dma) |= (interleaveLUT2[(uint8_t) (*(prev1 + 2) + (*(dest1 + 2) - *(prev1 + 2)) / 2)] << ch1);
			*(++dma) |= (interleaveLUT3[(uint8_t) (*(prev1 + 2) + (*(dest1 + 2) - *(prev1 + 2)) / 2)] << ch1);
			*(++dma) |= (interleaveLUT4[(uint8_t) (*(prev1 + 2) + (*(dest1 + 2) - *(prev1 + 2)) / 2)] << ch1);
			
			*(++dma) |= (interleaveLUT1[(uint8_t) (*(prev2 + 2) + (*(dest2 + 2) - *(prev2 + 2)) / 2)] << ch2);
			*(++dma) |= (interleaveLUT2[(uint8_t) (*(prev2 + 2) + (*(dest2 + 2) - *(prev2 + 2)) / 2)] << ch2);
			*(++dma) |= (interleaveLUT3[(uint8_t) (*(prev2 + 2) + (*(dest2 + 2) - *(prev2 + 2)) / 2)] << ch2);
			*(++dma) |= (interleaveLUT4[(uint8_t) (*(prev2 + 2) + (*(dest2 + 2) - *(prev2 + 2)) / 2)] << ch2);
			
			dest1 += RGB_FRAME_BPP;
			dest2 += RGB_FRAME_BPP;
			src1 += ETH_FRAME_BPP;
			src2 += ETH_FRAME_BPP;
			rgbFramePtr->pbuf_offset++;
			dmaFillingPtr->offset += DMA_FRAME_BPP;
			timeout--;
		}
		
		if (rgbFramePtr->pbuf_offset >= LEDS_PER_GPIO_CHANNEL)
		{
			pbuf_free(rgbFramePtr->channels[rgbFramePtr->current_pbuf]);
			rgbFramePtr->channels[rgbFramePtr->current_pbuf] = 0;
			rgbFramePtr->current_pbuf++;
			rgbFramePtr->pbuf_offset = 0;
			rgbFramePtr->flags |= (uint16_t) (1 << channel_num);
		}
	} else {
		
		
	}
}

void interpolate_channel(void)
{

}

void interleave_channel(void)
{

}

void run_dma_worker(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET && dmaFillingPtr != dmaFramePtr)
	{
		dmaSendRGBFrame(dmaFramePtr);
		fifoResetFrame(dmaFramePtr->prev);
		dmaFramePtr = dmaFramePtr->next;
		
		dmaStopLEDFrameTimer();
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		dmaInitLEDFrameTimer();
	}
}

void run_receiver(void)
{
	/* check if any packet received */
	if (ETH_CheckFrameReceived())
	{
		LwIP_Pkt_Handle();
	}
	LwIP_Periodic_Handle(LocalTime);
}

struct pbuf *cur_pbuf;
uint32_t *cur_pbuf_offset;
uint32_t pbuf_channel_num;
uint32_t current_interp_step;

void run_frame_checker()
{
	if (rgbFramePtr->start_time != 0)
	{
		if (LocalTime > rgbFramePtr->next->start_time + 10 || rgbFramePtr->next->flags == FRAME_CHANNEL_MASK)
		{
			/* Swap next filling frame */
			fifoResetFrame(rgbFramePtr->prev);
			rgbFramePtr = rgbFramePtr->next;
			cur_pbuf = 0;
			current_interp_step = 0;

			if (rgbFramePtr->prev->sent_frame != 2)
			{
				rgbFramePtr->prev->sent_frame = 2;

				/* Interleave entire frame NOW */
			}
		}
	}
}

void run_pbuf_selector()
{
	uint32_t i;

	if (cur_pbuf == 0)
	{
		for (i = 0; i < 8; i++)
		{
			if (rgbFramePtr->channels[i] != 0 && rgbFramePtr->channel_offset[i] < LEDS_PER_GPIO_CHANNEL)
			{
				cur_pbuf = rgbFramePtr->channels[i];
				cur_pbuf_offset = &(rgbFramePtr->channel_offset[i]);
				pbuf_channel_num = i;
				return;
			}
		}
	}
}

void run_pbuf_worker()
{
	uint32_t timeout = 16;

	if (cur_pbuf != 0)
	{
		/* if there is room left in the DMA frame buffer */
		if (dmaFillingPtr != dmaFramePtr->prev)
		{
			if (rgbFramePtr->sent_frame == 0)
			{
				/* Interpolated frame */
				interpolate_channel();
			} else if (rgbFramePtr->sent_frame == 1) {
				/* Regular frame */
				interleave_channel();
			}
		}
		/* If dma is done filling */
		if (*(cur_pbuf_offset) == LEDS_PER_GPIO_CHANNEL)
		{
			dmaFillingPtr->flags |= 1 << pbuf_channel_num;
			cur_pbuf = 0;
			if (dmaFillingPtr->flags == rgbFramePtr->flags)
			{
				dmaFillingPtr = dmaFillingPtr->next;
				current_interp_step++;
				rgbFramePtr->sent_frame = current_interp_step;
			}
		}
	}
}

/* void run_workers(void)
		Process everything 
		Packet handling in udp_colorserver.c */
void run_workers(void)
{
	uint32_t timeout;

	run_receiver();

	run_dma_worker();

	run_frame_checker();

	// Selects next pbuf to work on
	run_pbuf_selector();

	// Works on pbuf (interpolates and interleaves)
	run_pbuf_worker();
}

void run_lwip_periodic(void)
{
	/* handle periodic timers for LwIP */
	LwIP_Periodic_Handle(LocalTime);
}

/* WORKER INITIALIZATION CODE */
void init_lwip_worker(void)
{
	/* Give a little time for full PHY reset (40-50 ms) */
	sysDisablePHY();
	Delay(5);
	sysEnablePHY();
	
  /* configure ethernet */ 
  ETH_BSP_Config();

  /* Initilaize the LwIP stack */
  LwIP_Init();

  /* UDP echoserver */
  udp_echoserver_init();

	/* UDP Color Framebuffer server */
	udp_colorserver_init();
}

void initialize_workers(void)
{

	rgbFramePtr = fifoInitializeRGBFrames();
	dmaFramePtr = fifoInitializeDMAFrames();
	dmaFillingPtr = dmaFramePtr->next;
	
	//test_interleave_performance(fifoZeroedRGBFrame->first, fifoZeroedDMAFrame->first);
	
	/* Call all of the worker init functions here */
	init_lwip_worker();
	dmaInitLEDFrameTimer();
	dmaStartLEDFrameTimer();
	GPIOE->ODR = 0x0000;
	GPIOF->ODR = 0x0000;
}
