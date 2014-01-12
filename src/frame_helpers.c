#include "stm32f4xx.h"
#include "grizzly.h"

uint8_t* rgbGetPBufFramePtr(struct pbuf *p)
{
	OSCMessage* osc_msg;
	uint8_t* ret_ptr = 0;
	
	osc_msg = (OSCMessage*) p->payload;
	if (OSC_ENDIAN(osc_msg->header1) == 0xDEADBEEF && OSC_ENDIAN(osc_msg->header2) == 0xFEEDBEEF) {
		ret_ptr = &(osc_msg->data_start);
	}
	return ret_ptr;
}

uint8_t rgbGetPBufChannelNum(struct pbuf *p)
{
	OSCMessage* osc_msg;
	
	osc_msg = (OSCMessage*) p->payload;
	
	return osc_msg->packet_num;
}

uint32_t rgbGetPBufFrameNum(struct pbuf *p)
{
	OSCMessage* osc_msg;
	
	osc_msg = (OSCMessage*) p->payload;
	
	return osc_msg->frame_num;
}