#include "key.h"

void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;

	__HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    GPIO_Initure.Pin = GPIO_PIN_0;            //PA0
    GPIO_Initure.Mode = GPIO_MODE_IT_RISING;
    GPIO_Initure.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);
    
    GPIO_Initure.Pin = GPIO_PIN_13;           //PC13
    GPIO_Initure.Mode = GPIO_MODE_IT_RISING;
    GPIO_Initure.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &GPIO_Initure);
    
    GPIO_Initure.Pin = GPIO_PIN_2|GPIO_PIN_3; //PH2,3
    HAL_GPIO_Init(GPIOH, &GPIO_Initure);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    HAL_NVIC_SetPriority(EXTI2_IRQn, 2, 1);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);

    HAL_NVIC_SetPriority(EXTI3_IRQn, 2, 2);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 3);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

