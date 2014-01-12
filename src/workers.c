#include "stm32f4x7_eth.h"
#include "netconf.h"
#include "main.h"
#include "udp_echoserver.h"
#include "udp_colorserver.h"
#include "grizzly.h"
#include "fifobufs.h"

// FIFO buffer of zeroed RGB frames (frames ready to be filled with data from ethernet)
 ColorFrameFIFO* fifoZeroedRGBFrame;
// FIFO buffer of zeroed DMA frames (frames ready to be filled with interleaved RGB frames)
 ColorFrameFIFO* fifoZeroedDMAFrame;
// FIFO buffer of fully Received FIFO frames (frames ready for interpolation or interleaving)
 ColorFrameFIFO* fifoReceivedRGBFrame;
// FIFO buffer of partially received frames (frames that need to be fixed with interpolation)
 ColorFrameFIFO* fifoPartialRGBFrame;
// FIFO buffer of interpolated frames (ready to be interleaved)
 ColorFrameFIFO* fifoInterpolatedRGBFrame;
// FIFO buffer of interleaved frames (ready to be sent over DMA)
 ColorFrameFIFO* fifoInterleavedDMAFrame;
// FIFO buffer of RGB frames already sent over DMA (kept for backup)
 ColorFrameFIFO* fifoSentRGBFrame;
// FIFO buffer of RGB and DMA frames ready to be cleaned up
 ColorFrameFIFO* fifoGarbageFrame;

/* 
	receiver:
		takes RGB frame from zeroed fifo to partial fifo
		or fills in frame in partial fifo buffer
			functions: find frame in fifo from frame number
		end: decide whether to push partial to recevied
			functions: pull frame from fifo, newest frame in fifo
			if parial->frame_num < newest_frame->frame_num
				push to received
			if partial->ch flag == 0x00FF
				push to received
	interpolator:
		send to interpolated fifo
	interleaver:
		takes DMA Frame from zeroed fifo to dma interleaved fifo
		(NO INTERMEDIATE POINTERS! use FRAME_INTERLEAVED flag)
			funcs: find next frame in fifo with status, excluding
	DMA worker:
		if timer update interrupt flag is set, disable timer
		and send frame, move RGB frame to sent frame fifo
			funcs: find frame with fifo 


	fifoZeroedRGBFrame
			| by receiver worker
	fifoPartialRGBFrame
			| by receiver worker
	fifoReceivedRGBFrame
			| by DMA worker
	fifoSentRGBFrame
			| by DMA worker
	fifoGarbageFrame

interpolator worker creates ColorFrame in fifoInterpolatedRGBFrame
	from fram in fifoReceivedFrame and fifoSentFrame

	fifoZeroedDMAFrame
			| by interleaver worker
	fifoInterleavedDMAFrame
			| by DMA worker
	fifoGarbageFrame

*/

extern void Delay(uint32_t nCount);
extern __IO uint32_t LocalTime;

static const uint16_t MortonTable256[256] = 
{
  0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015, 
  0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055, 
  0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115, 
  0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155, 
  0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415, 
  0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455, 
  0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515, 
  0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555, 
  0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015, 
  0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055, 
  0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115, 
  0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155, 
  0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415, 
  0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455, 
  0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515, 
  0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555, 
  0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015, 
  0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055, 
  0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115, 
  0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155, 
  0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415, 
  0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455, 
  0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515, 
  0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555, 
  0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015, 
  0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055, 
  0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115, 
  0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155, 
  0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415,  
  0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455, 
  0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515, 
  0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555
};


#define INTERLEAVE(X, Y)  MortonTable256[Y >> 8]   << 17 |  \
                          MortonTable256[X >> 8]   << 16 |  \
                          MortonTable256[Y & 0xFF] <<  1 |  \
                          MortonTable256[X & 0xFF]
													


