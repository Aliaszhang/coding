#include "led.h"
#include "key.h"
#include "usart.h"
#include "sdram.h"
#include "lcd.h"
#include "ltdc.h"
#include "tim.h"
#include "touch.h"
#include <stdlib.h>
#include <string.h>
#include "lvgl/lvgl.h"
#include "lv_examples.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
// #include "lv_demo_keypad_encoder.h"

char key_string[10] = {0};
uint8_t key_value = 0;
// uint16_t testsram[250000] __attribute__((section(".bss.EXRAM1"))); // AC6
// uint16_t testsram[250000] __attribute__((at(0xC0000000))); // AC5


static void MPU_Config(void);
static void SystemClock_Config(void);
static void Error_Handler(void);
static void CPU_CACHE_Enable(void);

void delay_us(uint32_t nus);
    
void fsmc_sdram_test(void)
{  
	__IO uint32_t i=0;  	  
	__IO uint32_t temp=0;	   
	__IO uint32_t sval=0;

	for (i = 0; i < 32*1024*1024; i += 16*1024)
	{
		*(__IO uint32_t*)(SDRAM_BANK_ADDR + i) = temp; 
		temp++;
	}

 	for (i = 0; i < 32*1024*1024; i += 16*1024) 
	{	
  		temp = *(__IO uint32_t*)(SDRAM_BANK_ADDR + i);
		if (i == 0)
        {
            sval = temp;
        }
 		else if ( temp <= sval)
        {
            printf("SDRAM TEST FAIL AT:%d\r\n", i);
            break;
        }
		printf("SDRAM Capacity: %dKB\r\n", (uint16_t)(temp - sval + 1) * 16);
 	}				 
}

void Load_Drow_Dialog(void)
{
    LCD_Clear(GREEN);
    POINT_COLOR=BLUE;
    LCD_ShowString(lcddev.width-60, 0, 200, 24, 24, "Clear");
    POINT_COLOR=RED;
}

void gui_draw_hline(uint16_t x0, uint16_t y0, uint16_t len, uint16_t color)
{
	if (len == 0)
    {
        return;
    }

	if ((x0 + len - 1) >= lcddev.width)
    {
        x0 = lcddev.width - len - 1;	//限制坐标范围
    }
	if (y0 >= lcddev.height)
    {
        y0 = lcddev.height - 1;
    }

	LCD_Fill(x0, y0, x0 + len - 1, y0, color);	
}

void gui_fill_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{											  
	uint32_t i;
	uint32_t imax = ((uint32_t)r * 707) / 1000 + 1;
	uint32_t sqmax = (uint32_t)r * (uint32_t)r + (uint32_t)r / 2;
	uint32_t x = r;

	gui_draw_hline(x0 - r, y0, 2 * r, color);
	for (i = 1; i <= imax; i++) 
	{
		if ((i*i + x*x) > sqmax)// draw lines from outside  
		{
 			if (x > imax) 
			{
				gui_draw_hline(x0-i+1, y0+x, 2*(i-1), color);
				gui_draw_hline(x0-i+1, y0-x, 2*(i-1), color);
			}
			x--;
		}
		// draw lines from inside (center)  
		gui_draw_hline(x0-x, y0+i, 2*x, color);
		gui_draw_hline(x0-x, y0-i, 2*x, color);
	}
} 

void lcd_draw_bline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t size, uint16_t color)
{
    uint16_t t; 
    int xerr=0, yerr=0, delta_x, delta_y, distance; 
    int incx, incy, uRow, uCol; 

    if (x1 < size || x2 < size || y1 < size || y2 < size)
        return; 

    delta_x = x2-x1;
    delta_y = y2-y1; 
    uRow = x1; 
    uCol = y1; 

    if (delta_x > 0)
    {
        incx = 1;
    }
    else if (delta_x == 0)
    {
        incx = 0;
    }
    else 
    {
        incx = -1;
        delta_x = -delta_x;
    } 

    if (delta_y > 0)
    {
        incy = 1;
    } 
    else if (delta_y == 0)
    {
        incy = 0;
    }
    else
    {
        incy = -1;
        delta_y = -delta_y;
    } 

    if (delta_x > delta_y)
    {
        distance = delta_x;
    }
    else
    {
        distance = delta_y; 
    }

    for(t = 0; t <= distance + 1; t++)
    {
        gui_fill_circle(uRow, uCol, size, color); // 画点
        xerr += delta_x; 
        yerr += delta_y;

        if ( xerr > distance) 
        { 
            xerr -= distance; 
            uRow += incx; 
        } 
        if (yerr > distance) 
        { 
            yerr -= distance; 
            uCol += incy; 
        } 
    }  
}

