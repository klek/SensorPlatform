/**
  ******************************************************************************
  * @file    ADC/ADC_RegularConversion_DMA/Src/main.c
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    30-December-2016
  * @brief   This example describes how to use the DMA to transfer
  *          continuously converted data.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
//#include "main.h"
#include "debug/logging.h"
#include "adc/adcSetup.h"
#include "maths/processing.h"
#include "misc/complexBuffer.h"
//#include "ethernet/ethernetSetup.h"

/** @addtogroup STM32F7xx_HAL_Examples
  * @{
  */

/** @addtogroup ADC_RegularConversion_DMA
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ADC_BUFFER_SIZE             (FFT_SIZE * 2)
#define RING_BUFFER_SIZE			(FFT_SIZE)
#define DECIMATION_FACTOR           16

// Defines for the statusVector
#define HALF_BUFFER_INT             (1 << 0)
#define FULL_BUFFER_INT             (1 << 1)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* ADC handler declaration */
ADC_HandleTypeDef AdcAHandle;
ADC_HandleTypeDef AdcBHandle;

// ETH handler declaration
ETH_HandleTypeDef EthHandle;

// Defining a global buffer for DMA storage
uint32_t adcBuffer[ADC_BUFFER_SIZE];

// Defining a global variable for the network interface
//struct netif gnetif;

// Defining a global buffer for data storage

// ADC value
// Note(klek): These can most likely be moved into main-scope
uint32_t ADCAValue = 0;
uint32_t ADCBValue = 0;

uint32_t interrupted = 0;
uint32_t statusVector = 0;


/* Private function prototypes -----------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

// Note(klek): Added functions by me
// Interrupt handlers
//void ADC_IRQHandler(void);
void DMA2_Stream0_IRQHandler(void);
//void DMA2_Stream3_IRQHandler(void);

// Static functions
//static void Error_Handler(void);
//static void MPU_Config(void);
static void CPU_CACHE_Enable(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
	/*
	 * Setup the MPU for ETH descriptors etc
	 */
	//MPU_Config();

    /*
     * Initilizing variables
     */
    // Might need 2 of these, one for first part of buffer and one for second
    // These could be moved to main
    float32_t inData[ADC_BUFFER_SIZE];
    float32_t fftInData[FFT_SIZE];
    float32_t fftResult[FFT_SIZE];

    // The rotating buffer for data before FFT calculation
    // This is one item larger than the other buffers because it will always
    // contain one empty slot -- WHY DID I THINK THIS?
    struct complexData dataBuffer[RING_BUFFER_SIZE];
    struct circularBuffer workData =
    {
    	.buffer = dataBuffer,
		.head = 0,
		.tail = 0,
		.filled = 0,
		.maxLen = (RING_BUFFER_SIZE)
    };

    /*
     * Enable the CPU Cache
     */
    CPU_CACHE_Enable();

    /* STM32F7xx HAL library initialization:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user
         can eventually implement his proper time base source (a general purpose
         timer for example or other time source), keeping in mind that Time base
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
    HAL_Init();


    /*
     * Configure LED1 and LED3
     */
    BSP_LED_Init(LED1);
    BSP_LED_Init(LED3);

    /*
     * Configure the system clock to 216 MHz
     */
    SystemClock_Config();

    // Initialize variables
    memset(adcBuffer, 0, ADC_BUFFER_SIZE);

#ifdef LOGGING
    /*
    * Setup UART-debugging with serial console
    */
    uartSetup();
#endif

    /*
     * Setup the network interface
     */
    //netifConfig(&gnetif);

    /*
     * Setup the two ADCs
     */
    adcSetup(&AdcAHandle, &AdcBHandle);



    int n = 0;
    while ( n < 160 ){
        n++;
        LOG("The loopvalue is %i\n", n);
    }

    /*
     * Start both of the ADCs with associated buffers
     */
    // Start the conversion process for DUAL-mode interleaved ADCs
    if ( adcStart(&AdcAHandle, &AdcBHandle, adcBuffer, ADC_BUFFER_SIZE) != HAL_OK )
    {
        LOG("ERROR: Failed to start DUAL-mode interleaved conversion!\n");
    }
/*
    // Start the conversion process for ADC_A
    if(HAL_ADC_Start_DMA(&AdcAHandle, adcABuffer, ADC_BUFFER_SIZE) != HAL_OK)
    {
        LOG("ERROR: Failed to start ADC_A with DMA!\n");
    }

    // Start the conversion process for ADC_B
    if(HAL_ADC_Start_DMA(&AdcBHandle, adcBBuffer, ADC_BUFFER_SIZE) != HAL_OK)
    {
        LOG("ERROR: Failed to start ADC_B with DMA!\n");
    }
*/
    /*
     * NOTE(klek):  More initialization needed?
     */
    // Init the fft module
