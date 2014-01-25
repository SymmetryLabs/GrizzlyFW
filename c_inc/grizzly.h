/* --------------------------------------------------------------------------*/
/* Define to prevent recursive inclusion ------------------------------------*/
/* --------------------------------------------------------------------------*/
#ifndef __GRIZZLY_H
#define __GRIZZLY_H

/* ==========================================================================
																ENABLE DEBUG
   ==========================================================================*/
#define				FIFO_DEBUG

#define DEBUG_IP_ADDR0							192
#define DEBUG_IP_ADDR1							168
#define DEBUG_IP_ADDR2							88
#define DEBUG_IP_ADDR3							69
#define	DEBUG_SERVER_PORT						936

/* Number of DebugMsg structs in buffer */
#define DEBUG_NUM_BUFFERS 					1

/* Number of Systicks to wait before flushing debug buffers over UDP */
#define DEBUG_FLUSH_INTERVAL				50

/* Defines number of buffers that should be full before buffers flush early */
#define	DEBUG_FLUSH_LIMIT						30

/* ==========================================================================
									   BUFFER AND LED CHANNEL CONFIGURATION
   ==========================================================================*/
#define							NUM_UDP_CHANNELS								8
#define							LEDS_PER_UDP_CHANNEL						256
#define							NUM_GPIO_CHANNELS								16
#define							LEDS_PER_GPIO_CHANNEL						128
#define							NUM_RGB_BUFFERS									3
#define							NUM_DMA_BUFFERS									5
#define 						NUM_FIFO_BUFFERS								8
#define							NUM_INTERP_FRAMES								3

#define							ETH_FRAME_BPP										4
#define							RGB_FRAME_BPP										3
#define							DMA_FRAME_BPP										48

#define							MAX_FRAME_LAG									  20			// In SysTicks (ms)

// Bit mask that matches ColorFrame->flags when all channels are set
#define							FRAME_CHANNEL_MASK							0x00FF

// Frame has been zeroed
#define							FRAME_ZEROED										((uint8_t) 1)
// Frame has only been partially received before a new frame was started
#define 						FRAME_PARTIAL										((uint8_t) 1 << 1)
// Frame has been filled by receiver and is ready to be interpolated/interleaved
#define							FRAME_RECEIVED										((uint8_t) 1 << 2)
// Frame has been interpolated and is ready to be interleaved into DMA buffer
#define							FRAME_INTERPOLATED								((uint8_t)	1 << 3) 
// Frame has been interleaved and is ready to be sent over DMA
#define							FRAME_INTERLEAVED									((uint8_t) 1 << 4)
// Frame has been sent to DMA (set by DMA interrupt) and is waiting for next interpolation
#define 						FRAME_SENT 											((uint8_t) 1 << 5)
// Frame has been used for the next received frame's interpolation
#define							FRAME_REINTERPOLATED						((uint8_t) 1 << 6)
// Frame has been used for next interpolation and is ready for clean up and reuse
#define 						FRAME_GARBAGE											((uint8_t) 1 << 7)

// Frame is unlocked
#define 						FRAME_UNLOCKED										0
// Frame is locked by receiver
#define 						FRAME_RECEIVER										1
// Frame is locked by packet fixer (interpolates missing channels in the frame)
#define 						FRAME_PARTIAL_FIX											2
// Frame is locked by color interpolator
#define 						FRAME_INTERPOLATOR									3
// Frame is locked by bit interleaver
#define 						FRAME_INTERLEAVER									4
// Frame is locked by DMA transfer
#define 						FRAME_DMA 											5
// Frame is locked by garbage collector
#define 						FRAME_CLEANUP										7

// Frame is not set for any use
#define							FRAME_TYPE_UNUSED								0
// Frame is used to store LED RGB values
#define							FRAME_TYPE_RGB									1
// Frame is used to store GPIO output values for DMA
#define							FRAME_TYPE_DMA									2

/* --------------------------------------------------------------------------*/
/* Includes -----------------------------------------------------------------*/
/* --------------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "fifobufs.h"
#include "pbuf.h"

/* --------------------------------------------------------------------------*/
/* Defines ------------------------------------------------------------------*/
/* --------------------------------------------------------------------------*/
/* SysTick Defines */
#define				SYSTICK_IRQ_PRIORITY														0
#define				SYSTICK_IRQ_SUBPRIORITY													0
#define				SYSTICK_IRQ_FREQUENCY														100

/* Ethernet Defines */
#define DP83848_PHY_ADDRESS       0x01 /* Relative to STM324xG-EVAL Board */

