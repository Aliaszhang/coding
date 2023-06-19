## 说明 ##

```基于正点原子的stm32h743 阿波罗开发板创建的工程模板，用的是st hal库```

---------------------------------

1. 使用的是正点原子的阿波罗H743开发板

2. 调试串口接线如下:

   `USART1 - PA9  - TX`
   `USART1 - PA10 - RX`

3. HAL库版本: `STM32Cube_FW_H7_V1.11.0`

4. 时钟配置:

时钟配置
```
The system Clock is configured as follow : 
   System Clock source            = PLL (HSE)
   SYSCLK(Hz)                     = 400000000 (CPU Clock)
   HCLK(Hz)                       = 200000000 (AXI and AHBs Clock)
   AHB Prescaler                  = 2
   D1 APB3 Prescaler              = 2 (APB3 Clock  100MHz)
   D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz)
   D2 APB2 Prescaler              = 2 (APB2 Clock  100MHz)
   D3 APB4 Prescaler              = 2 (APB4 Clock  100MHz)
   HSE Frequency(Hz)              = 25000000
   PLL_M                          = 5
   PLL_N                          = 160
   PLL_P                          = 2
   PLL_Q                          = 4
   PLL_R                          = 2
   VDD(V)                         = 3.3
   Flash Latency(WS)              = 4
```


