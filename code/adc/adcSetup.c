/*******************************************************************
   $File:    adcSetup.c
   $Date:    Mon, 22 May 2017: 19:05
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#include "adcSetup.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx_nucleo_144.h"

#include "../debug/logging.h"

/*
 * Setup function for ADC 1 and ADC 2
 */
void adcSetup(ADC_HandleTypeDef* adcA, ADC_HandleTypeDef* adcB)
{
	// Initializing parameters
	ADC_ChannelConfTypeDef sConfig;

	/*##-1- Configure the ADC_A peripheral #######################################*/
	adcA->Instance          		 = ADC_A;
	adcA->Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV4;
	adcA->Init.Resolution            = ADC_RESOLUTION_12B;
	adcA->Init.ScanConvMode          = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
	adcA->Init.ContinuousConvMode    = ENABLE;                       /* Continuous mode enabled to have continuous conversion  */
	adcA->Init.DiscontinuousConvMode = DISABLE;                       /* Parameter discarded because sequencer is disabled */
	adcA->Init.NbrOfDiscConversion   = 0;
	adcA->Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;        /* Conversion start trigged at each external event */
	adcA->Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;
	adcA->Init.DataAlign             = ADC_DATAALIGN_RIGHT;
	adcA->Init.NbrOfConversion       = 1;
	adcA->Init.DMAContinuousRequests = ENABLE;
	adcA->Init.EOCSelection          = DISABLE;

	if (HAL_ADC_Init(adcA) != HAL_OK)
	{
		/* ADC initialization Error */
		LOG("Init of ADC_A failed!\n");
		//while(1);
	}

	if (HAL_ADC_Init(adcB) != HAL_OK)
	{
		/* ADC initialization Error */
		LOG("Init of ADC_B failed!\n");
		//while(1);
	}

	LOG("Init of ADC:s successful!\n");


	/*
	* Configure ADC_A regular channel
	*/
	sConfig.Channel      = ADC_CHANNEL_10;
	sConfig.Rank         = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	sConfig.Offset       = 0;

	if (HAL_ADC_ConfigChannel(adcA, &sConfig) != HAL_OK)
	{
		/* Channel Configuration Error */
		LOG("ADC_A channel configuration failed!\n");
		//while(1);
	}

	LOG("Channel setup for ADC_A was successful!\n");

	/*
	* Configure ADC_B regular channel
	*/
	sConfig.Channel      = ADC_CHANNEL_10;
	sConfig.Rank         = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	sConfig.Offset       = 0;

	if (HAL_ADC_ConfigChannel(adcB, &sConfig) != HAL_OK)
	{
		/* Channel Configuration Error */
		LOG("ADC_A channel configuration failed!\n");
		//while(1);
	}

	LOG("Channel setup for ADC_B was successful!\n");

}


/**
  * @brief ADC MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  * @param hadc: ADC handle pointer
  * @retval None
  */
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
  GPIO_InitTypeDef          GPIO_InitStruct;
  static DMA_HandleTypeDef  hdma_adc;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* ADC1 Periph clock enable */
  ADC_A_CLK_ENABLE();
  /* Enable GPIO clock ****************************************/
  ADC_A_CHANNEL_GPIO_CLK_ENABLE();
  /* Enable DMA2 clock */
  DMA_2_CLK_ENABLE();

  /*##-2- Configure peripheral GPIO ##########################################*/
  /* ADC Channel GPIO pin configuration */
  GPIO_InitStruct.Pin = ADC_A_CHANNEL_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ADC_A_CHANNEL_GPIO_PORT, &GPIO_InitStruct);

  /*##-3- Configure the DMA streams ##########################################*/
  /* Set the parameters to be configured */
  hdma_adc.Instance = ADC_A_DMA_STREAM;

  hdma_adc.Init.Channel  = ADC_A_DMA_CHANNEL;
  hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
  hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_adc.Init.Mode = DMA_CIRCULAR;
  hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_adc.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  hdma_adc.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
  hdma_adc.Init.MemBurst = DMA_MBURST_SINGLE;
  hdma_adc.Init.PeriphBurst = DMA_PBURST_SINGLE;

  HAL_DMA_Init(&hdma_adc);

  /* Associate the initialized DMA handle to the ADC handle */
  __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc);

  /*##-4- Configure the NVIC for DMA #########################################*/
  /* NVIC configuration for DMA transfer complete interrupt */
  HAL_NVIC_SetPriority(ADC_A_DMA_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(ADC_A_DMA_IRQn);
}

/**
  * @brief ADC MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO to their default state
  * @param hadc: ADC handle pointer
  * @retval None
  */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{

  /*##-1- Reset peripherals ##################################################*/
  ADC_A_FORCE_RESET();
  ADC_A_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks ################################*/
  /* De-initialize the ADC Channel GPIO pin */
  HAL_GPIO_DeInit(ADC_A_CHANNEL_GPIO_PORT, ADC_A_CHANNEL_PIN);
}
