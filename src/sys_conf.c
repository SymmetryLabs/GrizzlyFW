#include "stm32f4xx.h"
#include "grizzly.h"
#include "netif.h"
#include "stm32f4x7_eth.h"

void sysConfigDebug(uint8_t enable)
{
	if (enable > 0) {
		// Config TIM1 to stop ticking on debug halt
		DBGMCU->APB2FZ |= 0x01;
	} else {
		/* Config TIM1 to continue running on debug halt */
		DBGMCU->APB2FZ &= ~0x01;
	}
}

void sysConfigSysTick(void)
{
	RCC_ClocksTypeDef RCC_Clocks;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	/* Configure Systick clock source as HCLK */
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

  /* SystTick configuration: an interrupt every 10ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / SYSTICK_IRQ_FREQUENCY);
	
	NVIC_InitStructure.NVIC_IRQChannel = (uint8_t) SysTick_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SYSTICK_IRQ_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = SYSTICK_IRQ_SUBPRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void sysConfigRCC(void)
{
	/* Enable SYSCFG clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	/* DMA clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 , ENABLE);

  /* TIM1 clock enable for DMA transfer requests */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
	/* TIM1 clock enable for DMA frame start interrupts */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	/* Enable RCC clocks for all GPIOs */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC_Tx, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC_Rx, ENABLE);
}

void sysDisablePHY(void)
{
	/* Hold PG4 low to power down PHY */
	GPIOG->ODR &= ~GPIO_Pin_4;
}

void sysEnablePHY(void)
{
	/* Hold PG4 high to enable PHY */
  GPIOG->ODR |= GPIO_Pin_4;
}

/*
void sysConfigMAC(void)
{
	$%^ Reset ETHERNET on AHB Bus ^%$
  ETH_DeInit();

  $%^ Software reset ^%$
  ETH_SoftwareReset();

  $%^ Wait for software reset ^%$
  while (ETH_GetSoftwareResetStatus() == SET);

  $%^ ETHERNET Configuration --------------------------------------------------^%$
  $%^ Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter ^%$
  ETH_StructInit(&ETH_InitStructure);

  $%^ Fill ETH_InitStructure parametrs ^%$
  $%^------------------------   MAC   -----------------------------------^%$
  ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
//  ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Disable; 
//  ETH_InitStructure.ETH_Speed = ETH_Speed_10M;
//  ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;   

  ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
  ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
  ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
  ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
  ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
  ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
  ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
  ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
#ifdef CHECKSUM_BY_HARDWARE
  ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
#endif

  $%^------------------------   DMA   -----------------------------------^%$  
  
  $%^ When we use the Checksum offload feature, we need to enable the Store and Forward mode: 
  the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can insert/verify the checksum, 
  if the checksum is OK the DMA can handle the frame otherwise the frame is dropped ^%$
  ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
  ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
  ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;

  ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;
  ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;
  ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
  ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
  ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;
  ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
  ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
  ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

  $%^ Configure Ethernet ^%$
  EthStatus = ETH_Init(&ETH_InitStructure, DP83848_PHY_ADDRESS);
}*/

