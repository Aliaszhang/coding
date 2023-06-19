#include "usart.h"

UART_HandleTypeDef UART1_Handler;
UART_HandleTypeDef UART2_Handler;
// uint8_t syn_rx_buffer[SYN_BUFFER_MAX_SIZE] = {0};
uint8_t syn_rx_buffer = 0;

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
int fputc(int ch, FILE *f)
{ 	
	while ((USART1->ISR & 0x40) == 0);
	USART1->TDR = (uint8_t)ch;      
	return ch;
}

int debug_uart_init(void)
{
	UART1_Handler.Instance = USART1;
	UART1_Handler.Init.BaudRate = 115200;
	UART1_Handler.Init.WordLength = UART_WORDLENGTH_8B;
	UART1_Handler.Init.StopBits = UART_STOPBITS_1;
	UART1_Handler.Init.Parity = UART_PARITY_NONE;
	UART1_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UART1_Handler.Init.Mode = UART_MODE_TX_RX;
    UART1_Handler.Init.OverSampling = UART_OVERSAMPLING_16;

    if(HAL_UART_Init(&UART1_Handler) != HAL_OK)
	{
		return -1;
	}
	return 0;
}

int MX_USART2_UART_Init(uint32_t baudrate)
{
	UART2_Handler.Instance = USART2;
	UART2_Handler.Init.BaudRate = baudrate;
	UART2_Handler.Init.WordLength = UART_WORDLENGTH_8B;
	UART2_Handler.Init.StopBits = UART_STOPBITS_1;
	UART2_Handler.Init.Parity = UART_PARITY_NONE;
	UART2_Handler.Init.Mode = UART_MODE_TX_RX;
	UART2_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UART2_Handler.Init.OverSampling = UART_OVERSAMPLING_16;
	UART2_Handler.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	UART2_Handler.Init.ClockPrescaler = UART_PRESCALER_DIV8;
	UART2_Handler.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&UART2_Handler) != HAL_OK)
	{
		return -1;
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&UART2_Handler, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
		return -1;
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&UART2_Handler, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
		return -1;
	}
	if (HAL_UARTEx_DisableFifoMode(&UART2_Handler) != HAL_OK)
	{
		return -1;
	}

	HAL_UART_Receive_IT(&UART2_Handler, &syn_rx_buffer, SYN_BUFFER_MAX_SIZE);
	return 0;
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
	GPIO_InitTypeDef GPIO_Initure;
	RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;
	
	if(huart->Instance == USART1)
	{
		/* Select SysClk as source of USART1 clocks */
		RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART16;
		RCC_PeriphClkInit.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
		HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);

		__HAL_RCC_USART1_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
	
		GPIO_Initure.Pin = GPIO_PIN_9 | GPIO_PIN_10;
		GPIO_Initure.Mode = GPIO_MODE_AF_PP;
		GPIO_Initure.Pull = GPIO_PULLUP;
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_Initure.Alternate = GPIO_AF7_USART1;
		HAL_GPIO_Init(GPIOA, &GPIO_Initure);
	}
	else if (huart->Instance == USART2)
	{
		RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
		RCC_PeriphClkInit.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
		HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);

		__HAL_RCC_USART2_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**USART2 GPIO Configuration
		PA2     ------> USART2_TX
		PA3     ------> USART2_RX
		*/
		GPIO_Initure.Pin = GPIO_PIN_2|GPIO_PIN_3;
		GPIO_Initure.Mode = GPIO_MODE_AF_PP;
		GPIO_Initure.Pull = GPIO_PULLUP;
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_Initure.Alternate = GPIO_AF7_USART2;
		HAL_GPIO_Init(GPIOA, &GPIO_Initure);

		__HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
		HAL_NVIC_SetPriority(USART2_IRQn, 3, 3);
	}
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{
		/*##-1- Reset peripherals ##################################################*/
		__HAL_RCC_USART1_FORCE_RESET();
		__HAL_RCC_USART1_RELEASE_RESET();

		/*##-2- Disable peripherals and GPIO Clocks ################################*/
		/* Configure UART Tx as alternate function  */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
		/* Configure UART Rx as alternate function  */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10);
	}
	else if(huart->Instance == USART2)
	{
		/*##-1- Reset peripherals ##################################################*/
		__HAL_RCC_USART2_FORCE_RESET();
		__HAL_RCC_USART2_RELEASE_RESET();

		/*##-2- Disable peripherals and GPIO Clocks ################################*/
		/* Configure UART Tx as alternate function  */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2);
		/* Configure UART Rx as alternate function  */
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3);
	}
}

void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UART2_Handler);
  HAL_UART_Receive_IT(&UART2_Handler, &syn_rx_buffer, SYN_BUFFER_MAX_SIZE);
}

/**
  * @brief  Rx Transfer completed callback.
  * @param  huart UART handle.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
	{
		printf("syn_recv: %#x\r\n", syn_rx_buffer);
	}
}


