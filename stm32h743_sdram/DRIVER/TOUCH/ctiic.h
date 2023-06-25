#ifndef __MYCT_IIC_H
#define __MYCT_IIC_H

#include "stm32h7xx_hal.h"	 

#define CT_SDA_IN()  {GPIOI->MODER&=~(3<<(3*2));GPIOI->MODER|=0<<3*2;}
#define CT_SDA_OUT() {GPIOI->MODER&=~(3<<(3*2));GPIOI->MODER|=1<<3*2;}

#define CT_IIC_SCL(n) (n?HAL_GPIO_WritePin(GPIOH,GPIO_PIN_6,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOH,GPIO_PIN_6,GPIO_PIN_RESET))//SCL
#define CT_IIC_SDA(n) (n?HAL_GPIO_WritePin(GPIOI,GPIO_PIN_3,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOI,GPIO_PIN_3,GPIO_PIN_RESET))//SDA	 
#define CT_READ_SDA   HAL_GPIO_ReadPin(GPIOI,GPIO_PIN_3)
 
void CT_IIC_Init(void);
void CT_IIC_Start(void);
void CT_IIC_Stop(void);
void CT_IIC_Send_Byte(uint8_t txd);
uint8_t CT_IIC_Read_Byte(unsigned char ack);
uint8_t CT_IIC_Wait_Ack(void);
void CT_IIC_Ack(void);
void CT_IIC_NAck(void);

#endif







