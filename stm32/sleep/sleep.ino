/*
  STM32 LowPower modes testing
  (c)2019 Pawel A. Hernik
  YouTube video: 
  https://youtu.be/ThO8t149KmM
*/

// *** CONNECTIONS ***
/*
 N5110 LCD pinout from left:
 #1 RST      - PA0
 #2 CS/CE    - PA4
 #3 DC       - PA1
 #4 MOSI/DIN - PA7
 #5 SCK/CLK  - PA5
 #6 VCC      - 3.3V
 #7 LIGHT    - 200ohm to GND
 #8 GND
 STM32 SPI1 pins:
  PA4 CS1
  PA5 SCK1
  PA6 MISO1
  PA7 MOSI1
*/

#include <libmaple/pwr.h>
#include <libmaple/scb.h>
#include <RTClock.h>
RTClock rtclock(RTCSEL_LSE);





void disableClocks() 
{
    rcc_clk_disable(RCC_ADC1);
    rcc_clk_disable(RCC_ADC2);
    rcc_clk_disable(RCC_ADC3);
    rcc_clk_disable(RCC_AFIO);
    rcc_clk_disable(RCC_BKP);
    rcc_clk_disable(RCC_CRC);
    rcc_clk_disable(RCC_DAC);
    rcc_clk_disable(RCC_DMA1);
    rcc_clk_disable(RCC_DMA2);
    rcc_clk_disable(RCC_FLITF);
    rcc_clk_disable(RCC_FSMC);
    //rcc_clk_disable(RCC_GPIOA); // needed by N5110
    rcc_clk_disable(RCC_GPIOB);
    rcc_clk_disable(RCC_GPIOC);
    rcc_clk_disable(RCC_GPIOD);
    rcc_clk_disable(RCC_GPIOE);
    rcc_clk_disable(RCC_GPIOF);
    rcc_clk_disable(RCC_GPIOG);
    rcc_clk_disable(RCC_I2C1);
    rcc_clk_disable(RCC_I2C2);
    //rcc_clk_disable(RCC_PWR); // needed by standby
    rcc_clk_disable(RCC_SDIO);
    //rcc_clk_disable(RCC_SPI1); // needed by N5110
    rcc_clk_disable(RCC_SPI2);
    rcc_clk_disable(RCC_SPI3);
    rcc_clk_disable(RCC_SRAM);
    rcc_clk_disable(RCC_TIMER1);
    rcc_clk_disable(RCC_TIMER2);
    rcc_clk_disable(RCC_TIMER3);
    rcc_clk_disable(RCC_TIMER4);
    rcc_clk_disable(RCC_TIMER5);
    rcc_clk_disable(RCC_TIMER6);
    rcc_clk_disable(RCC_TIMER7);
    rcc_clk_disable(RCC_TIMER8);
    rcc_clk_disable(RCC_TIMER9);
    rcc_clk_disable(RCC_TIMER10);
    rcc_clk_disable(RCC_TIMER11);
    rcc_clk_disable(RCC_TIMER12);
    rcc_clk_disable(RCC_TIMER13);
    rcc_clk_disable(RCC_TIMER14);
    rcc_clk_disable(RCC_USART1);
    rcc_clk_disable(RCC_USART2);
    rcc_clk_disable(RCC_USART3);
    rcc_clk_disable(RCC_UART4);
    rcc_clk_disable(RCC_UART5);
    rcc_clk_disable(RCC_USB);
}




static void int_fun() {};

// standby=true for deep sleep
void sleepMode(bool standby, uint8_t seconds)
{ 
  rtclock.createAlarm(&int_fun, rtclock.getTime()+seconds);  // wakeup int
  PWR_BASE->CR &= PWR_CR_LPDS | PWR_CR_PDDS | PWR_CR_CWUF;
  PWR_BASE->CR |= PWR_CR_CWUF;
  PWR_BASE->CR |= PWR_CSR_EWUP;
  SCB_BASE->SCR |= SCB_SCR_SLEEPDEEP;
  if(standby) {
    PWR_BASE->CR |= PWR_CR_PDDS;
    PWR_BASE->CR &= ~PWR_CR_LPDS;
  } else {
    adc_disable(ADC1);
    adc_disable(ADC2);
    PWR_BASE->CR &= ~PWR_CR_PDDS;
    PWR_BASE->CR |= PWR_CR_LPDS;
  }
  asm("    wfi");
  SCB_BASE->SCR &= ~SCB_SCR_SLEEPDEEP;
}


void setup()
{

  disableClocks();
 

  sleepMode(true,100);  // standby

 
}



void loop()
{


 
}
