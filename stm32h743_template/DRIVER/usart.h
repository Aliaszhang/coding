#ifndef __USART_H__
#define __USART_H__

#include "stdio.h"
#include "stm32h7xx_hal.h"

#define SYN_BUFFER_MAX_SIZE         (1U)
int debug_uart_init(void);
int MX_USART2_UART_Init(uint32_t baudrate);

#endif
