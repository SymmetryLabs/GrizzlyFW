extern "C" {
#include <stm32f4xx.h>
#include <string.h>
#include <main.h>
#include <grizzly.h>
}

#include <vector>

using namespace std;

#define SYSTEMTICK_PERIOD_MS  1

__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 1ms */
uint32_t timingdelay;

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)

{
  /*!< At this stage the microcontroller clock setting is already configured to 
       168 MHz, this is done through SystemInit() function which is called from
       startup file (startup_stm32f4xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f4xx.c file
     */
		 
	vector<uint32_t> x;
	
	sysConfigDebug(1);
	sysConfigSysTick();
	sysConfigRCC();
	
	gpioConfigEthernet();
	gpioConfigLEDOutput();
	
	//dmaInitializeFrameTimer();
	
	/* initialize_workers(); */

  /* Infinite loop */
  while (1)
  {
		/* run_workers(); */
  } 
}

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


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