/*  if (fftProcess((float32_t*)adcABuffer) != ARM_MATH_SUCCESS ) {
        LOG("Error: Couldn't initialize the fft module!\n");
    }
*/


    /*
     * Main program loop
     */
    while (1)
    {
        /*
         * Note(klek):  Where should data be moved? IRQ not that great probably
         *              Should it be done here then, in the main-loop?
         */
        // Check status-vector for half buffer interrupt
        if ( statusVector & HALF_BUFFER_INT )
        {
            // Remove the flag
            statusVector &= ~HALF_BUFFER_INT;

            // Move data from first half of adcBuffer into processing buffer inData
            // This will also center the data into the middle of the new buffer
            copyBuffers((uint32_t*)adcBuffer, (float32_t*)inData, (uint32_t)ADC_BUFFER_SIZE);

            // Print the ADCXValues
            LOG("The current ADC_A-value is %f\t\t", inData[10] );
            LOG("The current ADC_B-value is %f\n", inData[11] );
        }
        // Check status-vector for full buffer
        else if ( statusVector & FULL_BUFFER_INT )
        {
            // Remove the flag
            statusVector &= ~FULL_BUFFER_INT;

            // Move last half of adcBuffer into processing buffer inData
            copyBuffers((uint32_t*)(adcBuffer + (ADC_BUFFER_SIZE/2)), (float32_t*)inData, (uint32_t)ADC_BUFFER_SIZE);
            // Print the ADCXValues
            LOG("The current ADC_A-value is %f\t\t", inData[10] );
            LOG("The current ADC_B-value is %f\n", inData[11] );
        }
        else
        {
            // Here we process data
        }

        // inData contains the new sampled data
        // This data needs to be filtered and decimated since we are looking for very low frequencies.
        // For the FFT to be accurate we need a resolution of 0.5 Hz
        // A rough estimate of the FFT resolution is possible to get from samples/sec divided by points
        // This is true for the low-pass filtering with FIR as well

        // Currently we have will decimate by half => removing half of the samples
        // This should generate more than enough samples left to do FFT, but reduce the samples/sec drastically
        // Essentially we should do a ring buffer with same size as the number of points in the FFT
        // And then we could continuously execute FFT on this vector
        uint32_t validItems = 0;
        validItems = filterAndDecimate((float32_t*)inData, ADC_BUFFER_SIZE, DECIMATION_FACTOR);

        if ( validItems == 0 )
        {
            LOG("ERROR: Filtering and decimation results in zero new items\n");
        }

        // inData can now be copied to the fftInData...
        LOG("New buffer contains %lu valid items\n", validItems);

        /*
         * TODO(klek): Arctangent calculation of the valid items in the buffer
         */
        phaseCalc((float32_t*)inData, validItems);

        /*
         * TODO(klek): Update the rotating buffer with the newly calculated values
         */
        circMultiPush(&workData, (struct complexData *)inData, validItems);

        /*
         * TODO(klek): When the ringBuffer has been filled, we should copy data to
         *             fftInData buffer for FFT-processing.
         *             This needs some form of checking
         */
        if ( workData.filled == 1 )
        {
            circMultiRead(&workData, (struct complexData *)fftInData, FFT_SIZE);
        }
        
        // Print the ADCXValues
        //LOG("The current ADC_A-value is %lu\t\t", inData[10] );
        //LOG("The current ADC_B-value is %lu\n", inData[11] );


        /*
         * NOTE(klek):  When we process data through FFT the previous buffer will be overwritten
         *              unless we do this some other way.
         *              Also can we implement some sort of ringbuffer to not have to copy data
         *              more than once?
         */
        // Process data through FFT
        //fftProcess((float32_t*)inData);

        /*
         * NOTE(klek):  The processing flow should be as follows after
         *              interrupts have occured.
         *
         *              Data should be moved from DMA-buffers
         *              Data should be pre-processed before being saved elsewhere
         *              Pre-processing includes filtering and decimation of the data
         *              After filtering and decimation of the data, we can start processing it
         *              by doing arctangent calculation and also follow that up with FFT
         */
        LOG("We reached interrupt routine %lu times since last\n", interrupted);
        interrupted = 0;
    }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 216000000
  *            HCLK(Hz)                       = 216000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 432
  *            PLL_P                          = 2
  *            PLL_Q                          = 9
  *            PLL_R                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 7
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 432;  
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  RCC_OscInitStruct.PLL.PLLR = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    while(1) {};
  }
  
  /* Activate the OverDrive to reach the 216 Mhz Frequency */
  if(HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    while(1) {};
  }
  
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    while(1) {};
  }
}


