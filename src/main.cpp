extern "C" {
 
  #include "main.h"
  #include "bget.h"
	#include "grizzlyapp.h"
	#include "stm32f4x7_eth.h"
	#include "netconf.h"
//	#include "udp_color_server.h"

	#define BOOST_EXCEPTION_DISABLE

  /* Somewhere in header file */
  #define SYSTEMTICK_PERIOD_MS  1

  /* Global C variables */
  __IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 1ms */
  uint32_t timingdelay;

  /**
    * @brief  Inserts a delay time.
    * @param  nCount: number of 10ms periods to wait for.
    * @retval None
    */
  void Delay(uint32_t nCount)
  {
    /* Capture the current local time */
    timingdelay = LocalTime + nCount;  

    /* wait until the desired delay finish */  
    while(timingdelay > LocalTime)
    {     
    }
  }

  /**
    * @brief  Updates the system local time
    * @param  None
    * @retval None
    */
  void Time_Update(void)
  {
    LocalTime += SYSTEMTICK_PERIOD_MS;
  }
	
	
  #ifdef  USE_FULL_ASSERT
  /**
    * @brief  Reports the name of the source file and the source line number
    *   where the assert_param error has occurred.
    * @param  file: pointer to the source file name
    * @param  line: assert_param error line source number
    * @retval None
    */
  void assert_failed(uint8_t* file, uint32_t line)
  {
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {}
  }
	#endif
	
	void *malloc(size_t size) 
  {
    return bget(size);
  }

  void free(void *pointer)
  {
    brel(pointer);
  }
} /* End extern C */

#include "grizzlylib.h"

GrizzlyApp *app;

int main() 
{
  bpool(&bpool_buffer[0], sizeof(bpool_buffer));
  // TODO: Create BPOOL buffer in C file with 

  app = new GrizzlyApp();

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

  /* UDP echoserver */
//  udp_echoserver_init();

	/* UDP Color Framebuffer server */
	udp_colorserver_init();

  /* Initialize all ethernet functions, must be done from extern "C"! */
  // initializeLwIP();

	while (1)
	{
		/* check if any packet received */
		if (ETH_CheckFrameReceived())
		{
			LwIP_Pkt_Handle();
		}
		LwIP_Periodic_Handle(LocalTime);
		// app->runReceivers();
	}
}
