#include "grizzly.h"

extern volatile uint16_t clock_rising_buffer;
extern volatile uint16_t clock_falling_buffer;

uint16_t uhTimerPrescaler = 0;

void dmaInitializeRGBData(ColorFrame* frame)
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
  DMA_InitStructure.DMA_Memory0BaseAddr = ((uint32_t) (frame->buffer));        //&(dma_frame_buffers[0][0]);
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

void dmaInitializeClockRising(void)
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

void dmaInitializeClockFalling(void)
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

void dmaSendRGBFrame(ColorFrame *frame)
{
  uhTimerPrescaler = (SystemCoreClock /  75000000 ) - 1;
	clock_rising_buffer = frame->flags;
	
  dmaInitializeRGBData(frame);
  dmaInitializeClockRising();
  dmaInitializeClockFalling();
	
	/* TIM1 counter enable */
  GPIOE->ODR = ((frame->buffer[0]) & 0x00FF) | ((frame->buffer[1] << 8) & 0xFF00);
  TIM1->CR1 |= TIM_CR1_CEN;
	TIM_GenerateEvent(TIM1, TIM_EventSource_Update);
	DMA2_Stream5->CR |= (uint32_t)DMA_SxCR_EN;
	DMA2_Stream1->CR |= (uint32_t)DMA_SxCR_EN;
  DMA2_Stream2->CR |= (uint32_t)DMA_SxCR_EN;

}

void dmaInitializeFrameTimer(void)
{
	TIM_TimeBaseInitTypeDef timerInitStructure; 
	NVIC_InitTypeDef nvicStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	timerInitStructure.TIM_Prescaler = 45000;
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period = 25;
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &timerInitStructure);
	TIM_Cmd(TIM2, ENABLE);
	//TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	
	nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 2;
	nvicStructure.NVIC_IRQChannelSubPriority = 1;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);
}

void dmaInitLEDFrameTimer(void)
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

void dmaStartLEDFrameTimer(void)
{
	TIM3->CR1 |= TIM_CR1_CEN;
}

void dmaStopLEDFrameTimer(void)
{
	TIM3->CR1 &= ~TIM_CR1_CEN;
}
