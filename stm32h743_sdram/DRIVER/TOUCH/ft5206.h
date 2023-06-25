#ifndef __FT5206_H
#define __FT5206_H	
#include "stm32h7xx_hal.h"	

#define FT_RST(n)  (n?HAL_GPIO_WritePin(GPIOI,GPIO_PIN_8,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOI,GPIO_PIN_8,GPIO_PIN_RESET))//FT5206��λ����
#define FT_INT      HAL_GPIO_ReadPin(GPIOH,GPIO_PIN_7)
   
#define FT_CMD_WR 				0X70
#define FT_CMD_RD 				0X71
  
#define FT_DEVIDE_MODE 			0x00
#define FT_REG_NUM_FINGER       0x02

#define FT_TP1_REG 				0X03
#define FT_TP2_REG 				0X09
#define FT_TP3_REG 				0X0F
#define FT_TP4_REG 				0X15
#define FT_TP5_REG 				0X1B
 

#define	FT_ID_G_LIB_VERSION		0xA1
#define FT_ID_G_MODE 			0xA4
#define FT_ID_G_THGROUP			0x80
#define FT_ID_G_PERIODACTIVE	0x88


uint8_t FT5206_WR_Reg(uint16_t reg,uint8_t *buf,uint8_t len);
void FT5206_RD_Reg(uint16_t reg,uint8_t *buf,uint8_t len);
uint8_t FT5206_Init(void);
uint8_t FT5206_Scan(uint8_t mode);

#endif



