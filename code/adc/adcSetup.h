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

/*
 * Common defines for the ADCs
 */
#define ADC_SAMPLING_TIME				ADC_SAMPLETIME_56CYCLES

/*
 * Definition for ADC_A clock resources
 */
#define ADC_A							ADC1
#define ADC_A_CLK_ENABLE()				__HAL_RCC_ADC1_CLK_ENABLE()
#define DMA_2_CLK_ENABLE()				__HAL_RCC_DMA2_CLK_ENABLE()
#define ADC_A_CHANNEL_GPIO_CLK_ENABLE()	__HAL_RCC_GPIOC_CLK_ENABLE()

#define ADC_A_FORCE_RESET()				__HAL_RCC_ADC_FORCE_RESET()
#define ADC_A_RELEASE_RESET()			__HAL_RCC_ADC_RELEASE_RESET()

// Definition for ADC_A Channel Pin
// Should be PC0 on CN9 (pin nr 3)
#define ADC_A_CHANNEL_PIN				GPIO_PIN_0
#define ADC_A_CHANNEL_GPIO_PORT			GPIOC

/* Definition for ADC_A's Channel */
#define ADC_A_CHANNEL					ADC_CHANNEL_10

/* Definition for ADC_A's DMA */
#define ADC_A_DMA_CHANNEL				DMA_CHANNEL_0
#define ADC_A_DMA_STREAM				DMA2_Stream0

/* Definition for ADC_A's NVIC */
#define ADC_A_DMA_IRQn					DMA2_Stream0_IRQn
#define ADC_A_DMA_IRQHandler			DMA2_Stream0_IRQHandler

/*
 * Definition for ADC_B clock resources
 */
#define ADC_B							ADC2
#define ADC_B_CLK_ENABLE()				__HAL_RCC_ADC2_CLK_ENABLE()
//#define DMA_2_CLK_ENABLE()				__HAL_RCC_DMA2_CLK_ENABLE()
#define ADC_B_CHANNEL_GPIO_CLK_ENABLE()	__HAL_RCC_GPIOC_CLK_ENABLE()

#define ADC_B_FORCE_RESET()				__HAL_RCC_ADC_FORCE_RESET()
#define ADC_B_RELEASE_RESET()			__HAL_RCC_ADC_RELEASE_RESET()

// Definition of ADC_B channel pin
// Should be PF4 on CN10 (pin nr 11)
// Changed to PC2 on CN10 (pin nr 9)
#define ADC_B_CHANNEL_PIN				GPIO_PIN_2
#define ADC_B_CHANNEL_GPIO_PORT			GPIOC

// Definition for ADC_B's channel
#define ADC_B_CHANNEL					ADC_CHANNEL_12

// Definition for ADC_B's DMA
#define ADC_B_DMA_CHANNEL				DMA_CHANNEL_1
#define ADC_B_DMA_STREAM				DMA2_Stream3

// Definition for ADC_B's NVIC
#define ADC_B_DMA_IRQn					DMA2_Stream3_IRQn
#define ADC_B_DMA_IRQHandler			DMA2_Stream3_IRQHandler

void adcSetup(ADC_HandleTypeDef* adcA, ADC_HandleTypeDef* adcB);
HAL_StatusTypeDef adcStart(ADC_HandleTypeDef* adcA, ADC_HandleTypeDef* adcB, uint32_t* data, uint32_t len);
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc);
void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc);

#endif
