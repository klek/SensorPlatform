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

	ADC_MultiModeTypeDef multiModeConfig;

	// Configure the ADC_A peripheral
	adcA->Instance          		 = ADC_A;
	adcA->Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV2;
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

	// Configure the ADC_B peripheral
	adcB->Instance          		 = ADC_B;
	adcB->Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV4;
	adcB->Init.Resolution            = ADC_RESOLUTION_12B;
	adcB->Init.ScanConvMode          = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
	adcB->Init.ContinuousConvMode    = ENABLE;                       /* Continuous mode enabled to have continuous conversion  */
	adcB->Init.DiscontinuousConvMode = DISABLE;                       /* Parameter discarded because sequencer is disabled */
	adcB->Init.NbrOfDiscConversion   = 0;
	adcB->Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;        /* Conversion start trigged at each external event */
	adcB->Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;
	adcB->Init.DataAlign             = ADC_DATAALIGN_RIGHT;
	adcB->Init.NbrOfConversion       = 1;
	adcB->Init.DMAContinuousRequests = ENABLE;
	adcB->Init.EOCSelection          = DISABLE;


	if (HAL_ADC_Init(adcA) != HAL_OK)
	{
		/* ADC initialization Error */
		LOG("ERROR: Init of ADC_A failed!\n");
		//while(1);
	}

	if (HAL_ADC_Init(adcB) != HAL_OK)
	{
		/* ADC initialization Error */
		LOG("ERROR: Init of ADC_B failed!\n");
		//while(1);
	}

	LOG("Init of ADC:s successful!\n");


	/*
	 * Configure ADC_A regular channel
	 */
	sConfig.Channel      = ADC_A_CHANNEL;
	sConfig.Rank         = 1;
	sConfig.SamplingTime = ADC_SAMPLING_TIME;
	sConfig.Offset       = 0;

	if ( HAL_ADC_ConfigChannel(adcA, &sConfig) != HAL_OK)
	{
		/* Channel Configuration Error */
		LOG("ERROR: ADC_A channel configuration failed!\n");
		//while(1);
	}

	LOG("Channel setup for ADC_A was successful!\n");

	/*
	 * Configure ADC_B regular channel
	 */
	sConfig.Channel      = ADC_B_CHANNEL;
	sConfig.Rank         = 1;
	sConfig.SamplingTime = ADC_SAMPLING_TIME;
	sConfig.Offset       = 0;

	if ( HAL_ADC_ConfigChannel(adcB, &sConfig) != HAL_OK)
	{
		/* Channel Configuration Error */
		LOG("ERROR: ADC_A channel configuration failed!\n");
		//while(1);
	}

	LOG("Channel setup for ADC_B was successful!\n");

	/*
	 * Configure DUAL-mode interleaved
	 */
	multiModeConfig.Mode				= ADC_DUALMODE_REGSIMULT;
	multiModeConfig.DMAAccessMode		= ADC_DMAACCESSMODE_2;
	multiModeConfig.TwoSamplingDelay	= ADC_TWOSAMPLINGDELAY_5CYCLES;

	if ( HAL_ADCEx_MultiModeConfigChannel(adcA, &multiModeConfig) != HAL_OK )
	{
		LOG("ERROR: Failed to set multimode config!\n");
	}
}