uint16_t POINT_COLOR_TBL[5] = {RED, GREEN, BLUE, BROWN, GRED}; // 5点电容触摸的颜色  
uint16_t lastpos[5][2];
int main(void)
{
    uint8_t x = 0, y = 0, t = 0, i = 0;

    /* Configure the MPU attributes */
    MPU_Config();

    /* Enable the CPU Cache */
    CPU_CACHE_Enable();

    /* STM32H7xx HAL library initialization:
          - Systick timer is configured by default as source of time base, but user 
            can eventually implement his proper time base source (a general purpose 
            timer for example or other time source), keeping in mind that Time base 
            duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
            handled in milliseconds basis.
          - Set NVIC Group Priority to 4
          - Low Level Initialization
        */
    HAL_Init();

    /* Configure the system clock to 400 MHz */
    SystemClock_Config();
    HAL_Delay(400);

    /* Initialize BSP Led for LED3 */
    LED_Init();
    KEY_Init();
    debug_uart_init();
    MX_USART2_UART_Init(9600);
    MX_FMC_SDRAM_Init();
    LCD_Init();
    TIM3_Init(10-1, 20000-1);
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    tp_dev.init();

    LED0(0);
    
    /* Output a message on Hyperterminal using printf function */
    printf("\n\r UART Printf Example: retarget the C library printf function to the UART\n\r");
    printf("** Test finished successfully. ** \n\r");

	// POINT_COLOR = BLACK; 
    // LCD_ShowString(10,40,260,32,32,"Alias Zhang"); 	
    // LCD_ShowString(10,80,240,24,24,"LTDC TEST");
    // LCD_ShowString(10,120,240,24,24,"123456");
    // LCD_ShowString(10,160,240,12,12,"2023-06-20");
    // delay_us(1500000);
    // Load_Drow_Dialog();    
    // HAL_Delay(1000);

    key_value = 0;

    // lv_demo_keypad_encoder();
    lv_demo_benchmark();
    
    while(1)
    {
        lv_task_handler();
#if 0
        tp_dev.scan(0);
        for (t = 0; t < 5; t++)
        {
            if ((tp_dev.sta) & (1 << t)) // 相应的触摸点有数据
            {
                if (tp_dev.x[t] < lcddev.width && tp_dev.y[t] < lcddev.height)
                {
                    if(lastpos[t][0] == 0xFFFF)
                    {
                        lastpos[t][0] = tp_dev.x[t];
                        lastpos[t][1] = tp_dev.y[t];
                    }

                    lcd_draw_bline(lastpos[t][0],lastpos[t][1],tp_dev.x[t],tp_dev.y[t],2,POINT_COLOR_TBL[t]); // 画线

                    lastpos[t][0]=tp_dev.x[t];
                    lastpos[t][1]=tp_dev.y[t];
                    if(tp_dev.x[t] > (lcddev.width-24) && tp_dev.y[t]<20)
                    {
                        Load_Drow_Dialog();
                    }
                }
            }
            else
            {
                lastpos[t][0]=0XFFFF;
            }
        }       

        if (key_value != 0)
        {
            // printf("key %s press...\r\n", key_string);
            // fsmc_sdram_test();
            if (key_value == KEY0_PRES) // 清楚屏幕&更换背景色
            {
                switch(x)
                {
                    case 0:LCD_Clear(WHITE);break;
                    case 1:LCD_Clear(BLACK);break;
                    case 2:LCD_Clear(BLUE);break;
                    case 3:LCD_Clear(RED);break;
                    case 4:LCD_Clear(MAGENTA);break;
                    case 5:LCD_Clear(GREEN);break;
                    case 6:LCD_Clear(CYAN);break; 
                    case 7:LCD_Clear(YELLOW);break;
                    case 8:LCD_Clear(BRRED);break;
                    case 9:LCD_Clear(GRAY);break;
                    case 10:LCD_Clear(LGRAY);break;
                    case 11:LCD_Clear(BROWN);break;
                }
                x++;
                if(x==12)
                {
                    x=0;      
                }
            }
            else if (key_value == KEY1_PRES)
            {
                switch(y)
                {
                    case 0:POINT_COLOR = WHITE;break;
                    case 1:POINT_COLOR = BLACK;break;
                    case 2:POINT_COLOR = BLUE;break;
                    case 3:POINT_COLOR = BRED;break;
                    case 4:POINT_COLOR = GRED;break;
                    case 5:POINT_COLOR = GBLUE;break;
                    case 6:POINT_COLOR = RED;break; 
                    case 7:POINT_COLOR = MAGENTA;break;
                    case 8:POINT_COLOR = GREEN;break;
                    case 9:POINT_COLOR = YELLOW;break;
                    case 10:POINT_COLOR = CYAN;break;
                    case 11:POINT_COLOR = BROWN;break;
                    case 12:POINT_COLOR = BRRED;break;
                    case 13:POINT_COLOR = GRAY;break;
                    default: POINT_COLOR = RED;break;
                }
                y++;
                if(y==13)
                {
                    y=0;      
                }
                POINT_COLOR_TBL[0] = POINT_COLOR;
            }
            else
            {
                Load_Drow_Dialog();
            }
            key_value = 0;
        }
        HAL_Delay(5);
		if(i++ % 20 == 0)
        {
            LED0_Toggle;
        }
#endif
    }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 400000000 (CPU Clock)
  *            HCLK(Hz)                       = 200000000 (AXI and AHBs Clock)
  *            AHB Prescaler                  = 2
  *            D1 APB3 Prescaler              = 2 (APB3 Clock  100MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock  100MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock  100MHz)
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 5
  *            PLL_N                          = 160
  *            PLL_P                          = 2
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    HAL_StatusTypeDef ret = HAL_OK;
    
    /*!< Supply configuration update enable */
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

    /* The voltage scaling allows optimizing the power consumption when the device is
      clocked below the maximum system frequency, to update the voltage scaling value
      regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
        
    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
    RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    
    RCC_OscInitStruct.PLL.PLLM = 5;
    RCC_OscInitStruct.PLL.PLLN = 160;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
    if(ret != HAL_OK)
    {
        while(1) { ; }
    }
  
    /* Select PLL as system clock source and configure  bus clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | \
                                  RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);
    
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;  
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; 
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; 
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; 
    ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
    if(ret != HAL_OK)
    {
        while(1) { ; }
    }

    /*Activate CSI clock mondatory for I/O Compensation Cell*/ 
    __HAL_RCC_CSI_ENABLE() ;

    /* Enable SYSCFG clock mondatory for I/O Compensation Cell */
    __HAL_RCC_SYSCFG_CLK_ENABLE() ;

    /* Enables the I/O Compensation Cell */ 
    HAL_EnableCompensationCell();
}

