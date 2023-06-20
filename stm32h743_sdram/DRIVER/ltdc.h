#ifndef _LCD_H
#define _LCD_H
#include "stm32h7xx_hal.h"

// PD13： LCD背光控制
#define LCD_LED(n)              (n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET))

typedef struct  
{							 
	uint32_t pwidth;		//LCD面板的宽度，固定参数，不随显示方向更改，如果0，表示没有RGB屏幕接入
	uint32_t pheight;		//LCD面板的高度
	uint16_t hsw;			//水平同步宽度
	uint16_t vsw;			//垂直同步宽度
	uint16_t hbp;			//水平后廊
	uint16_t vbp;			//垂直后廊
	uint16_t hfp;			//水平前廊
	uint16_t vfp;			//垂直前廊
	uint8_t activelayer;	//当前层编号:0/1	
	uint8_t dir;			//0,竖屏;1,横屏
	uint16_t width;			//LCD宽度
	uint16_t height;		//LCD高度
	uint32_t pixsize;		//每个像素所占用字节数
}_ltdc_dev; 

extern _ltdc_dev lcdltdc;
extern LTDC_HandleTypeDef LTDC_Handler;
extern DMA2D_HandleTypeDef DMA2D_Handler;
extern uint32_t *ltdc_framebuf[2];	//LTDC LCD֡帧缓存数组指针,必须指向对应大小的内存区域

#define LCD_PIXEL_FORMAT_ARGB8888       0X00    
#define LCD_PIXEL_FORMAT_RGB888         0X01    
#define LCD_PIXEL_FORMAT_RGB565         0X02       
#define LCD_PIXEL_FORMAT_ARGB1555       0X03      
#define LCD_PIXEL_FORMAT_ARGB4444       0X04     
#define LCD_PIXEL_FORMAT_L8             0X05     
#define LCD_PIXEL_FORMAT_AL44           0X06     
#define LCD_PIXEL_FORMAT_AL88           0X07      

//定义颜色像素格式，一般用RGB565
#define LCD_PIXFORMAT				LCD_PIXEL_FORMAT_RGB565	
//定义默认背景颜色
#define LTDC_BACKLAYERCOLOR			0XFFFFFFFF	
//LCD帧缓冲区首地址，这里定义在SDRAM里面
#define LCD_FRAME_BUF_ADDR			0XC0000000  

void LTDC_Switch(uint8_t sw);
void LTDC_Layer_Switch(uint8_t layerx,uint8_t sw);
void LTDC_Select_Layer(uint8_t layerx);
void LTDC_Display_Dir(uint8_t dir);
void LTDC_Draw_Point(uint16_t x,uint16_t y,uint32_t color);
uint32_t LTDC_Read_Point(uint16_t x,uint16_t y);
void LTDC_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint32_t color);
void LTDC_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color);
void LTDC_Clear(uint32_t color);
uint8_t LTDC_Clk_Set(uint32_t pll3m,uint32_t pll3n,uint32_t pll3r);
//uint8_t LTDC_Clk_Set(uint32_t pllsain,uint32_t pllsair,uint32_t pllsaidivr);
void LTDC_Layer_Window_Config(uint8_t layerx,uint16_t sx,uint16_t sy,uint16_t width,uint16_t height);
void LTDC_Layer_Parameter_Config(uint8_t layerx,uint32_t bufaddr,uint8_t pixformat,uint8_t alpha,uint8_t alpha0,uint8_t bfac1,uint8_t bfac2,uint32_t bkcolor);
uint16_t LTDC_PanelID_Read(void);
void LTDC_Init(void);

#endif 