/* Specific defines for EXTI line, used to manage Ethernet link status */
#define ETH_LINK_EXTI_LINE             EXTI_Line14
#define ETH_LINK_EXTI_PORT_SOURCE      EXTI_PortSourceGPIOB
#define ETH_LINK_EXTI_PIN_SOURCE       EXTI_PinSource14
#define ETH_LINK_EXTI_IRQn             EXTI15_10_IRQn 
/* PB14 */
#define ETH_LINK_PIN                   GPIO_Pin_14
#define ETH_LINK_GPIO_PORT             GPIOB
#define ETH_LINK_GPIO_CLK              RCC_AHB1Periph_GPIOB

/* Ethernet Flags for EthStatus variable */   
#define ETH_INIT_FLAG           0x01 /* Ethernet Init Flag */
#define ETH_LINK_FLAG           0x10 /* Ethernet Link Flag */

/* FIFO IDs */
#define								FIFO_ID_ZERO_RGB									1
#define								FIFO_ID_ZERO_DMA									2
#define								FIFO_ID_RECEIVED									3
#define								FIFO_ID_PARTIAL										4
#define								FIFO_ID_INTERPOLATED							5
#define								FIFO_ID_INTERLEAVED								6
#define								FIFO_ID_SENT											7
#define								FIFO_ID_GARBAGE										8

/* ==================== UDP DEBUG CLIENT DEFINES ==================== */
/* DEBUG MESSAGE TYPE */
#define								DBG_INFO													1 << 0
#define								DBG_WARNING												1 << 1
#define								DBG_ERROR													1 << 2
#define								DBG_FATAL													1 << 3

/* DEBUG MESSAGE SOURCE */
/* Microcontroller sources (not dealing with LEDs) */
#define								DBG_SYSTEM												1
#define								DBG_INIT													2
#define								DBG_LWIP													3
#define								DBG_FIFO													4
/* Worker sources (works with RGB and FMA frames) */
#define								DBG_RECEIVER											100
#define								DBG_INTERPOLATOR									101
#define								DBG_INTERLEAVER										102
#define								DBG_DMA														103
#define							  DBG_GC														104

/* Debug events */
#define EVENT_ZERO_RGB 0x01
#define	EVENT_ZERO_DMA 0x02
#define EVENT_RECEIVE_START 0x10
#define EVENT_RECEIVE_CHANNEL 0x11
#define EVENT_RECEIVE_COMPLETE 0x12
#define EVENT_RECEIVE_HEADER_ERROR 0x1A
#define EVENT_RECEIVE_NO_NEW_FRAME_POPPED 0x1B
#define EVENT_RECEIVE_NO_NEW_FRAME_PUSHED 0x1C
#define	EVENT_RECEIVE_LATE_PACKET	0x1D
#define EVENT_INTERLEAVE_START 0x20
#define EVENT_INTERLEAVE_END 0x21
#define EVENT_INTERLEAVE_NO_NEW_RGB_FRAME 0x2A
#define EVENT_INTERLEAVE_NO_NEW_DMA_FRAME 0x2B

// #define EVENT_

#define			OSC_ENDIAN(num)				((num>>24)&0xff) | ((num<<8)&0xff0000) | ((num>>8)&0xff00) | ((num<<24)&0xff000000)

extern __IO uint32_t LocalTime;

typedef struct {
	uint8_t osc_url[20];
	uint32_t reserved[3];
	uint32_t frame_num;
	uint32_t header1;
	uint32_t packet_num;
	uint32_t header2;
	uint32_t data_length;
	uint32_t data_check;
	uint8_t data_start;
} OSCMessage;

typedef struct DebugMsg DebugMsg;

struct DebugMsg {
	uint32_t local_time;
	uint32_t event_type;
	uint32_t frame_num;
	uint32_t channel_flag;
};

typedef struct {
	uint32_t start_time;
	uint32_t total_time;
	uint32_t total_calls;
	uint32_t frame_start_time;
	uint32_t frame_total_time;
	uint32_t total_frames;
} debugWorkerStats;

typedef struct {
	struct {
		uint32_t received_frames;
		uint32_t received_packets;
		uint32_t dropped_packets;
		uint32_t interpolated_frames;
		uint32_t interleaved_frames;
		uint32_t last_frame_num;
	} count;
	debugWorkerStats receiver;
	debugWorkerStats interpolator;
	debugWorkerStats interleaver;
	debugWorkerStats garbage_collector;
	
} tDebugStats;

extern tDebugStats debugStats;

