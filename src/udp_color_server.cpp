/**
  ******************************************************************************
  * @file    udp_echoserver.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   UDP echo server
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
extern "C" {
	#include "main.h"
	#include "lwip/pbuf.h"
	#include "lwip/udp.h"
	#include "lwip/tcp.h"
	
}

#include <string.h>
#include <stdio.h>
#include "grizzly_types.h"
#include "grizzly_buffers.h"
#include "grizzlyapp.h"
#include "OscReceivedElements.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define     UDP_COLOR_SERVER_PORT    779   /* define the UDP local connection port */

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
extern "C" void udp_colorserver_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);

/* Private functions ---------------------------------------------------------*/
/**
	* @brief  Initialize the server application.
	* @param  None
	* @retval None
	*/
void udp_colorserver_init(GrizzlyApp* app)
{
	
	 struct udp_pcb *upcb;
	 err_t err;
	 
	 /* Create a new UDP control block  */
	 upcb = udp_new();
		
	 if (upcb)
	 {
		 /* Bind the upcb to the UDP_PORT port */
		 /* Using IP_ADDR_ANY allow the upcb to be used by any local interface */
			err = udp_bind(upcb, IP_ADDR_ANY, UDP_COLOR_SERVER_PORT);
			
			if(err == ERR_OK)
			{
				/* Set a receive callback for the upcb */
				udp_recv(upcb, udp_colorserver_receive_callback, app);
			}
			else
			{
				udp_remove(upcb);
				printf("can not bind pcb");
			}	   
	 }
	 else
	 {
		 printf("can not create pcb");
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
	extern "C" void udp_colorserver_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
	{
		
		/*
		uint32_t frame_num;
		uint32_t channel_num;

		ColorFrame* frame;

		/* free the UDP connection, so we can accept new clients 
		udp_disconnect(upcb);
		
		src = rgbGetPBufFramePtr(p);
		if (src != 0) {
			frame_num = rgbGetPBufFrameNum(p);
			channel_num = rgbGetPBufChannelNum(p);
			frame = rgbFramePtr->next;
			
			if (frame->start_time == 0)
			{
				frame->start_time = LocalTime;
			}
			frame->channels[channel_num] = p;
			pbuf_ref(p);
			frame->channel_offset[channel_num] = 0;
			frame->flags |= 1 << channel_num;
		}
		
		/* Free the p buffer */
		GrizzlyApp* app = (GrizzlyApp*) arg;
		
		osc::ReceivedPacket osc_packet((char *) p->payload, p->len);
		osc::ReceivedMessage osc_msg(osc_packet);
		
		if (std::strcmp( osc_msg.AddressPattern(), "/shady/pointbuffer" ) == 0)
		{
			
			osc::ReceivedMessage::const_iterator arg = osc_msg.ArgumentsBegin();
			uint32_t frame_num = (arg++)->AsInt32();
			uint32_t packet_num = (arg++)->AsInt32();
			uint32_t num_channels = (arg++)->AsInt32();

			const char* ch_type[num_channels];
			uint32_t ch_num[num_channels];
			uint32_t ch_offset[num_channels];
			uint32_t ch_size[num_channels];
			for (uint32_t i = 0; i < num_channels; i++)
			{
				ch_type[i] = (arg++)->AsString();
				ch_num[i] = (arg++)->AsInt32();
				ch_offset[i] = (arg++)->AsInt32();
				ch_size[i] = (arg++)->AsInt32();
		
			}
			uint32_t total_size = (arg++)->AsInt32();
			const void* data;
			osc::osc_bundle_element_size_t data_size;
			(arg++)->AsInt32Unchecked();
			for (int i = 0; i < num_channels; i++)
			{
				if (std::strcmp(ch_type[i], "argb") == 0)
				{
					pbuf_ref(p);
					std::shared_ptr<pbuf> pbuf_ptr(p, pbuf_free);
					ObjectId obj_id = BufferFactory<ARGBFormat>::allocateBuffer(app->objman, pbuf_ptr, &*(arg), &*(arg) + ch_offset[i]*sizeof(ARGBFormat));
					app->objman->pushPendingBuffer(ch_num[i], obj_id, "argb");

					/*
					auto buf = app->objman->allocateBuffer<RGBFormat>(ch_size[i]);
					auto buf_ptr = app->objman->getBuffer<RGBFormat>(buf);
					auto void_ptr = const_cast<void*>(data);
					auto format_ptr = static_cast<ARGBFormat*>(void_ptr);

					for (auto ite = buf_ptr->begin(); ite != buf_ptr->end(); ite++)
					{
						ElementConverter<ARGBFormat, RGBFormat>::convertElement(format_ptr++, ite);
						// *(ite++) = *(format_ptr++);
					}
					printBufferElements<RGBFormat, uint8_t, int>(buf_ptr->end()-3, buf_ptr->end());*/
				}
			}
		}
		
		udp_disconnect(upcb);
		pbuf_free(p);
		/* WILL FREE PBUFS AFTER INTERPOLATION! */
	}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
