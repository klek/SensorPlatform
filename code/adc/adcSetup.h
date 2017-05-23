/*******************************************************************
   $File:    adcSetup.h
   $Date:    Mon, 22 May 2017: 19:06
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#if !defined(ADCSETUP_H)
#define ADCSETUP_H

#include "stm32f7xx_hal.h"
#include "stm32f7xx_nucleo_144.h"


/* Definition for ADC_A clock resources */
#define ADC_A                           ADC1
#define ADC_A_CLK_ENABLE()               __HAL_RCC_ADC1_CLK_ENABLE()
#define DMA_2_CLK_ENABLE()               __HAL_RCC_DMA2_CLK_ENABLE()
#define ADC_A_CHANNEL_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOC_CLK_ENABLE()

#define ADC_A_FORCE_RESET()              __HAL_RCC_ADC_FORCE_RESET()
#define ADC_A_RELEASE_RESET()            __HAL_RCC_ADC_RELEASE_RESET()

/* Definition for ADC_A Channel Pin */
#define ADC_A_CHANNEL_PIN                GPIO_PIN_0
#define ADC_A_CHANNEL_GPIO_PORT          GPIOC

/* Definition for ADC_A's Channel */
#define ADC_A_CHANNEL                    ADC_CHANNEL_10

/* Definition for ADC_A's DMA */
#define ADC_A_DMA_CHANNEL                DMA_CHANNEL_0
#define ADC_A_DMA_STREAM                 DMA2_Stream0

/* Definition for ADC_A's NVIC */
#define ADC_A_DMA_IRQn                   DMA2_Stream0_IRQn
#define ADC_A_DMA_IRQHandler             DMA2_Stream0_IRQHandler

void adcSetup(ADC_HandleTypeDef* adcA, ADC_HandleTypeDef* adcB);


#endif
