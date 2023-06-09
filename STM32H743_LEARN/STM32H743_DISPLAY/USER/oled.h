#ifndef __OLED_H
#define __OLED_H			  	 
#include "main.h"
#include "stdlib.h"	    

#define OLED_MODE 	0 
		    						  
//-----------------OLED�˿ڶ���----------------  					   
#define OLED_CS(x) 		HAL_GPIO_WritePin(GPIOG,GPIO_PIN_10,(x == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET))	 
#define OLED_RST(x)  	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_14,(x == 0 ? GPIO_PIN_RESET : GPIO_PIN_SET)) 
#define OLED_DC(x) 		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_13,(x == 0? GPIO_PIN_RESET : GPIO_PIN_SET))	// DC 
		     
#define OLED_CMD  	0		//д����
#define OLED_DATA 	1		//д����

//OLED�����ú���
void OLED_WR_Byte(uint8_t dat,uint8_t cmd);	    
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Refresh_Gram(void);		   
							   		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t);
void OLED_Fill(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t dot);
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size,uint8_t mode);
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size);
void OLED_ShowString(uint8_t x,uint8_t y,const uint8_t *p,uint8_t size);

void OLED_ShowChinese(uint8_t x,uint8_t y,uint8_t size,uint8_t mode);
#endif  
	 



