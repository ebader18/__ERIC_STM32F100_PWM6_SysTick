#include "main.h"
#include "stm32f10x.h"

#define __READ_BIT(var,pos) ((var) & (1<<(pos)))
#define __CLEAR_BIT(var,pos) ((var) &= ~(1<<(pos)))
#define __SET_BIT(var,pos) ((var) |= (1<<(pos)))
#define __TOGGLE_BIT(var,pos) ((var) ^= (1<<(pos)))

uint16_t SysTick_frequency = 100;
uint32_t PWM_frequency = 20000;
float Ch1_Duty = 0.05;
float Ch2_Duty = 0.1;
float Ch3_Duty = 0.15;

void Init_6CHPWM();
void ChangeDuty_6CHPWM();

int main(void)
{
  RCC->APB2ENR = RCC_APB2ENR_AFIOEN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN;     // Enable all GPIO Clock & Alternate Function Clock*//*Enable all GPIO Clock & Alternate Function Clock
  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;           // Enable TIMER1 & USART1 Clock

  GPIOB->CRL = 0x444444AA;                      // Configure PB0 & PB1 as Alternate Function Push Pull Output Max Speed = 2MHz
  GPIOB->CRH = 0x44444444;                      // Default value => Floating input
  GPIOA->CRL = 0xA4444444;                      // PA7 as Alternate Function Push Pull Output Max Speed = 2MHz
  GPIOA->CRH = 0x44444AAA;                      // PA8 - PA10 as Alternate Function Push Pull Output Max Speed = 2MHz
  GPIOC->CRL = 0x44444422;                      // PC0 & PC1 as General Purpose Push Pull Output Max Speed = 2MHz
  GPIOC->CRH = 0x44444424;                      // PC9 as General Purpose Push Pull Output Max Speed = 2MHz

  /* Partial remap (ETR/PA12, CH1/PA8, CH2/PA9, CH3/PA10, CH4/PA11, BKIN/PA6, CH1N/PA7, CH2N/PB0, CH3N/PB1) */
  /* CH1: PA8 / PA7 */
  /* CH2: PA9 / PB0 */
  /* CH3: PA10 / PB1 */
  AFIO->MAPR |= 0x00000040;

  Init_6CHPWM();                                //Initialize 20KHz PWM on CH 1-3
  
  if (SysTick_Config(SystemCoreClock / SysTick_frequency))    // Setup SysTick Timer for 1 msec interrupts
    while (1);                                  // Capture error
  
  while(1)
  {
  }
}

void Init_6CHPWM()
{
  uint16_t PWM_period = SystemCoreClock/PWM_frequency/2;
  uint16_t Ch1_OnPeriod = (uint16_t)((float)PWM_period * Ch1_Duty);
  uint16_t Ch2_OnPeriod = (uint16_t)((float)PWM_period * Ch2_Duty);
  uint16_t Ch3_OnPeriod = (uint16_t)((float)PWM_period * Ch3_Duty);
  
  TIM1->CCMR1 |= 0x00006868;                    // CH1 & CH2 Output Compare, Preload, PWM1 Enable
  TIM1->CCMR2 |= 0x00000068;                    // CH3 Output Compare, Preload, PWM1 Enable
  TIM1->CCER  |= 0x00000555;                    // CH1, CH2, CH3 and their Corresponding  Complement Output Enable
  TIM1->CR1 |= 0x000000A0;                      // Centre Align Mode 1 & Auto-reload Preload enable
  TIM1->PSC = 0;                                // Prescaler set to 0
  TIM1->ARR = PWM_period;                       // Auto reload value 1400 (Period = 50us)
  TIM1->CCR1 = Ch1_OnPeriod;                    // Start PWM duty for channel 1
  TIM1->CCR2 = Ch2_OnPeriod;                    // Start PWM duty for channel 2
  TIM1->CCR3 = Ch3_OnPeriod;                    // Start PWM duty for channel 3
  TIM1->BDTR |= 0x00008818;                     // Enable Main O/P & Enable OSSR
}

void ChangeDuty_6CHPWM()
{
  uint16_t PWM_period = SystemCoreClock/PWM_frequency/2;
  uint16_t Ch1_OnPeriod = (uint16_t)((float)PWM_period * Ch1_Duty);
  uint16_t Ch2_OnPeriod = (uint16_t)((float)PWM_period * Ch2_Duty);
  uint16_t Ch3_OnPeriod = (uint16_t)((float)PWM_period * Ch3_Duty);
  
  TIM1->CCR1 = Ch1_OnPeriod;                    // Start PWM duty for channel 1
  TIM1->CCR2 = Ch2_OnPeriod;                    // Start PWM duty for channel 2
  TIM1->CCR3 = Ch3_OnPeriod;                    // Start PWM duty for channel 3
}

void SysTickReady(void)
{
  if (__READ_BIT(TIM1->CR1, 0) == 0)            // If function called for first time, enable TIM1
    __SET_BIT(TIM1->CR1, 0);                    // Counter Enable
  
  __TOGGLE_BIT(GPIOC->ODR, 0);                  // Toggle PC0, trigger on oscilloscope
  
  Ch1_Duty += 0.001;
  if (Ch1_Duty > 1.0)
    Ch1_Duty = 0.001;
  
  ChangeDuty_6CHPWM();
}