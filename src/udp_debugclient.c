/**
  ******************************************************************************
  * @file    udp_echoclient.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   UDP echo client
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "grizzly.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
typedef struct {
	uint32_t average_time_per_packet;
	uint32_t missed_packets[NUM_UDP_CHANNELS];
} DebugAnalyticsResult;

/* Private define ------------------------------------------------------------*/
/* Number of bytes per debug packet, must be larger than DEBUG_NUM_BUFFERS+12 */
#define	SIZE_OF_DEBUG_PACKET			 400

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void udp_debug_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);

uint8_t   data[SIZE_OF_DEBUG_PACKET];
__IO uint32_t message_count = 0;

DebugMsg debugFIFO[DEBUG_NUM_BUFFERS];
uint32_t debugNextBuffer = 0;

uint32_t debugLastSentTime = 0;

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Connect to UDP echo server
  * @param  None
  * @retval None
  */
void udp_debugclient_connect(void)
{
  struct udp_pcb *upcb;
  struct pbuf *p;
  struct ip_addr DestIPaddr;
  err_t err;
       
  /* Create a new UDP control block  */
  upcb = udp_new();
  
  if (upcb!=NULL)
  {
    /*assign destination IP address */
    IP4_ADDR( &DestIPaddr, DEBUG_IP_ADDR0, DEBUG_IP_ADDR1, DEBUG_IP_ADDR2, DEBUG_IP_ADDR3 );
  
    /* configure destination IP address and port */
    err= udp_connect(upcb, &DestIPaddr, DEBUG_SERVER_PORT);
    
    if (err == ERR_OK)
    {
      /* Set a receive callback for the upcb */
      udp_recv(upcb, udp_debug_receive_callback, NULL);
      
      // sprintf((char*)data, "sending udp client message %d", (int*)message_count);
  
      /* allocate pbuf from pool*/
      p = pbuf_alloc(PBUF_TRANSPORT,SIZE_OF_DEBUG_PACKET, PBUF_POOL);
      
      if (p != NULL)
      {
        /* copy data to pbuf */
        pbuf_take(p, (char*)data, SIZE_OF_DEBUG_PACKET);
          
        /* send udp data */
        udp_send(upcb, p); 
        
        /* free pbuf */
        pbuf_free(p);
      }
      else
      {
        /* free the UDP connection, so we can accept new clients */
        udp_remove(upcb);
        #ifdef SERIAL_DEBUG
        printf("\n\r can not allocate pbuf ");
        #endif
      }
    }
    else
    {
      /* free the UDP connection, so we can accept new clients */
      udp_remove(upcb);
      #ifdef SERIAL_DEBUG
       printf("\n\r can not connect udp pcb");
      #endif
    }
  }
  else
  {
    #ifdef SERIAL_DEBUG
     printf("\n\r can not create udp pcb");
    #endif
  } 
}


/**
  * @brief This function is called when an UDP datagrm has been received on the port UDP_PORT.
  * @param arg user supplied argument (udp_pcb.recv_arg)
  * @param pcb the udp_pcb which received data
  * @param p the packet buffer that was received
  * @param addr the remote IP address from which the packet was received
  * @param port the remote port from which the packet was received
  * @retval None
  */
void udp_debug_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{

  /*increment message count */
  message_count++;
  
  /* Free receive pbuf */
  pbuf_free(p);
  
  /* free the UDP connection, so we can accept new clients */
  udp_remove(upcb);   
}