/*
 * This function handles interrupts for the ADC1, ADC2 and ADC3
 */
/*void ADC_IRQHandler(void)
{
    HAL_ADC_IRQHandler(&AdcAHandle);
}*/

/*
 * This function handles interrupts for the DMA2 Stream0
 */
void DMA2_Stream0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(AdcAHandle.DMA_Handle);
}

/*
 * This function handles interrupts for the DMA2 Stream0
 */
//void DMA2_Stream3_IRQHandler(void)
//{
//  HAL_DMA_IRQHandler(AdcBHandle.DMA_Handle);
//}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
/*static void Error_Handler(void)
{
  // Turn LED3 on
  BSP_LED_On(LED3);
  while (1)
  {
  }
}*/

/**
  * @brief  Conversion complete callback in non blocking mode
  * @param  AdcHandle : AdcHandle handle
  * @note   This example shows a simple way to report end of conversion, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
    /*
     * We need to find out which ADC gave the interrupt
     */
    if ( AdcHandle->Instance == ADC_A ) {
        // Copy data
        //ADCAValue = accumulate(adcABuffer, adcABuffer + ADC_BUFFER_SIZE, 0) / ADC_BUFFER_SIZE;
        ADCAValue = adcBuffer[4];
        ADCBValue = adcBuffer[5];

        // Set status bit
        statusVector |= FULL_BUFFER_INT;

        interrupted++;
    }
    else if ( AdcHandle->Instance == ADC_B ) {
        // Copy data
        //ADCBValue = accumulate(adcBBuffer, adcBBuffer + ADC_BUFFER_SIZE, 0) / ADC_BUFFER_SIZE;
//      ADCBValue = adcBBuffer[4];
        LOG("BUG: Why did this happen??\n");
    }
    else {
        // What now?
        // Turn LED3 on, to indicate something went wrong
        BSP_LED_On(LED3);
    }
    /* Turn LED1 on: Transfer process is correct */
    BSP_LED_On(LED1);
}

/**
  * @brief  Conversion complete callback in non blocking mode
  * @param  AdcHandle : AdcHandle handle
  * @note   This example shows a simple way to report end of conversion, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
    /*
     * We need to find out which ADC gave the interrupt
     */
    if ( AdcHandle->Instance == ADC_A ) {
        // Copy data
        //ADCAValue = accumulate(adcABuffer, adcABuffer + ADC_BUFFER_SIZE, 0) / ADC_BUFFER_SIZE;
        ADCAValue = adcBuffer[4];

        // Set status bit
        statusVector |= HALF_BUFFER_INT;

        //interrupted++;
    }
    else if ( AdcHandle->Instance == ADC_B ) {
        // Copy data
        //ADCBValue = accumulate(adcBBuffer, adcBBuffer + ADC_BUFFER_SIZE, 0) / ADC_BUFFER_SIZE;
//      ADCBValue = adcBBuffer[4];
        LOG("BUG: Why did this happen??\n");
    }
    else {
        // What now?
        // Turn LED3 on, to indicate something went wrong
        BSP_LED_On(LED3);
    }
    /* Turn LED1 on: Transfer process is correct */
    BSP_LED_On(LED1);
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
  * @brief  Configure the MPU attributes as Device for  Ethernet Descriptors in the SRAM1.
  * @note   The Base Address is 0x20020000 since this memory interface is the AXI.
  *         The Configured Region Size is 256B (size of Rx and Tx ETH descriptors)
  *
  * @param  None
  * @retval None
  */
//static void MPU_Config(void)
//{
//  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
//  HAL_MPU_Disable();

//  /* Configure the MPU attributes as Device for Ethernet Descriptors in the SRAM */
//  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
//  MPU_InitStruct.BaseAddress = 0x20020000;
//  MPU_InitStruct.Size = MPU_REGION_SIZE_256B;
//  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
//  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
//  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
//  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
//  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
//  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
//  MPU_InitStruct.SubRegionDisable = 0x00;
//  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

//  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
//  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
//}

/**
* @brief  CPU L1-Cache enable.
* @param  None
* @retval None
*/
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