HAL_StatusTypeDef adcStart(ADC_HandleTypeDef* adcA, ADC_HandleTypeDef* adcB, uint32_t* data, uint32_t len)
{
	// Enable ADC_B
	if ( HAL_ADC_Start(adcB) != HAL_OK )
	{
		return HAL_ERROR;
	}

	// Enable ADC_A as master for DUAL-mode interleaved DMA with double buffering
	if ( HAL_ADCEx_MultiModeStart_DMA(adcA, data, len) != HAL_OK )
	{
		return HAL_ERROR;
	}

	return HAL_OK;
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
	static DMA_HandleTypeDef  hdma_adcA;
	static DMA_HandleTypeDef  hdma_adcB;


	/*
	 * So we have 2 cases here. We either come here with ADC_A or ADC_B
	 * We need to find out which one
	 */
	// Enable DMA2 clock which is same for both cases
	DMA_2_CLK_ENABLE();

	if ( hadc->Instance == ADC_A )
	{
		// ADC1 Peripheral clock enable
		ADC_A_CLK_ENABLE();
		// Enable GPIO clock
		ADC_A_CHANNEL_GPIO_CLK_ENABLE();

		// Configure peripheral GPIO
		// ADC_A Channel GPIO pin configuration
		GPIO_InitStruct.Pin 				= ADC_A_CHANNEL_PIN;
		GPIO_InitStruct.Mode 				= GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull 				= GPIO_NOPULL;
		HAL_GPIO_Init(ADC_A_CHANNEL_GPIO_PORT, &GPIO_InitStruct);

		// Configure the DMA streams
		// Set the parameters to be configured
		hdma_adcA.Instance 					= ADC_A_DMA_STREAM;

		hdma_adcA.Init.Channel  			= ADC_A_DMA_CHANNEL;
		hdma_adcA.Init.Direction 			= DMA_PERIPH_TO_MEMORY;
		hdma_adcA.Init.PeriphInc 			= DMA_PINC_DISABLE;
		hdma_adcA.Init.MemInc 				= DMA_MINC_ENABLE;
		hdma_adcA.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_WORD;
		hdma_adcA.Init.MemDataAlignment 	= DMA_MDATAALIGN_WORD;
		hdma_adcA.Init.Mode 				= DMA_CIRCULAR;
		hdma_adcA.Init.Priority 			= DMA_PRIORITY_HIGH;
		hdma_adcA.Init.FIFOMode 			= DMA_FIFOMODE_DISABLE;
		hdma_adcA.Init.FIFOThreshold 		= DMA_FIFO_THRESHOLD_HALFFULL;
		hdma_adcA.Init.MemBurst 			= DMA_MBURST_SINGLE;
		hdma_adcA.Init.PeriphBurst 			= DMA_PBURST_SINGLE;

		HAL_DMA_Init(&hdma_adcA);

		// Associate the initialized DMA handle to the ADC handle
		__HAL_LINKDMA(hadc, DMA_Handle, hdma_adcA);

		// Configure the NVIC for DMA
		// NVIC configuration for DMA transfer complete interrupt
		HAL_NVIC_SetPriority(ADC_A_DMA_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(ADC_A_DMA_IRQn);
	}
	else if ( hadc->Instance == ADC_B )
	{
		// ADC2 Peripheral clock enable
		ADC_B_CLK_ENABLE();
		// Enable GPIO clock
		ADC_B_CHANNEL_GPIO_CLK_ENABLE();

		// Configure peripheral GPIO
		// ADC_B Channel GPIO pin configuration
		GPIO_InitStruct.Pin = ADC_B_CHANNEL_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(ADC_B_CHANNEL_GPIO_PORT, &GPIO_InitStruct);

		// Configure the DMA streams
		// Set the parameters to be configured
/*		hdma_adcB.Instance = ADC_B_DMA_STREAM;

		hdma_adcB.Init.Channel  = ADC_B_DMA_CHANNEL;
		hdma_adcB.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_adcB.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_adcB.Init.MemInc = DMA_MINC_ENABLE;
		hdma_adcB.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		hdma_adcB.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
		hdma_adcB.Init.Mode = DMA_CIRCULAR;
		hdma_adcB.Init.Priority = DMA_PRIORITY_HIGH;
		hdma_adcB.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		hdma_adcB.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
		hdma_adcB.Init.MemBurst = DMA_MBURST_SINGLE;
		hdma_adcB.Init.PeriphBurst = DMA_PBURST_SINGLE;

		HAL_DMA_Init(&hdma_adcB);

		// Associate the initialized DMA handle to the ADC handle
		__HAL_LINKDMA(hadc, DMA_Handle, hdma_adcB);

		// Configure the NVIC for DMA
		// NVIC configuration for DMA transfer complete interrupt
		HAL_NVIC_SetPriority(ADC_B_DMA_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(ADC_B_DMA_IRQn);
*/
	}
	else
	{
		LOG("ERROR: Couldn't determine which ADC to be configured!\n");
	}




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

	// Reset peripherals
	ADC_A_FORCE_RESET();
	ADC_A_RELEASE_RESET();
	ADC_B_FORCE_RESET();
	ADC_B_RELEASE_RESET();

	// Disable peripherals and GPIO Clocks
	// De-initialize the ADC Channel GPIO pin
	HAL_GPIO_DeInit(ADC_A_CHANNEL_GPIO_PORT, ADC_A_CHANNEL_PIN);
	HAL_GPIO_DeInit(ADC_B_CHANNEL_GPIO_PORT, ADC_B_CHANNEL_PIN);

}
