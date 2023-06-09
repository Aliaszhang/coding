/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LED0_TG         HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1)
#define LED1_TG         HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0)
#define LED0(x)  	    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,((x) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET)) 
#define LED1(x)  	    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,((x) == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET)) 
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern SDRAM_HandleTypeDef hsdram1;

static uint8_t key[256] = {0};

uint16_t testsram[250000] __attribute__((at(0xC0000000)));

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Prevent unused argument(s) compilation warning */
  memset(key, 0, 256);
  switch (GPIO_Pin)
  {
  case GPIO_PIN_0 :
    memcpy(key, "WakeUp", 6);
    break;
  case GPIO_PIN_2 :
    memcpy(key, "Key1  ", 6);
    break;
  case GPIO_PIN_3 :
    memcpy(key, "Key0  ", 6);
    break;
  case GPIO_PIN_13 :
    memcpy(key, "Key2  ", 6);
    break;
  default:
    break;
  };

}

void fsmc_sdram_test(uint16_t x,uint16_t y)
{  
  uint32_t i=0;  	  
  uint32_t temp=0;	   
  uint32_t sval=0;
  OLED_ShowString(x,y, (uint8_t *)"sdram:", 12); 
  for(i=0;i<32*1024*1024;i+=16*1024)
  {
    *(uint32_t*)(Bank5_SDRAM_ADDR+i)=temp; 
    temp++;
  }
  for(i=0;i<32*1024*1024;i+=16*1024) 
  {	
    temp=*(uint32_t*)(Bank5_SDRAM_ADDR+i);
    printf("%d ---- temp = %#x, sval = %#x\r\n", i, temp, sval);
	  if(i==0)
      sval=temp;
 	  else if(temp<=sval)
    {
      printf("return!!! temp = %d, sval = %d\r\n", temp, sval);
      break;   		   
    }
	  OLED_ShowNum(x+36,y,(uint16_t)(temp-sval+1)*16,5,12);
    OLED_Refresh_Gram();
    HAL_Delay(500);
  }					 
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint8_t t = 0,led0stat = 0,led1stat = 1;
  uint32_t ts = 0;

  SCB_EnableICache();
  SCB_EnableDCache();   
	SCB->CACR|=1<<2;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_FMC_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  OLED_ShowString(0,0,(const uint8_t *)"Alias",24);  
  OLED_ShowString(0,24, (const uint8_t *)"0.96' OLED TEST",16);  
  OLED_ShowString(0,40,(const uint8_t *)"KEY Press:",12);  
  OLED_ShowString(0,52,(const uint8_t *)"ASCII:",12);  
  OLED_ShowString(64,52,(const uint8_t *)"CODE:",12);  
  OLED_Refresh_Gram();
  t=' ';
  // OLED_ShowChinese(0,0,32,1);

  printf("start test\r\n");
  for(ts=0;ts<250000;ts++)
  {
    testsram[ts]=ts; 
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    printf("start test\r\n");
    OLED_ShowChar(36,52,t,12,1);
    OLED_ShowNum(94,52,t,3,12); 
    OLED_ShowString(60,40,(const uint8_t *)key,12); 
    OLED_Refresh_Gram();
    t++;
    if(t>'~')
      t=' ';  

    // LED1_TG;
    // LED0_TG;
    LED0(led0stat ^= 1);
    LED1(led1stat ^= 1);
    HAL_Delay(500);
    HAL_UART_Transmit(&huart2,(uint8_t*)"nihao\r\n",7,1000);
    while(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_TC)!=SET);
    
    if (strcmp((char*)key, "WakeUp") == 0)
    {
      memset(key, 0, 256);
      fsmc_sdram_test(0, 40);
      OLED_ShowString(0,40,(const uint8_t *)"KEY Press:          ",12);
      HAL_Delay(500);
    }


  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
void _sys_exit(int x) 
{ 
	x = x; 
} 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->ISR&0x40)==0); 
	USART1->TDR=(uint8_t)ch;      
	return ch;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
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
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
