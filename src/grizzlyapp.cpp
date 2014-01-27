#include "grizzlylib.h"
#include "grizzlyapp.h"

extern "C" {
	#include "main.h"
	#include "stm32f4xx.h"
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
  /* If timer is up and DMA buffer is ready on queue, send it */
}

void GrizzlyApp::initialize()
{
//	objman = new ObjectManager();
}

void GrizzlyApp::sysConfigRCC() 
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

void GrizzlyApp::sysConfigSysTick() 
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

void GrizzlyApp::sysConfigDebug(uint8_t enable) 
{
    if (enable > 0) {
        // Config TIM1 to stop ticking on debug halt
        DBGMCU->APB2FZ |= 0x01;
    } else {
        /* Config TIM1 to continue running on debug halt */
        DBGMCU->APB2FZ &= ~0x01;
    }
}

void GrizzlyApp::sysDisablePHY()
{
    /* Hold PG4 low to power down PHY */
    GPIOG->ODR &= ~GPIO_Pin_4;
}

void GrizzlyApp::sysEnablePHY()
{
    /* Hold PG4 high to enable PHY */
  GPIOG->ODR |= GPIO_Pin_4;
}

void GrizzlyApp::gpioConfigEthernet() 
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure PG4 for PHY RESET# */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    /* Ethernet pins configuration ************************************************/
    /*
        ETH_MDIO -------------------------> PA2  -  SAME
        ETH_MDC --------------------------> PC1  -  SAME
        ETH_PPS_OUT ----------------------> PB5  -  NONE
        ETH_MII_CRS ----------------------> PH2  -  DIFFERENT PA0
        ETH_MII_COL ----------------------> PH3  -  DIFFERENT PA3
        ETH_MII_RX_ER --------------------> PI10 -  DIFFERENT PB10
        ETH_MII_RXD2 ---------------------> PH6  -  DIFFERENT PB0
        ETH_MII_RXD3 ---------------------> PH7  -  DIFFERENT PB1
        ETH_MII_TX_CLK -------------------> PC3  -  SAME
        ETH_MII_TXD2 ---------------------> PC2  -  SAME
        ETH_MII_TXD3 ---------------------> PB8  -  SAME
        ETH_MII_RX_CLK/ETH_RMII_REF_CLK---> PA1  -  SAME
        ETH_MII_RX_DV/ETH_RMII_CRS_DV ----> PA7  -  SAME
        ETH_MII_RXD0/ETH_RMII_RXD0 -------> PC4  -  SAME
        ETH_MII_RXD1/ETH_RMII_RXD1 -------> PC5  -  SAME
        ETH_MII_TX_EN/ETH_RMII_TX_EN -----> PG11 -  DIFFERENT PB11
        ETH_MII_TXD0/ETH_RMII_TXD0 -------> PG13 -  DIFFERENT PB12
        ETH_MII_TXD1/ETH_RMII_TXD1 -------> PG14 -  DIFFERENT PB13
        
        FIX PH2, PH3, PI10, PH6, PH7, PG11, PG13, PG14
                                                  */

    /* Configure PA1, PA2 and PA7 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);

    /* Configure PB5 and PB8 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_ETH);

    /* Configure PC1, PC2, PC3, PC4 and PC5 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource2, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);
}

void GrizzlyApp::gpioConfigLEDOutput() 
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure LED Data and clock out lines */
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | \
                                    GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;/* | \
                                    GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | \
                                    GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;*/
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    GPIOE->ODR = 0x00;
    GPIOF->ODR = 0x00;

    /* Level translator 1 & 2 Output Enable (active low) */
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Reset level translator 1 & 2 for some cycles */
    GPIO_SetBits(GPIOB, GPIO_Pin_15);

    /* Level translator 1 & 2 Direction (active low) */
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOC, GPIO_Pin_8);
    GPIO_ResetBits(GPIOA, GPIO_Pin_5 | GPIO_Pin_6);
    GPIO_ResetBits(GPIOB, GPIO_Pin_14);
    GPIO_ResetBits(GPIOB, GPIO_Pin_15);
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

uint16_t uhTimerPrescaler = 0;

