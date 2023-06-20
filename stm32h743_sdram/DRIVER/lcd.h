#ifndef __LCD_H
#define __LCD_H	

#include "stm32h7xx_hal.h"
#include "stdlib.h" 

/* LCD 参数 */
typedef struct  
{		 	 
	uint16_t width;			//LCD 宽度
	uint16_t height;		//LCD 高度
	uint16_t id;			//LCD ID
	uint8_t  dir;			//横屏 or 竖屏： 0-竖屏， 1-横屏
	uint16_t wramcmd;		//开始写gram指令
	uint16_t setxcmd;		//设置x坐标
	uint16_t setycmd;		//设置y坐标
}_lcd_dev; 	  

extern _lcd_dev lcddev;		//LCD 参数
extern uint32_t  POINT_COLOR;//画笔颜色，默认红色   
extern uint32_t  BACK_COLOR; //背景颜色，默认白色

//LCD MPU保护参数
#define LCD_REGION_NUMBER		MPU_REGION_NUMBER0
#define LCD_ADDRESS_START		(0x60000000)
#define LCD_REGION_SIZE			MPU_REGION_SIZE_256MB

// LCD 背光控制引脚 PB5
#define LCD_LED(n) (n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET)) 	    

//LCD 地址结构体
typedef struct
{
	__IO uint16_t LCD_REG;
	__IO uint16_t LCD_RAM;
} LCD_TypeDef;
/* 使用NOR/SRAM的 Bank1.sector4,地址位HADDR[27,26]=11 A18作为数据命令区分线
 * 注意！设置时，STM32内部会有移一位对齐			     */
#define LCD_BASE        ((uint32_t)(0x60000000 | 0x0007FFFE))
#define LCD             ((LCD_TypeDef *) LCD_BASE)
	 
//扫描方向定义
#define L2R_U2D  0 		//从左到右，从上到下
#define L2R_D2U  1 		//从左到右，从下到上
#define R2L_U2D  2 		//从右到左，从上到下
#define R2L_D2U  3 		//从右到左，从下到上

#define U2D_L2R  4 		//从上到下，从左到右
#define U2D_R2L  5 		//从上到下，从右到左
#define D2U_L2R  6 		//从下到上，从左到右
#define D2U_R2L  7		//从下到上，从右到左 

#define DFT_SCAN_DIR  L2R_U2D  //默认扫描方向，从左到右，从上到下

//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40
#define BRRED 			 0XFC07
#define GRAY  			 0X8430 

//GUI颜色
#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上3色为PANEL的颜色
 
#define LIGHTGREEN     	 0X841F
//#define LIGHTGRAY        0XEF5B
#define LGRAY 			 0XC618 // 浅灰色，窗体背景色
#define LGRAYBLUE        0XA651 // 浅蓝色，中间层颜色 
#define LBBLUE           0X2B12 // 浅棕蓝色，选择条目的反色
	    															  
void LCD_Init(void);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_Clear(uint32_t Color);
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
void LCD_DrawPoint(uint16_t x,uint16_t y);
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint32_t color);
uint32_t  LCD_ReadPoint(uint16_t x,uint16_t y);
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r);
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint32_t color);
void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color);
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode);
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size);
void LCD_ShowxNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode);
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,uint8_t *p);

void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue);
uint16_t LCD_ReadReg(uint16_t LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(uint16_t RGB_Code);
void LCD_SSD_BackLightSet(uint8_t pwm);
void LCD_Scan_Dir(uint8_t dir);
void LCD_Display_Dir(uint8_t dir);
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height);
//LCD分辨率设置
#define SSD_HOR_RESOLUTION		800		//LCD水平分辨率
#define SSD_VER_RESOLUTION		480		//LCD垂直分辨率
//LCD驱动参数设置
#define SSD_HOR_PULSE_WIDTH		1		//水平脉宽
#define SSD_HOR_BACK_PORCH		46		//水平前廊
#define SSD_HOR_FRONT_PORCH		210		//水平后廊

#define SSD_VER_PULSE_WIDTH		1		//垂直脉宽
#define SSD_VER_BACK_PORCH		23		//垂直前廊
#define SSD_VER_FRONT_PORCH		22		//垂直后廊
// 如下几个参数，自动计算
#define SSD_HT	(SSD_HOR_RESOLUTION + SSD_HOR_BACK_PORCH + SSD_HOR_FRONT_PORCH)
#define SSD_HPS	(SSD_HOR_BACK_PORCH)
#define SSD_VT 	(SSD_VER_RESOLUTION + SSD_VER_BACK_PORCH + SSD_VER_FRONT_PORCH)
#define SSD_VPS (SSD_VER_BACK_PORCH)

#endif  
	 
	 