void debugSendMessages(void)
{
	DebugMsg* dbg = &(debugFIFO[0]);
	
	uint8_t *packet = &(data[0]);
	packet--;
	if (LocalTime > (debugLastSentTime + DEBUG_FLUSH_INTERVAL) )/*|| debugNextMessage >= &(debugFIFO[DEBUG_FLUSH_LIMIT - 1]))*/
	{
		debugLastSentTime = LocalTime;
		/*
		if (debugNextMessage == &(debugFIFO[0]))
			return;
		*/
		memset((void *) packet, 0x00, 100);
		*(++packet) = 0xDE;				// Silly header (0xDEADBEEF)
		*(++packet) = 0xAD;
		*(++packet) = 0xBE;
		*(++packet) = 0xEF;
		*(++packet) = 0x00;				// Size of packet data (including silly header) (always 400)
		*(++packet) = 0x00;
		*(++packet) = 400 >> 8;
		*(++packet) = 400 & 0xFF;
		
		while (dbg != 0)
		{
			/*
			*(++packet) = (dbg->local_time >> 24) & 0xFF;				
			*(++packet) = (dbg->local_time >> 16) & 0xFF;
			*(++packet) = (dbg->local_time >> 8) & 0xFF;
			*(++packet) = (dbg->local_time >> 0) & 0xFF;
			*(++packet) = (dbg->msg >> 24) & 0xFF;				
			*(++packet) = (dbg->msg >> 16) & 0xFF;
			*(++packet) = (dbg->msg >> 8) & 0xFF;
			*(++packet) = (dbg->msg >> 0) & 0xFF;
			*(++packet) = (dbg->extra1 >> 24) & 0xFF;				
			*(++packet) = (dbg->extra1 >> 16) & 0xFF;
			*(++packet) = (dbg->extra1 >> 8) & 0xFF;
			*(++packet) = (dbg->extra1 >> 0) & 0xFF;
			*(++packet) = (dbg->extra2 >> 24) & 0xFF;				
			*(++packet) = (dbg->extra2 >> 16) & 0xFF;
			*(++packet) = (dbg->extra2 >> 8) & 0xFF;
			*(++packet) = (dbg->extra2 >> 0) & 0xFF;
			*(++packet) = (dbg->extra3 >> 24) & 0xFF;				
			*(++packet) = (dbg->extra3 >> 16) & 0xFF;
			*(++packet) = (dbg->extra3 >> 8) & 0xFF;
			*(++packet) = (dbg->extra3 >> 0) & 0xFF;
			*(++packet) = (dbg->extra4 >> 24) & 0xFF;				
			*(++packet) = (dbg->extra4 >> 16) & 0xFF;
			*(++packet) = (dbg->extra4 >> 8) & 0xFF;
			*(++packet) = (dbg->extra4 >> 0) & 0xFF;
			
			dbg = dbg->next;*/
		}
		
		*(++packet) = 0xBE;				// Silly header (0xBEADDEEF)
		*(++packet) = 0xAD;
		*(++packet) = 0xDE;
		*(++packet) = 0xEF;
		
		udp_debugclient_connect();
		/* debugNextMessage = &(debugFIFO[0]); */
	}
}

void debugInitialize(void)
{
	memset((void *) &(debugFIFO[0]), 0x00, DEBUG_NUM_BUFFERS * sizeof(DebugMsg));
	/*
	for (i = 0; i < DEBUG_NUM_BUFFERS; i++)
	{
		if (i + 1 < DEBUG_NUM_BUFFERS)
		{
			debugFIFO[i].next = &(debugFIFO[i+1]);
		}
	}
	
	debugNextMessage = &(debugFIFO[0]); */
}

DebugMsg __dbg__;

void debugPostMessage(uint32_t event_type, uint32_t frame_num, uint32_t channel_flag)
{
	
	__dbg__.local_time = LocalTime;
	__dbg__.channel_flag = channel_flag;
	__dbg__.frame_num = frame_num;
	__dbg__.event_type = event_type;
	debugPostMessageStruct(&__dbg__);
}

DebugMsg* debugPostMessageStruct(DebugMsg* msg)
{
	uint32_t i;
	uint8_t* msg_ptr;
	uint8_t* dbg;
	
	if (debugNextBuffer >= DEBUG_NUM_BUFFERS)
	{
		debugInitialize();
		debugNextBuffer = 0;
	}
	
	msg_ptr = (uint8_t*) msg;
	dbg = (uint8_t*) &(debugFIFO[debugNextBuffer]);
	debugNextBuffer++;
	
	for (i = 0; i < sizeof(DebugMsg); i++)
	{
		*(dbg + i) = *(msg_ptr + i);
	}
	
	return (DebugMsg*) dbg;
}

/*
DebugMsg* debugPostMessage(uint8_t type, uint8_t source, uint16_t msg)
{
	DebugMsg* dbg = debugNextMessage;
	if (dbg == 0)
		return 0;
	dbg->local_time = LocalTime;
	dbg->msg = (type << 24) | (source < 16) | msg;
	dbg->extra1 = 0;
	dbg->extra2 = 0;
	dbg->extra3 = 0;
	dbg->extra4 = 0;
	
	debugNextMessage = dbg->next;
	return dbg;
}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
