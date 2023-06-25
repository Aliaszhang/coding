#ifndef __TOUCH_H__
#define __TOUCH_H__
#include "stm32h7xx_hal.h"   
#include "ft5206.h"	    

#define TP_PRES_DOWN 0x80  //触屏被按下
#define TP_CATH_PRES 0x40  //有按键按下 
#define CT_MAX_TOUCH  5    //电容屏支持的点数，固定为5

typedef struct
{
	uint8_t (*init)(void);
	uint8_t (*scan)(uint8_t);
	void (*adjust)(void);
	uint16_t x[CT_MAX_TOUCH];   //当前坐标
	uint16_t y[CT_MAX_TOUCH];
	uint8_t  sta;				
								//b7:1-按下/0-松开; 
	                            //b6:0-没有按键按下;1-有按键按下
								//b5:保留
								//b4~b0:按下的点数
	float xfac;					
	float yfac;
	short xoff;
	short yoff;	   
//b0:0,竖屏
//   1,横屏
//b1~6:保留.
//b7:0,电阻屏
//   1,电容屏 
	uint8_t touchtype;
}_m_tp_dev;

extern _m_tp_dev tp_dev;

#define PEN         HAL_GPIO_ReadPin(GPIOH,GPIO_PIN_7) //T_PEN
#define DOUT        HAL_GPIO_ReadPin(GPIOG,GPIO_PIN_3) //T_MISO
#define TDIN(n)     (n?HAL_GPIO_WritePin(GPIOI,GPIO_PIN_3,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOI,GPIO_PIN_3,GPIO_PIN_RESET))//T_MOSI
#define TCLK(n)     (n?HAL_GPIO_WritePin(GPIOH,GPIO_PIN_6,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOH,GPIO_PIN_6,GPIO_PIN_RESET))//T_SCK
#define TCS(n)      (n?HAL_GPIO_WritePin(GPIOI,GPIO_PIN_8,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOI,GPIO_PIN_8,GPIO_PIN_RESET))//T_CS  
   
void TP_Write_Byte(uint8_t num);
uint16_t TP_Read_AD(uint8_t CMD);
uint16_t TP_Read_XOY(uint8_t xy);
uint8_t TP_Read_XY(uint16_t *x,uint16_t *y);
uint8_t TP_Read_XY2(uint16_t *x,uint16_t *y);
void TP_Drow_Touch_Point(uint16_t x,uint16_t y,uint16_t color);
void TP_Draw_Big_Point(uint16_t x,uint16_t y,uint16_t color);

// uint8_t TP_Scan(uint8_t tp);
uint8_t TP_Init(void);
 
#endif

















