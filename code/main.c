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


// Includes for testing purposes
//#include "../testing/superPosSignal.h"
//#include "../testing/phaseShiftSignal.h"
#include "../testing/testbench.h"

/** @addtogroup STM32F7xx_HAL_Examples
  * @{
  */

/** @addtogroup ADC_RegularConversion_DMA
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ADC_BUFFER_SIZE             (FFT_SIZE * 2)
#define RING_BUFFER_SIZE            (FFT_SIZE)
#define DECIMATION_FACTOR           TIME_DEC//1//16

// Defines for the statusVector
#define HALF_BUFFER_INT             (1 << 0)
#define FULL_BUFFER_INT             (1 << 1)

//#define TEST_BUTTER
//#define TEST_ARCTAN
//#define TEST_FFT
#define PRINT_PEAKS
//#define PRINT_SPECTRUM

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* ADC handler declaration */
ADC_HandleTypeDef AdcAHandle;
ADC_HandleTypeDef AdcBHandle;

// ETH handler declaration
//ETH_HandleTypeDef EthHandle;

// Defining a global buffer for DMA storage
uint32_t adcBuffer[ADC_BUFFER_SIZE];

// Defining a global variable for the network interface
//struct netif gnetif;

// Defining a global buffer for data storage

// ADC value
// Note(klek): These can most likely be moved into main-scope
//uint32_t ADCAValue = 0;
//uint32_t ADCBValue = 0;

uint32_t interrupted = 0;
uint32_t curIndex = 0;
uint32_t statusVector = 0;
static const int testBenchLength = sizeof(testBenchSignal) / sizeof(testBenchSignal[0]);


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
     * Initializing variables
     */
    // Flag to indicate new data for processing
    uint8_t newData = 0;

    // Might need 2 of these, one for first part of buffer and one for second
    float32_t inData[ADC_BUFFER_SIZE];
    float32_t fftInData[FFT_SIZE * 2];
    float32_t fftResult[FFT_SIZE];

    // The rotating buffer for data before FFT calculation
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
    memset((char *)adcBuffer, 0, (ADC_BUFFER_SIZE*sizeof(uint32_t)));

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



/*    int n = 0;
    while ( n < 160 ){
        n++;
        LOG("The loopvalue is %i\n", n);
    }
*/

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
            //copyBuffers((uint32_t*)adcBuffer, (float32_t*)inData, (uint32_t)ADC_BUFFER_SIZE);

            // Copy the test signal from the testbench
            copyBuffers((testBenchSignal + curIndex), inData, (uint32_t)ADC_BUFFER_SIZE);

            // Indicate new data is available for processing
            newData = 1;

            // Print the ADCXValues
