
#include "stm32f4xx.h"

static void Delay(const uint32_t Count)
{
  __IO uint32_t index;
  for(index = (16800*Count); index !=0; index--);
}

void putch(uint8_t c)
{
  USART_SendData(USART1, c);
  while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}


//unsigned int LED_Data = 0x0010;
unsigned int FND_DATA_TBL[] = {0x3f00, 0x0600, 0x5b00, 0x4f00,
                                0x6600, 0x6d00, 0x7c00, 0x0700,
                                0x7f00, 0x6700, 0x7700, 0x7c00,
                                0x3900, 0x5e00, 0x7900, 0x7100, 
                                0x0800, 0x8000};
unsigned char Time_STOP = 0;
unsigned int CCR1_Val = 0;
unsigned char cnt = 0;
unsigned char text[] = "IoT System Report \r\n";

int main()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_OCInitTypeDef TIM_OCInitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOC ,ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  
  // GPIOA -> USART
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_9 | GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
  
   //Mode OUT PIN 설정(3: DC Motor)
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  //DC Motor 구동 PIN 설정
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  
  //GPIOB -> LED On Switch Pin 12
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  //Mode OUT PIN 설정(8~15 : FND)
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = 
    GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  // TIMER 설정
  TIM_TimeBaseStructure.TIM_Prescaler = 84-1;
  TIM_TimeBaseStructure.TIM_Period = 100-1;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  
  
  // PWM 설정
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC1Init(TIM2, &TIM_OCInitStructure);
  
  // TIMER2 동작
  TIM_OCPreloadConfig(TIM2, TIM_OCPreload_Enable);
  TIM_ARRPreloadConfig(TIM2, ENABLE);
  TIM_Cmd(TIM2, ENABLE);
  
  
  // USART 설정
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);
  USART_Cmd(USART1, ENABLE);
  
  while(1)
  {
    if((GPIO_ReadInputData(GPIOB)&0xf000) == 0x1000 || Time_STOP == 1)
    {
      Time_STOP = 1;
      GPIO_Write(GPIOC, 0x0);
      GPIO_ResetBits(GPIOA, GPIO_Pin_3);
      
      TIM_Cmd(TIM2, DISABLE);
      TIM_SetCompare1(TIM2, CCR1_Val);
      TIM_Cmd(TIM2, ENABLE);
      CCR1_Val++;
      if(CCR1_Val>100) CCR1_Val = 0;
      Delay(10);
    }

    if((GPIO_ReadInputData(GPIOB)&0xf000)== GPIO_Pin_13 || Time_STOP == 2)
    {
      Time_STOP = 2;
      TIM_SetCompare1(TIM2, 0x0);
      GPIO_ResetBits(GPIOA, GPIO_Pin_3);
      
      GPIO_Write(GPIOC, FND_DATA_TBL[cnt]);
      cnt++;
      if(cnt>17)
        cnt = 0;
      Delay(500);
    }
    
    if((GPIO_ReadInputData(GPIOB)&0xf000) == 0x4000 || Time_STOP == 3)  
    {
      Time_STOP = 3;                                
      TIM_SetCompare1(TIM2, 0x0);               
      GPIO_ResetBits(GPIOA, GPIO_Pin_3);
      
      GPIO_SetBits(GPIOB, GPIO_Pin_8);
      GPIO_ResetBits(GPIOB, GPIO_Pin_9);
      GPIO_SetBits(GPIOA, GPIO_Pin_3);
      Delay(200);
    }
    
    if((GPIO_ReadInputData(GPIOB)&0xf000) == 0x8000 || Time_STOP == 4)  
    {
      Time_STOP = 4;                                
      cnt = 0;                                  
      TIM_SetCompare1(TIM2, 0x0);               
      GPIO_Write(GPIOC, 0x0);                   
      GPIO_ResetBits(GPIOA, GPIO_Pin_3);        
      
      
      while(text[cnt] != '\0')      
        putch(text[cnt++]);
      
      Time_STOP = 0;                                
      Delay(500);
    }
  } 
}



