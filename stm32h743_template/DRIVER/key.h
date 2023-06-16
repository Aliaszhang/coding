#ifndef _KEY_H
#define _KEY_H
#include "stm32h7xx_hal.h"

#define KEY0        HAL_GPIO_ReadPin(GPIOH,GPIO_PIN_3)  //KEY0 PH3
#define KEY1        HAL_GPIO_ReadPin(GPIOH,GPIO_PIN_2)  //KEY1 PH2
#define KEY2        HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13) //KEY2 PC13
#define WK_UP       HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)  //WKUP PA0

#define KEY0_PRES 	1  	//KEY0���º󷵻�ֵ
#define KEY1_PRES	2	//KEY1���º󷵻�ֵ
#define KEY2_PRES	3	//KEY2���º󷵻�ֵ
#define WKUP_PRES   4	//WKUP���º󷵻�ֵ

void KEY_Init(void);
#endif