void interleave_frame(ColorFrame *frame, ColorFrame* dma)
{
  /*uint16_t tshort[8];
  uint32_t tlong[4];
  volatile uint8_t* cptr = 0;
	volatile uint8_t *interleave_dmaptr = 0;
  uint8_t i = 0;
  
  cptr = frame->buffer;
  cptr += frame->offset * RGB_FRAME_BPP;
  
  interleave_dmaptr = dma->buffer;
  interleave_dmaptr += dma->offset * DMA_FRAME_BPP;
  dma->offset++;
  frame->offset++;
  for ( i = 0; i<3; i++)
  {
    tshort[0] = (*(cptr + LEDS_PER_CHANNEL * RGB_FRAME_BPP * 0) << 8) | *(cptr + LEDS_PER_CHANNEL * RGB_FRAME_BPP * 8);
    tshort[1] = (*(cptr + LEDS_PER_CHANNEL * RGB_FRAME_BPP * 1) << 8) | *(cptr + LEDS_PER_CHANNEL * RGB_FRAME_BPP * 9);
    tshort[2] = (*(cptr + LEDS_PER_CHANNEL * RGB_FRAME_BPP * 2) << 8) | *(cptr + LEDS_PER_CHANNEL * RGB_FRAME_BPP * 10);
    tshort[3] = (*(cptr + LEDS_PER_CHANNEL * RGB_FRAME_BPP * 3) << 8) | *(cptr + LEDS_PER_CHANNEL * RGB_FRAME_BPP * 11);
    tshort[4] = (*(cptr + LEDS_PER_CHANNEL * RGB_FRAME_BPP * 4) << 8) | *(cptr + LEDS_PER_CHANNEL * RGB_FRAME_BPP * 12);
    tshort[5] = (*(cptr + LEDS_PER_CHANNEL * RGB_FRAME_BPP * 5) << 8) | *(cptr + LEDS_PER_CHANNEL * RGB_FRAME_BPP * 13);
    tshort[6] = (*(cptr + LEDS_PER_CHANNEL * RGB_FRAME_BPP * 6) << 8) | *(cptr + LEDS_PER_CHANNEL * RGB_FRAME_BPP * 14);
    tshort[7] = (*(cptr + LEDS_PER_CHANNEL * RGB_FRAME_BPP * 7) << 8) | *(cptr + LEDS_PER_CHANNEL * RGB_FRAME_BPP * 15);
    
    tlong[0] = INTERLEAVE(tshort[0], tshort[4]);
    tlong[1] = INTERLEAVE(tshort[1], tshort[5]);
    tlong[2] = INTERLEAVE(tshort[2], tshort[6]);
    tlong[3] = INTERLEAVE(tshort[3], tshort[7]);
    
    tshort[0] = tlong[0] >> 16;
    tshort[1] = tlong[1] >> 16;
    tshort[2] = tlong[2] >> 16;
    tshort[3] = tlong[3] >> 16;
    tshort[4] = tlong[0] & 0xFFFF;
    tshort[5] = tlong[1] & 0xFFFF;
    tshort[6] = tlong[2] & 0xFFFF;
    tshort[7] = tlong[3] & 0xFFFF;
    
    tlong[0] = INTERLEAVE(tshort[0], tshort[2]);
    tlong[1] = INTERLEAVE(tshort[1], tshort[3]);
    tlong[2] = INTERLEAVE(tshort[4], tshort[6]);
    tlong[3] = INTERLEAVE(tshort[5], tshort[7]);
    
    tshort[0] = tlong[0] >> 16;
    tshort[1] = tlong[1] >> 16;
    tshort[2] = tlong[0] & 0xFFFF;
    tshort[3] = tlong[1] & 0xFFFF;
    tshort[4] = tlong[2] >> 16;
		tshort[5] = tlong[3] >> 16;
    tshort[6] = tlong[2] & 0xFFFF;
    tshort[7] = tlong[3] & 0xFFFF;
    
    tlong[0] = INTERLEAVE(tshort[0], tshort[1]);
    tlong[1] = INTERLEAVE(tshort[2], tshort[3]);
    tlong[2] = INTERLEAVE(tshort[4], tshort[5]);
    tlong[3] = INTERLEAVE(tshort[6], tshort[7]);
		
		*(  interleave_dmaptr) = (tlong[0] >> 24)& 0xFF;
		*(++interleave_dmaptr) = (tlong[2] >> 24)& 0xFF;
		*(++interleave_dmaptr) = (tlong[0] >> 16)& 0xFF;
		*(++interleave_dmaptr) = (tlong[2] >> 16)& 0xFF;
		*(++interleave_dmaptr) = (tlong[0] >> 8)& 0xFF;
		*(++interleave_dmaptr) = (tlong[2] >> 8)& 0xFF;
		*(++interleave_dmaptr) = (tlong[0] >> 0)& 0xFF;
		*(++interleave_dmaptr) = (tlong[2] >> 0)& 0xFF;
		*(++interleave_dmaptr) = (tlong[1] >> 24)& 0xFF;
		*(++interleave_dmaptr) = (tlong[3] >> 24)& 0xFF;
		*(++interleave_dmaptr) = (tlong[1] >> 16)& 0xFF;
		*(++interleave_dmaptr) = (tlong[3] >> 16)& 0xFF;
		*(++interleave_dmaptr) = (tlong[1] >> 8)& 0xFF;
		*(++interleave_dmaptr) = (tlong[3] >> 8)& 0xFF;
		*(++interleave_dmaptr) = (tlong[1] >> 0)& 0xFF;
		*(++interleave_dmaptr) = (tlong[3] >> 0)& 0xFF;
    
    interleave_dmaptr++;
    cptr++;
  }*/
}

void process_rgb_frame(void)
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

void run_receiver(void)
{
	/* check if any packet received */
	if (ETH_CheckFrameReceived())
	{
		LwIP_Pkt_Handle();
		END_WORKER(receiver);
	}
	LwIP_Periodic_Handle(LocalTime);
}

/* void run_workers(void)
		Process everything 
		Packet handling in udp_colorserver.c */
void run_workers(void)
{	
	uint32_t start_time = LocalTime;
	uint32_t timeout;

	while (LocalTime < start_time + 10 && rgbFramePtr->next->flags != FRAME_CHANNEL_MASK)
	{
		run_receiver();
		
		/* Call rgb frame processing functions */
		run_inner_frame_worker();
		
		run_dma_worker();
	}
	while (rgbFramePtr->offset < LEDS_PER_GPIO_CHANNEL)
	{
		for (timeout = 6; timeout > 0; timeout--)
		{
			/* Call rgb frame processing functions */
		}
	}
	dmaFramePtr->flags = rgbFramePtr->flags;
	dmaFramePtr->offset = 0;
	rgbFramePtr->offset = 0;
	rgbFramePtr->flags = 0;
	fifoResetFrame(rgbFramePtr->prev);
	rgbFramePtr = rgbFramePtr->next;
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