static void CPU_CACHE_Enable(void)
{
    /* Enable I-Cache */
    SCB_EnableICache();

    /* Enable D-Cache */
    SCB_EnableDCache();

    // SCB->CACR |= 1<<2;
}

static void Error_Handler(void)
{
    /* Turn LED3 on */
    LED0(1);
    while(1)
    {
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    /* Prevent unused argument(s) compilation warning */
    memset(key_string, 0, 10);
    switch (GPIO_Pin)
    {
    case GPIO_PIN_0 :
        memcpy(key_string, "WakeUp", 6);
        key_value = WKUP_PRES;
        break;
    case GPIO_PIN_2 :
        memcpy(key_string, "Key1  ", 6);
        key_value = KEY1_PRES;
        break;
    case GPIO_PIN_3 :
        memcpy(key_string, "Key0  ", 6);
        key_value = KEY0_PRES;
        break;
    case GPIO_PIN_13 :
        memcpy(key_string, "Key2  ", 6);
        key_value = KEY2_PRES;
        break;
    default:
        break;
    };
}

static void MPU_Config(void)
{
    MPU_Region_InitTypeDef MPU_InitStruct;

    /* Disable the MPU */
    HAL_MPU_Disable();

    /* Configure the MPU as Strongly ordered for not defined regions */
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = 0x00;
    MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
    MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER0;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x87;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Configure the MPU attributes as WT for SDRAM */
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = SDRAM_BANK_ADDR;
    MPU_InitStruct.Size = MPU_REGION_SIZE_32MB;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER2;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Configure the MPU attributes as WT for SDRAM */
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = LCD_ADDRESS_START;
    MPU_InitStruct.Size = LCD_REGION_SIZE;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number = LCD_REGION_NUMBER;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Enable the MPU */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
      ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

void delay_us(uint32_t nus)
{		
	uint32_t ticks;
	uint32_t told, tnow, tcnt=0;
	uint32_t reload = SysTick->LOAD;
	ticks = nus * 400; // 400Mhz
	told = SysTick->VAL;
	while(1)
	{
		tnow = SysTick->VAL;	
		if (tnow != told)
		{	    
			if (tnow < told)
            {
                tcnt += told - tnow;
            }
			else
            {
                tcnt += reload - tnow + told;
            } 
			told = tnow;
			if (tcnt >= ticks)
            {
                break;
            }
		}  
	};
}