#define START_WORKER(__worker) \
			debugStats.__worker.start_time = LocalTime;
			
#define START_WORKER_FRAME(__worker) \
			debugStats.__worker.frame_start_time = LocalTime;

#define END_WORKER_FRAME(__worker) \
			debugStats.__worker.frame_total_time += LocalTime - debugStats.__worker.frame_start_time; \
			debugStats.__worker.total_frames++

#define END_WORKER(__worker) \
			debugStats.__worker.total_time += LocalTime - debugStats.__worker.start_time; \
			debugStats.__worker.total_calls++
			
#define DROPPED_PACKET() \
			debugStats.count.dropped_packets++

#define RECEIVED_PACKET() \
			debugStats.count.received_packets++

#define RECEIVED_FRAME(frame_num) \
			debugStats.count.received_frames++; \
			debugStats.count.last_frame_num = frame_num
			

// FIFO buffer of zeroed RGB frames (frames ready to be filled with data from ethernet)
extern ColorFrameFIFO* fifoZeroedRGBFrame;
// FIFO buffer of zeroed DMA frames (frames ready to be filled with interleaved RGB frames)
extern ColorFrameFIFO* fifoZeroedDMAFrame;
// FIFO buffer of fully Received FIFO frames (frames ready for interpolation or interleaving)
extern ColorFrameFIFO* fifoReceivedRGBFrame;
// FIFO buffer of partially received frames (frames that need to be fixed with interpolation)
extern ColorFrameFIFO* fifoPartialRGBFrame;
// FIFO buffer of interpolated frames (ready to be interleaved)
extern ColorFrameFIFO* fifoInterpolatedRGBFrame;
// FIFO buffer of interleaved frames (ready to be sent over DMA)
extern ColorFrameFIFO* fifoInterleavedDMAFrame;
// FIFO buffer of RGB frames already sent to DMA (kept for backup)
extern ColorFrameFIFO* fifoSentRGBFrame;
// FIFO buffer of RGB and DMA frames ready to be cleaned up
extern ColorFrameFIFO* fifoGarbageFrame;

extern const uint32_t interleaveLUT1[256];
extern const uint32_t interleaveLUT2[256];
extern const uint32_t interleaveLUT3[256];
extern const uint32_t interleaveLUT4[256];

/* --------------------------------------------------------------------------*/
/* Global Configuration Functions -------------------------------------------*/
/* --------------------------------------------------------------------------*/
/* Configure debug registers */
extern void sysConfigDebug(uint8_t enable);;
/* Configure SysTick interrupt for LwIP */
extern void sysConfigSysTick(void);
/* Configure RCC clocks for peripherals */
extern void sysConfigRCC(void);
/* Configure MAC */
// void sysConfigMAC(void);

extern ColorFrame* rgbFramePtr;
extern ColorFrame* dmaFramePtr;
extern ColorFrame* dmaFillingPtr;

/* Configure Ethernet GPIO */
extern void gpioConfigEthernet(void);
extern void sysEnablePHY(void);
extern void sysDisablePHY(void);

/* Configure LED output and level translator control GPIO */
extern void gpioConfigLEDOutput(void);

/* Initialize all RGB, interpolation, and DMA buffers */
extern void fifoInitializeFrames(void);
extern void fifoInitializeDummyFrame(ColorFrame* frame);

/* Initialize all workers */
extern void initialize_workers(void);

/* Run all workers that work the FIFOs */
extern void run_workers(void);

/* LED output DMA functions */
extern void dmaInitializeRGBData(ColorFrame* frame);
extern void dmaInitializeClockRising(void);
extern void dmaInitializeClockFalling(void);
extern void dmaSendRGBFrame(ColorFrame *frame);
extern void dmaInitializeFrameTimer(void);
extern void dmaInitLEDFrameTimer(void);
extern void dmaStartLEDFrameTimer(void);
extern void dmaStopLEDFrameTimer(void);

extern uint8_t* rgbGetPBufFramePtr(struct pbuf *p);
extern uint32_t rgbGetPBufFrameNum(struct pbuf *p);
extern uint8_t rgbGetPBufChannelNum(struct pbuf *p);

extern void debugInitialize(void);
/* extern DebugMsg* debugPostMessage(uint8_t type, uint8_t source, uint16_t msg); */
extern DebugMsg* debugPostMessageStruct(DebugMsg* msg);
extern void debugSendMessages(void);
extern void debugPostMessage(uint32_t event_type, uint32_t frame_num, uint32_t channel_flag);
#endif /* __GRIZZLY_H */
