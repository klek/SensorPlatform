/*******************************************************************
   $File:    uartSetup.c
   $Date:    Mon, 22 May 2017: 18:59
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#include "uartSetup.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx_nucleo_144.h"
#include "stm32f7xx_hal_uart.h"
#include "stdio.h"


#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

// Initialize the static UartHandle
static UART_HandleTypeDef UartHandle;

/*
 *
 */
void uartSetup(void)
{
      /*##-1- Configure the UART peripheral ######################################*/
      /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
      /* UART configured as follows:
          - Word Length = 8 Bits (7 data bit + 1 parity bit) :
                          BE CAREFUL : Program 7 data bits + 1 parity bit in PC HyperTerminal
          - Stop Bit    = One Stop bit
          - Parity      = ODD parity
          - BaudRate    = 9600 baud
          - Hardware flow control disabled (RTS and CTS signals) */
      UartHandle.Instance        = USARTx;

      UartHandle.Init.BaudRate   = UART_BAUD_RATE;
      UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
      UartHandle.Init.StopBits   = UART_STOPBITS_1;
      UartHandle.Init.Parity     = UART_PARITY_ODD;
      UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
      UartHandle.Init.Mode       = UART_MODE_TX_RX;
      UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
      if (HAL_UART_Init(&UartHandle) != HAL_OK)
      {
        /* Initialization Error */
        //Error_Handler();
      }

      /* Output a message on Hyperterminal using printf function */
      printf("\n UART Printf Example: retarget the C library printf function to the UART\n");
      printf("** Test finished successfully. ** \n");

}

/**
  * @brief UART MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  USARTx_TX_GPIO_CLK_ENABLE();
  USARTx_RX_GPIO_CLK_ENABLE();

  /* Select SysClk as source of USART1 clocks */
  RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  RCC_PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
  HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);

  /* Enable USARTx clock */
  USARTx_CLK_ENABLE();

  /*##-2- Configure peripheral GPIO ##########################################*/
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = USARTx_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = USARTx_TX_AF;

  HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = USARTx_RX_PIN;
  GPIO_InitStruct.Alternate = USARTx_RX_AF;

  HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);
}

/**
  * @brief UART MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  /*##-1- Reset peripherals ##################################################*/
  USARTx_FORCE_RESET();
  USARTx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure UART Tx as alternate function  */
  HAL_GPIO_DeInit(USARTx_TX_GPIO_PORT, USARTx_TX_PIN);
  /* Configure UART Rx as alternate function  */
  HAL_GPIO_DeInit(USARTx_RX_GPIO_PORT, USARTx_RX_PIN);

}

/*
 *
 */
void uartSend(char type, uint8_t* data, uint16_t dataSize)
{
    // Initialize the variables used
    uint8_t unused = 0;
    uint16_t nrOfPackets;
    int16_t restData;

    // Calculate how many packets will be sent
    if ( dataSize > 0 ) {
    	nrOfPackets = dataSize / PACKET_SIZE;
    	// Find potential rest
    	restData = dataSize - (nrOfPackets * PACKET_SIZE);
    	if ( restData > 0 ) {
    		// Add 1 packet to nrOfPackets
    		nrOfPackets += 1;
    	}
    }
    else {
    	return;
    }

    // Setup the packet
    uint16_t i = 0;
    for ( ; i < (nrOfPackets) ; i++)
    {
        /*
         * Sending the header
         */
    	// Sending the type + the empty byte
        HAL_UART_Transmit(&UartHandle, (uint8_t*)&type, 1, 0xFFFF);
        HAL_UART_Transmit(&UartHandle, (uint8_t*)&unused, 1, 0xFFFF);
        // Sending the packet nr
        // This should be set to zero if there is only one packet to send
        uint8_t tmp = i >> 8;
        HAL_UART_Transmit(&UartHandle, (uint8_t*)&(tmp), 1, 0xFFFF);
        if ( nrOfPackets == 1 ) {
        	tmp = i;
        }
        else {
        	tmp = i + 1;
        }
        HAL_UART_Transmit(&UartHandle, (uint8_t*)&tmp, 1, 0xFFFF);
        // Sending the total size of the payload
        tmp = dataSize >> 8;
        HAL_UART_Transmit(&UartHandle, (uint8_t*)&(tmp), 1, 0xFFFF);
        HAL_UART_Transmit(&UartHandle, (uint8_t*)&dataSize, 1, 0xFFFF);

        // Calculate remaining data
        restData = dataSize - (i * PACKET_SIZE);

        // Loop, sending one byte at a time here
        // If no more data exists, fill the rest of the bytes with zero
        int j = 0;
        for ( ; j < PACKET_SIZE ; j++)
        {
        	// We shall always send 256 bytes here
        	// How much is still available in buffer?
        	if ( restData > 0 ) {
        		// Send the data
        		HAL_UART_Transmit(&UartHandle, (uint8_t*)(data + i*PACKET_SIZE + j), 1, 0xFFFF);

        		// Check if we are in the last amount of data
        		if ( restData < PACKET_SIZE ) {
        			// Decrement restData
        			restData--;
        		}
        	}
        	else {
        		tmp = 0;
        		HAL_UART_Transmit(&UartHandle, (uint8_t*)&tmp, 1, 0xFFFF);
        	}
        }
    }
}

/**
  * @brief  Retargets the C library printf function to the USART.__io_putchar
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART3 and Loop until the end of transmission */
  HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}
