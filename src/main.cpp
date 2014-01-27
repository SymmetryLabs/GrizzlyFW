extern "C" {
 
  #include "main.h"
  #include "bget.h"
	#include "stm32f4x7_eth.h"
	#include "netconf.h"
} /* End extern C */

#include "grizzlyapp.h"

extern void udp_colorserver_init(GrizzlyApp*);

int main() 
{
  bpool(&bpool_buffer[0], sizeof(bpool_buffer));
  // TODO: Create BPOOL buffer in C file with 

  GrizzlyApp* app = new GrizzlyApp();
	
	app->initialize();

  /* All of these functions are unrelated to C callbacks, no extern */
	app->sysConfigDebug(1);
	app->sysConfigSysTick();
	app->sysConfigRCC();
  app->gpioConfigEthernet();
	app->gpioConfigLEDOutput();
	
	app->sysDisablePHY();
	Delay(5);
	app->sysEnablePHY();
	
	/* configure ethernet */ 
  ETH_BSP_Config();

  /* Initilaize the LwIP stack */
  LwIP_Init();

  /* UDP echoserver
      TODO: Reimplement for command server/pings
   */
//  udp_echoserver_init();

	/* UDP Color Framebuffer server */
	udp_colorserver_init(app);

	while (1)
	{
		app->runReceivers();
    app->runWorker();
    app->processDMA();
	}
}
