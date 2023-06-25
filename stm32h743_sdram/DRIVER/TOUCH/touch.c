#include "touch.h" 
#include "lcd.h"
#include "stdlib.h"
#include "math.h"

_m_tp_dev tp_dev=
{
	TP_Init,
	0,
	0,
	0,
	0, 
	0,
	0,
	0,
	0,	  	 		
	0,
	0,	  	 		
};					

uint8_t CMD_RDX=0XD0;
uint8_t CMD_RDY=0X90;
 
extern void delay_us(uint32_t nus);

#if 0
//spi写数据
void TP_Write_Byte(uint8_t num)    
{  
	uint8_t count=0;   
	for(count=0;count<8;count++)  
	{ 	  
		if(num&0x80)TDIN(1);  
		else TDIN(0);   
		num<<=1;    
		TCLK(0); 
		delay_us(1);
		TCLK(1);//上升沿有效      
	}		 			    
} 		 
// spi读数据
uint16_t TP_Read_AD(uint8_t CMD)	  
{ 	 
	uint8_t count=0; 	  
	uint16_t Num=0; 
	TCLK(0);		//拉低时钟
	TDIN(0); 	    //拉低数据线
	TCS(0); 		//选中触摸屏IC
	TP_Write_Byte(CMD);//发送命令字
	delay_us(16);   //ADS7846最长处理时间6us
	TCLK(0); 	     	    
	delay_us(1);    	   
	TCLK(1);		//给一个时钟，清除BUSY
	delay_us(1);    
	TCLK(0); 	     	    
	for(count=0;count<16;count++)//读出16bit数据，只用12bits
	{ 				  
		Num<<=1; 	 
		TCLK(0);	//下降沿有效
		delay_us(1);    
 		TCLK(1);
 		if(DOUT)Num++; 		 
	}  	
	Num>>=4;   	//ֻ只有高12bit有效
	TCS(1);		//释放片选
	return(Num);   
}

#define READ_TIMES 5 	//读取次数
#define LOST_VAL 1	  	//丢弃值
uint16_t TP_Read_XOY(uint8_t xy)
{
	uint16_t i, j;
	uint16_t buf[READ_TIMES];
	uint16_t sum=0;
	uint16_t temp;
	for(i=0;i<READ_TIMES;i++)buf[i]=TP_Read_AD(xy);		 		    
	for(i=0;i<READ_TIMES-1; i++)
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j])//升序排列
			{
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}	  
	sum=0;
	for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;   
} 

uint8_t TP_Read_XY(uint16_t *x,uint16_t *y)
{
	uint16_t xtemp,ytemp;			 	 		  
	xtemp=TP_Read_XOY(CMD_RDX);
	ytemp=TP_Read_XOY(CMD_RDY);	  												   
	//if(xtemp<100||ytemp<100)return 0;//读取失败
	*x=xtemp;
	*y=ytemp;
	return 1;//成功
}

#define ERR_RANGE 50 //误差范围

//连续读取两次，偏差不能超过ERR_RANGE
uint8_t TP_Read_XY2(uint16_t *x,uint16_t *y) 
{
	uint16_t x1,y1;
 	uint16_t x2,y2;
 	uint8_t flag;    
    flag=TP_Read_XY(&x1,&y1);   
    if(flag==0)return(0);
    flag=TP_Read_XY(&x2,&y2);	   
    if(flag==0)return(0);   
    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))
    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    }else return 0;	  
}   

void TP_Drow_Touch_Point(uint16_t x,uint16_t y,uint16_t color)
{
	POINT_COLOR=color;
	LCD_DrawLine(x-12,y,x+13,y);//横线
	LCD_DrawLine(x,y-12,x,y+13);//竖线
	LCD_DrawPoint(x+1,y+1);
	LCD_DrawPoint(x-1,y+1);
	LCD_DrawPoint(x+1,y-1);
	LCD_DrawPoint(x-1,y-1);
	LCD_Draw_Circle(x,y,6);//画中心圆
}	  

void TP_Draw_Big_Point(uint16_t x,uint16_t y,uint16_t color)
{	    
	POINT_COLOR=color;
	LCD_DrawPoint(x,y);//中心点 
	LCD_DrawPoint(x+1,y);
	LCD_DrawPoint(x,y+1);
	LCD_DrawPoint(x+1,y+1);	 	  	
}							  

uint8_t TP_Scan(uint8_t tp)
{			   
	if(PEN==0)//有按键按下
	{
		if(tp)TP_Read_XY2(&tp_dev.x[0],&tp_dev.y[0]);//读取物理坐标
		else if(TP_Read_XY2(&tp_dev.x[0],&tp_dev.y[0]))//读取屏幕坐标
		{
	 		tp_dev.x[0]=tp_dev.xfac*tp_dev.x[0]+tp_dev.xoff;//转换为屏幕坐标
			tp_dev.y[0]=tp_dev.yfac*tp_dev.y[0]+tp_dev.yoff;  
	 	} 
		if((tp_dev.sta&TP_PRES_DOWN)==0)//֮之前未按下
		{		 
			tp_dev.sta=TP_PRES_DOWN|TP_CATH_PRES;//按键按下
			tp_dev.x[4]=tp_dev.x[0];//记录第一次按下时的坐标
			tp_dev.y[4]=tp_dev.y[0];  	   			 
		}			   
	}else
	{
		if(tp_dev.sta&TP_PRES_DOWN)//֮之前已经按下
		{
			tp_dev.sta&=~(1<<7);//标记按键松开
		}else//之前就没有被按下
		{
			tp_dev.x[4]=0;
			tp_dev.y[4]=0;
			tp_dev.x[0]=0xffff;
			tp_dev.y[0]=0xffff;
		}	    
	}
	return tp_dev.sta&TP_PRES_DOWN;//返回当前的触摸屏状态
}
#endif

uint8_t TP_Init(void)
{	
    GPIO_InitTypeDef GPIO_Initure;
	if(lcddev.id==0X1963||lcddev.id==0X7084||lcddev.id==0X7016)//SSD1963 7吋屏，或者7吋800*480/1024*600 RGB屏
	{
		FT5206_Init();
		tp_dev.scan=FT5206_Scan;		
		tp_dev.touchtype|=0X80;			//电容屏
		tp_dev.touchtype|=lcddev.dir&0X01;//横屏or竖屏
		return 0;
	}
	return 1; 									 
}









