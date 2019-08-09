#define RCC_AHB1ENR      (*(unsigned int*)(0x40023830))
// PORTC 주소 + 모드별 오프셋 LED 출력
#define GPIOC_MODER      (*(unsigned int*)0x40020800)
#define GPIOC_OTYPE      (*(unsigned int*)0x40020804)
#define GPIOC_OSPEED     (*(unsigned int*)0x40020808)
#define GPIOC_PUPDR      (*(unsigned int*)0x4002080C)
#define GPIOC_ODR        (*(unsigned int*)0x40020814)

// PORTB 주소 + 모드별 오프셋 스위치 입력
#define GPIOB_MODER     (*(unsigned int*)0x40020400)
#define GPIOB_OTYPE     (*(unsigned int*)0x40020404)
#define GPIOB_OSPEED    (*(unsigned int*)0x40020408)
#define GPIOB_PUPDR     (*(unsigned int*)0x4002040C)
#define GPIOB_IDR       (*(unsigned int*)0x40020410)

//static void Delay(const uint32_t Count)
//{
//  __IO uint32_t index = 0;
//  for(index = (16800 * Count); index != 0; index--);
//}

void _Delay(int Count)
{
  Count = Count * 16800;
  while(Count--);
}

unsigned int LED_Data = 0x01;

int main()
{
  RCC_AHB1ENR = 0x00000006;
  
  GPIOC_MODER = 0x55555555;
  GPIOC_OTYPE = 0x0;
  GPIOC_OSPEED = 0xffffffff;
  GPIOC_PUPDR = 0x0;
  
  
  GPIOB_MODER = 0x00000000;
  GPIOB_OTYPE = 0x0;
  GPIOB_OSPEED = 0xffffffff;
  GPIOB_PUPDR = 0x0;
   
  while(1)
  {
    
    if((GPIOB_IDR & 0xf000) == 0x1000)GPIOC_ODR = LED_Data;
    else if((GPIOB_IDR & 0xf000) == 0x2000)
    {
      if(LED_Data < 8)
        LED_Data <<= 1;
      else
        LED_Data = 0x01;
      
      GPIOC_ODR = LED_Data;
    }
    else if((GPIOB_IDR & 0xf000) == 0x4000)
    { 
      if(LED_Data > 1)
        LED_Data >>= 1;
      else
        LED_Data = 0x08;
      
      GPIOC_ODR = LED_Data;
    }
    else if((GPIOB_IDR & 0xf000) == 0x8000)
      GPIOC_ODR = 0x0000;
    
    _Delay(200);
  }
}

