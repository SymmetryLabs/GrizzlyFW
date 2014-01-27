#include "grizzlyapp.h"

extern "C" {
	#include "main.h"
	#include "stm32f4xx.h"
	#include "stm32f4x7_eth.h"
	#include "netconf.h"
	#include "legacy.h"
}


void GrizzlyApp::pushNextWorker()
{

}

void GrizzlyApp::runWorker()
{

}

void GrizzlyApp::processDMA()
{
  /* If timer is up and usouh DMA buffer is ready on queue, send it */
}

void GrizzlyApp::initialize()
{
//	objman = new ObjectManager();
}

void GrizzlyApp::initReceivers() 
{

}

void GrizzlyApp::runReceivers() 
{
  /* check if any packet received */
  if (ETH_CheckFrameReceived())
  {
    if (first_frame_packet_time == 0)
    {
      first_frame_packet_time = LocalTime;
    }
    LwIP_Pkt_Handle();
  }
  if (LocalTime > first_frame_packet_time + 10)
  {
    /* TODO: Add handling for grabbing frame */
  }
  LwIP_Periodic_Handle(LocalTime);
}