void GrizzlyApp::dmaInitializeRGBData()
{
    NVIC_InitTypeDef nvic;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
  DMA_InitTypeDef DMA_InitStructure;
    
    TIM_OCStructInit(&TIM_OCInitStructure);
    
  TIM_TimeBaseStructure.TIM_Period = 120;          
  TIM_TimeBaseStructure.TIM_Prescaler = uhTimerPrescaler;  
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    
  /* TIM1 Update DMA Request enable */
  TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE);

    DMA_DeInit(DMA2_Stream5);

  DMA_InitStructure.DMA_Channel = DMA_Channel_6;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(GPIOE->ODR);
  //DMA_InitStructure.DMA_Memory0BaseAddr = ((uint32_t) (frame->buffer));        //&(dma_frame_buffers[0][0]);
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = (LEDS_PER_GPIO_CHANNEL - 1) * (DMA_FRAME_BPP / 2);
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream5, &DMA_InitStructure);

  DMA_ITConfig(DMA2_Stream5, DMA_IT_TC, ENABLE);

  /* Enable the DMA Stream IRQ Channel */
  nvic.NVIC_IRQChannel = DMA2_Stream5_IRQn;
  nvic.NVIC_IRQChannelPreemptionPriority = 1;
  nvic.NVIC_IRQChannelSubPriority = 1;
  nvic.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvic);
}

void GrizzlyApp::dmaInitializeClockRising()
{
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  DMA_InitTypeDef DMA_InitStructure;
    
    TIM_OCStructInit(&TIM_OCInitStructure);
    
  /* Output Compare Active Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Active;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 60;
  TIM_OC1Init(TIM1, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);
  TIM_ARRPreloadConfig(TIM1, DISABLE);

  TIM_DMACmd(TIM1, TIM_DMA_CC1 , ENABLE);

  DMA_DeInit(DMA2_Stream1);

  DMA_InitStructure.DMA_Channel = DMA_Channel_6;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(GPIOF->ODR);
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) &(clock_rising_buffer);        //&(dma_frame_buffers[0][0]);
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = 1;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream1, &DMA_InitStructure);
}

void GrizzlyApp::dmaInitializeClockFalling(void)
{
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  DMA_InitTypeDef DMA_InitStructure;
    
    TIM_OCStructInit(&TIM_OCInitStructure);

  /* Output Compare Active Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Inactive;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 120;
  TIM_OC2Init(TIM1, &TIM_OCInitStructure);

  TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Disable);
  TIM_ARRPreloadConfig(TIM1, DISABLE);

  TIM_DMACmd(TIM1, TIM_DMA_CC2, ENABLE);

  DMA_DeInit(DMA2_Stream2);

  DMA_InitStructure.DMA_Channel = DMA_Channel_6;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(GPIOF->ODR);
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) &(clock_falling_buffer);        //&(dma_frame_buffers[0][0]);
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = 1;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream2, &DMA_InitStructure);
}

void GrizzlyApp::dmaSendRGBFrame()
{
  uhTimerPrescaler = (SystemCoreClock /  75000000 ) - 1;
//  clock_rising_buffer = frame->flags;
    
  //dmaInitializeRGBData(frame);
  dmaInitializeClockRising();
  dmaInitializeClockFalling();
    
    /* TIM1 counter enable */
  //GPIOE->ODR = ((frame->buffer[0]) & 0x00FF) | ((frame->buffer[1] << 8) & 0xFF00);
  TIM1->CR1 |= TIM_CR1_CEN;
    TIM_GenerateEvent(TIM1, TIM_EventSource_Update);
    DMA2_Stream5->CR |= (uint32_t)DMA_SxCR_EN;
    DMA2_Stream1->CR |= (uint32_t)DMA_SxCR_EN;
  DMA2_Stream2->CR |= (uint32_t)DMA_SxCR_EN;

}

void GrizzlyApp::dmaInitLEDFrameTimer(void)
{
  TIM_TimeBaseInitTypeDef timerInitStructure; 
  // NVIC_InitTypeDef nvicStructure;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  timerInitStructure.TIM_Prescaler = ((SystemCoreClock / 2) / 200000) - 1;
  timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
  timerInitStructure.TIM_Period = 25;
  timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  timerInitStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM2, &timerInitStructure);
  //TIM_Cmd(TIM3, ENABLE);
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
  
  /*nvicStructure.NVIC_IRQChannel = TIM3_IRQn;
  nvicStructure.NVIC_IRQChannelPreemptionPriority = 6;
  nvicStructure.NVIC_IRQChannelSubPriority = 1;
  nvicStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvicStructure);*/
}

void GrizzlyApp::dmaStartLEDFrameTimer(void)
{
    TIM3->CR1 |= TIM_CR1_CEN;
}

void GrizzlyApp::dmaStopLEDFrameTimer(void)
{
    TIM3->CR1 &= ~TIM_CR1_CEN;
}