//            LOG("The current ADC_A-value is %f\t\t", inData[10] );
//            LOG("The current ADC_B-value is %f\n", inData[11] );
        }
        // Check status-vector for full buffer
        else if ( statusVector & FULL_BUFFER_INT )
        {
            // Remove the flag
            statusVector &= ~FULL_BUFFER_INT;

            // Move last half of adcBuffer into processing buffer inData
            //copyBuffers((uint32_t*)(adcBuffer + (ADC_BUFFER_SIZE/2)), (float32_t*)inData, (uint32_t)ADC_BUFFER_SIZE);

            // Copy the test signal from the testbench
            copyBuffers((testBenchSignal + curIndex), inData, (uint32_t)ADC_BUFFER_SIZE);

            // Indicate new data is available for processing
            newData = 1;

            // Print the ADCXValues
//            LOG("The current ADC_A-value is %f\t\t", inData[10] );
//            LOG("The current ADC_B-value is %f\n", inData[11] );
        }
        else
        {
            // Here we process data
            if ( newData == 1 ) {
                newData = 0;
#ifdef TEST_BUTTER
                memcpy(inData, superPosSignal, ((FFT_SIZE * 2) * sizeof(float32_t)) );
#endif

                // inData contains the new sampled data
                // This data needs to be filtered and decimated since we are looking for very low frequencies.
                uint32_t validItems = 0;
                validItems = filterAndDecimate(inData, ADC_BUFFER_SIZE, DECIMATION_FACTOR);

#ifdef TEST_BUTTER
                // Should move this debugging into main-file
                LOG("Printing the filtered vector: \n");
                int n = 0;
                // Delimiter
                LOG("{ %f; %f", inData[n], inData[n+1]);
                n += 2;
                while (n < (FFT_SIZE * 2) )
                {
                    LOG(";\n%f; %f", inData[n], inData[n+1]);
                    n += 2;
                }
                // Delimiter
                LOG(" };\n");
#endif

                if ( validItems == 0 )
                {
                    LOG("ERROR: Filtering and decimation results in zero new items\n");
                }

                // Debugging
                LOG("New buffer contains %lu valid items\n", validItems);

#ifdef TEST_ARCTAN
                // Testing the arctan implementation with a signal
                // vector from matlab
                memcpy(inData, phaseShiftSignal, ((FFT_SIZE * 2) * sizeof(float32_t)) );
#endif

                // Calculating the phase for the incoming data
                phaseCalc((float32_t*)inData, validItems);


                // Updating the ring buffer with new values
                circMultiPush(&workData, (struct complexData *)inData, validItems/2);

                // When the ring buffer has been filled, data is copied into the
                // FFT-algorithm for processing
                if ( workData.filled == 1 )
                {
                    circMultiRead(&workData, (struct complexData *)fftInData, FFT_SIZE);
        
#if (defined(TEST_ARCTAN) || defined(TEST_BUTTER))
                    // Copy phase-calculated data into fftInData
                    memcpy(fftInData, inData, ((FFT_SIZE * 2) * sizeof(float32_t)) );
#endif

#ifdef TEST_FFT
                    // Testing the FFT with signal vector from matlab.
                    // This needs to be applied to the fftInData
                    memcpy(fftInData, superPosSignal, ((FFT_SIZE * 2) * sizeof(float32_t)) );

//                    LOG("%f \n %f \n %f\n", fftInData[0], fftInData[1], fftInData[2]);
#endif

                    // Calculate mean for the input data
                    float32_t meanVal = 0.0f;
                    arm_mean_f32(fftInData, FFT_SIZE*2, &meanVal);

                    // Process data through FFT
                    float32_t maxVal[NR_OF_PEAKS];
                    uint32_t resIndex[NR_OF_PEAKS];
                    fftProcess(fftInData, fftResult, maxVal, resIndex);

#ifdef PRINT_PEAKS
                    /*
                     * Debugging
                     * Print the result to debugging terminal for capture in MATLAB
                     */
                    int s = 0;
                    for (s = 0; s < NR_OF_PEAKS; s++)
                    {
                        LOG("Max value of %f at bin %lu (%.2f Hz) \n", maxVal[s], resIndex[s], (float32_t)(1.0f/DECIMATION_FACTOR)*resIndex[s]);
                    }
#endif

#ifdef PRINT_SPECTRUM
                    /*
                     * Debugging
                     * Print the result to debugging terminal for capture in MATLAB
                     */
                    s = 0;
                    LOG("[ %f; %f", fftResult[s], fftResult[s+1]);
                    s += 2;
                    while ( s < FFT_SIZE / 2 )
                    {
                        LOG(";\n%f; %f", fftResult[s], fftResult[s+1]);
                        s += 2;
                    }
                    LOG(" ]; \n");
#endif
                }
                // Debug output to verify interrupts
                LOG("%lu interrupts since last time\n", interrupted);
                interrupted = 0;
            }
        }
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV16; //RCC_HCLK_DIV2; Modified to reduce ADC sample rate
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
        // Set status bit
        statusVector |= FULL_BUFFER_INT;

        // Testing the algorithm
        curIndex += ADC_BUFFER_SIZE / 2;
        if ( curIndex >= testBenchLength ) {
            curIndex = 0;
        }

        interrupted++;
    }
    else if ( AdcHandle->Instance == ADC_B ) {
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
        // Set status bit
        statusVector |= HALF_BUFFER_INT;

        // Testing the algorithm
        curIndex += ADC_BUFFER_SIZE / 2;
        if ( curIndex >= testBenchLength ) {
            curIndex = 0;
        }

        interrupted++;
    }
    else if ( AdcHandle->Instance == ADC_B ) {
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
