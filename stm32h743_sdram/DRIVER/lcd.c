#include "lcd.h"
#include "font.h"  	 
#include "ltdc.h"
#include "stdio.h"

SRAM_HandleTypeDef SRAM_Handler;    //SRAM句柄，用于控制LCD
//LCD画笔颜色和背景颜色
uint32_t POINT_COLOR=0xFF000000;    //画笔颜色
uint32_t BACK_COLOR =0xFFFFFFFF;  	//背景颜色

//默认为竖屏
_lcd_dev lcddev;
    
//写寄存器函数，regval:寄存器值
void LCD_WR_REG(__IO uint16_t regval)
{   
    LCD->LCD_REG = regval;
}

void LCD_WR_DATA(__IO uint16_t data)
{	  
    LCD->LCD_RAM=data;		 
}

uint16_t LCD_RD_DATA(void)
{
    __IO uint16_t ram;
    ram = LCD->LCD_RAM;	
    return ram;	 
}					   

//LCD_Reg:寄存器号
//LCD_RegValue: 代写入数据
void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue)
{	
    LCD->LCD_REG = LCD_Reg;	 
    LCD->LCD_RAM = LCD_RegValue;
}	   

//LCD_Reg:寄存器地址
//返回寄存器值
uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{										   
    LCD_WR_REG(LCD_Reg);
    HAL_Delay(1);
    __NOP();
    __NOP();		  
    return LCD_RD_DATA();
}   

// 写GRAM
void LCD_WriteRAM_Prepare(void)
{
    LCD->LCD_REG = lcddev.wramcmd;	  
}	 

//RGB_Code:颜色值
void LCD_WriteRAM(uint16_t RGB_Code)
{							    
    LCD->LCD_RAM = RGB_Code; //写16位GRAM
}
/* 从ILI93xx读出的数据为GBR格式，而我们写入的时候时RGB格式，使用此函数进行转换
* @param:  GBR格式的颜色值
* @return: RGB格式的颜色值
*/
uint16_t LCD_BGR2RGB(uint16_t c)
{
    uint16_t  r,g,b,rgb;   
    b = (c>>0)&0x1f;
    g = (c>>5)&0x3f;
    r = (c>>11)&0x1f;	 
    rgb = (b<<11)+(g<<5)+(r<<0);		 
    return(rgb);
} 

void opt_delay(uint8_t i)
{
    while(i--);
}

//读取某个点的颜色
uint32_t LCD_ReadPoint(uint16_t x,uint16_t y)
{
    uint16_t r=0,g=0,b=0;
    if(x >= lcddev.width || y >= lcddev.height)
        return 0;
    if(lcdltdc.pwidth != 0)
    {
        return LTDC_Read_Point(x,y);
    }
    LCD_SetCursor(x,y);	    
    if(lcddev.id==0X9341||lcddev.id==0X5310||lcddev.id==0X1963)
        LCD_WR_REG(0X2E); //9341/3510/1963
    else if(lcddev.id==0X5510)
        LCD_WR_REG(0X2E00);	//5510

    r=LCD_RD_DATA();								//dummy Read	   
    if(lcddev.id==0X1963)
        return r;					//1963, 可以直接读

    opt_delay(2);	  

    r = LCD_RD_DATA(); // 实际坐标的颜色

    // 9341/NT35310/NT35510要分两次读取出来
    opt_delay(2);	  

    b = LCD_RD_DATA(); 
    g = r & 0XFF;		//9341/5310/5510,第一次读取的时RG的值，R在前，G在后，各占8位
    g <<= 8; 
    return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11));	//ILI9341/NT35310/NT35510需要用公式转换
}			 

void LCD_DisplayOn(void)
{					   
    if(lcdltdc.pwidth!=0)
        LTDC_Switch(1);     // 开启LCD
    else if(lcddev.id==0X9341||lcddev.id==0X5310||lcddev.id==0X1963)
        LCD_WR_REG(0X29);   // 开启显示
    else if(lcddev.id==0X5510)
        LCD_WR_REG(0X2900); // 开启显示
}	 

void LCD_DisplayOff(void)
{	   
    if(lcdltdc.pwidth!=0)
        LTDC_Switch(0);     //关闭LCD
    else if(lcddev.id==0X9341||lcddev.id==0X5310||lcddev.id==0X1963)
        LCD_WR_REG(0X28);	//关闭显示
    else if(lcddev.id==0X5510)
        LCD_WR_REG(0X2800);	//关闭显示
}

/* 设置光标的位置，对RGB屏无效 */
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{	 
    if(lcddev.id==0X9341||lcddev.id==0X5310)
    {		    
        LCD_WR_REG(lcddev.setxcmd); 
        LCD_WR_DATA(Xpos>>8);LCD_WR_DATA(Xpos&0XFF); 			 
        LCD_WR_REG(lcddev.setycmd); 
        LCD_WR_DATA(Ypos>>8);LCD_WR_DATA(Ypos&0XFF); 		
    }
    else if(lcddev.id==0X1963)
    {  			 		
        if(lcddev.dir==0)//x坐标需要转换
        {
            Xpos=lcddev.width-1-Xpos;
            LCD_WR_REG(lcddev.setxcmd); 
            LCD_WR_DATA(0);LCD_WR_DATA(0); 		
            LCD_WR_DATA(Xpos>>8);LCD_WR_DATA(Xpos&0XFF);		 	 
        }else
        {
            LCD_WR_REG(lcddev.setxcmd); 
            LCD_WR_DATA(Xpos>>8);LCD_WR_DATA(Xpos&0XFF); 		
            LCD_WR_DATA((lcddev.width-1)>>8);LCD_WR_DATA((lcddev.width-1)&0XFF);		 	 			
        }	
        LCD_WR_REG(lcddev.setycmd); 
        LCD_WR_DATA(Ypos>>8);LCD_WR_DATA(Ypos&0XFF); 		
        LCD_WR_DATA((lcddev.height-1)>>8);LCD_WR_DATA((lcddev.height-1)&0XFF); 			 		
        
    }else if(lcddev.id==0X5510)
    {
        LCD_WR_REG(lcddev.setxcmd);LCD_WR_DATA(Xpos>>8); 		
        LCD_WR_REG(lcddev.setxcmd+1);LCD_WR_DATA(Xpos&0XFF);			 
        LCD_WR_REG(lcddev.setycmd);LCD_WR_DATA(Ypos>>8);  		
        LCD_WR_REG(lcddev.setycmd+1);LCD_WR_DATA(Ypos&0XFF);			
    } 
} 		 
/* 
* 设置LCD的自动扫描方向（RGB屏无效），注意！其他函数会受到此函数的影响，尤其是9341
* 所以一般设置为L2R, U2D，其他方式可能无法正常显示 
* @param : dir:0~7, 代表8个方向，具体见头文件定义
* 9341/5310/5510/1963等IC已经实际测试没问题 
*/
void LCD_Scan_Dir(uint8_t dir)
{
    uint16_t regval=0;
    uint16_t dirreg=0;
    uint16_t temp;  
    if((lcddev.dir==1&&lcddev.id!=0X1963)||(lcddev.dir==0&&lcddev.id==0X1963))//横屏时对1963不改变扫描方向，竖屏时1963改变扫描方向
    {			   
        switch(dir)//方向转换
        {
        case 0:
            dir = 6;
            break;
        case 1:
            dir = 7;
            break;
        case 2:
            dir = 4;
            break;
        case 3:
            dir = 5;
            break;
        case 4:
            dir = 1;
            break;
        case 5:
            dir = 0;
            break;
        case 6:
            dir = 3;
            break;
        case 7:
            dir = 2;
            break;
        default:
            dir = 6;
            break;	     
        }
    }

    if(lcddev.id==0x9341||lcddev.id==0X5310||lcddev.id==0X5510||lcddev.id==0X1963)//9341/5310/5510/1963,特殊处理
    {
        switch(dir)
        {
        case L2R_U2D:
            regval|=(0<<7)|(0<<6)|(0<<5); 
            break;
        case L2R_D2U:
            regval|=(1<<7)|(0<<6)|(0<<5); 
            break;
        case R2L_U2D:
            regval|=(0<<7)|(1<<6)|(0<<5); 
            break;
        case R2L_D2U:
            regval|=(1<<7)|(1<<6)|(0<<5); 
            break;	 
        case U2D_L2R:
            regval|=(0<<7)|(0<<6)|(1<<5); 
            break;
        case U2D_R2L:
            regval|=(0<<7)|(1<<6)|(1<<5); 
            break;
        case D2U_L2R:
            regval|=(1<<7)|(0<<6)|(1<<5); 
            break;
        case D2U_R2L:
            regval|=(1<<7)|(1<<6)|(1<<5); 
            break;	 
        }

        if(lcddev.id==0X5510)
            dirreg=0X3600;
        else 
            dirreg=0X36;

        if((lcddev.id!=0X5310)&&(lcddev.id!=0X5510)&&(lcddev.id!=0X1963))
            regval|=0X08;//5310/5510/1963不需要BGR   
        LCD_WriteReg(dirreg,regval);
        if(lcddev.id!=0X1963)//1963不做坐标处理
        {
            if(regval&0X20)
            {
                if(lcddev.width<lcddev.height)//交换X,Y
                {
                    temp=lcddev.width;
                    lcddev.width=lcddev.height;
                    lcddev.height=temp;
                }
            }else  
            {
                if(lcddev.width>lcddev.height)//交换X,Y
                {
                    temp=lcddev.width;
                    lcddev.width=lcddev.height;
                    lcddev.height=temp;
                }
            }  
        }
        if(lcddev.id==0X5510)
        {
            LCD_WR_REG(lcddev.setxcmd);LCD_WR_DATA(0); 
            LCD_WR_REG(lcddev.setxcmd+1);LCD_WR_DATA(0); 
            LCD_WR_REG(lcddev.setxcmd+2);LCD_WR_DATA((lcddev.width-1)>>8); 
            LCD_WR_REG(lcddev.setxcmd+3);LCD_WR_DATA((lcddev.width-1)&0XFF); 
            LCD_WR_REG(lcddev.setycmd);LCD_WR_DATA(0); 
            LCD_WR_REG(lcddev.setycmd+1);LCD_WR_DATA(0); 
            LCD_WR_REG(lcddev.setycmd+2);LCD_WR_DATA((lcddev.height-1)>>8); 
            LCD_WR_REG(lcddev.setycmd+3);LCD_WR_DATA((lcddev.height-1)&0XFF);
        }else
        {
            LCD_WR_REG(lcddev.setxcmd); 
            LCD_WR_DATA(0);LCD_WR_DATA(0);
            LCD_WR_DATA((lcddev.width-1)>>8);LCD_WR_DATA((lcddev.width-1)&0XFF);
            LCD_WR_REG(lcddev.setycmd); 
            LCD_WR_DATA(0);LCD_WR_DATA(0);
            LCD_WR_DATA((lcddev.height-1)>>8);LCD_WR_DATA((lcddev.height-1)&0XFF);  
        }
    } 
}     

void LCD_DrawPoint(uint16_t x,uint16_t y)
{
    if(lcdltdc.pwidth!=0) // RGB屏幕
    {
        LTDC_Draw_Point(x,y,POINT_COLOR);
    }else
    {
        LCD_SetCursor(x,y);
        LCD_WriteRAM_Prepare();
        LCD->LCD_RAM=POINT_COLOR; 
    }
}

// 快速画点
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint32_t color)
{	   
    if(lcdltdc.pwidth!=0)// RGB屏幕
    {
        LTDC_Draw_Point(x,y,color);
        return;
    }else if(lcddev.id==0X9341||lcddev.id==0X5310)
    {
        LCD_WR_REG(lcddev.setxcmd); 
        LCD_WR_DATA(x>>8);LCD_WR_DATA(x&0XFF);  			 
        LCD_WR_REG(lcddev.setycmd); 
        LCD_WR_DATA(y>>8);LCD_WR_DATA(y&0XFF); 		 	 
    }else if(lcddev.id==0X5510)
    {
        LCD_WR_REG(lcddev.setxcmd);LCD_WR_DATA(x>>8);  
        LCD_WR_REG(lcddev.setxcmd+1);LCD_WR_DATA(x&0XFF);	  
        LCD_WR_REG(lcddev.setycmd);LCD_WR_DATA(y>>8);  
        LCD_WR_REG(lcddev.setycmd+1);LCD_WR_DATA(y&0XFF); 
    }else if(lcddev.id==0X1963)
    {
        if(lcddev.dir==0)x=lcddev.width-1-x;
        LCD_WR_REG(lcddev.setxcmd); 
        LCD_WR_DATA(x>>8);LCD_WR_DATA(x&0XFF); 		
        LCD_WR_DATA(x>>8);LCD_WR_DATA(x&0XFF); 		
        LCD_WR_REG(lcddev.setycmd); 
        LCD_WR_DATA(y>>8);LCD_WR_DATA(y&0XFF); 		
        LCD_WR_DATA(y>>8);LCD_WR_DATA(y&0XFF); 		
    }		 
    LCD->LCD_REG=lcddev.wramcmd; 
    LCD->LCD_RAM=color; 
}

//SSD1963 背光设置
//pwm:背光等级,0~100 越大越亮
void LCD_SSD_BackLightSet(uint8_t pwm)
{	
    LCD_WR_REG(0xBE);	    //配置PWM输出
    LCD_WR_DATA(0x05);	    //1设置PWM频率
    LCD_WR_DATA(pwm*2.55);  //2设置PWM占空比
    LCD_WR_DATA(0x01);	    //3设置C
    LCD_WR_DATA(0xFF);	    //4设置D
    LCD_WR_DATA(0x00);	    //5设置E
    LCD_WR_DATA(0x00);	    //6设置F
}

//设置LCD显示方向
//dir:0,横屏；1,竖屏
void LCD_Display_Dir(uint8_t dir)
{
    lcddev.dir=dir;
    if(lcdltdc.pwidth!=0)   //RGB屏幕
    {
        LTDC_Display_Dir(dir);
        lcddev.width=lcdltdc.width;
        lcddev.height=lcdltdc.height;
        return;
    }
    if(dir==0)
    {
        lcddev.width=240;
        lcddev.height=320;
        if(lcddev.id==0X9341||lcddev.id==0X5310)
        {
            lcddev.wramcmd=0X2C;
            lcddev.setxcmd=0X2A;
            lcddev.setycmd=0X2B;  	 
            if(lcddev.id==0X5310)
            {
                lcddev.width=320;
                lcddev.height=480;
            }
        }else if(lcddev.id==0x5510)
        {
            lcddev.wramcmd=0X2C00;
            lcddev.setxcmd=0X2A00;
            lcddev.setycmd=0X2B00; 
            lcddev.width=480;
            lcddev.height=800;
        }else if(lcddev.id==0X1963)
        {
            lcddev.wramcmd=0X2C;
            lcddev.setxcmd=0X2B;
            lcddev.setycmd=0X2A;
            lcddev.width=480;
            lcddev.height=800;
        }
    }
    else
    {	  				
        lcddev.width=320;
        lcddev.height=240;
        if(lcddev.id==0X9341||lcddev.id==0X5310)
        {
            lcddev.wramcmd=0X2C;
            lcddev.setxcmd=0X2A;
            lcddev.setycmd=0X2B;  	 
        }else if(lcddev.id==0x5510)
        {
            lcddev.wramcmd=0X2C00;
            lcddev.setxcmd=0X2A00;
            lcddev.setycmd=0X2B00; 
            lcddev.width=800;
            lcddev.height=480;
        }else if(lcddev.id==0X1963)
        {
            lcddev.wramcmd=0X2C;
            lcddev.setxcmd=0X2A;
            lcddev.setycmd=0X2B;
            lcddev.width=800;
            lcddev.height=480;
        }
        if(lcddev.id==0X5310)
        { 	 
            lcddev.width=480;
            lcddev.height=320; 			
        }
    } 
    LCD_Scan_Dir(DFT_SCAN_DIR);	//默认扫描方向
}

//设置窗口，并自动设置画点坐标到窗口左上角（对RGB屏幕无效）
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
{     
    uint16_t twidth,theight;
    twidth=sx+width-1;
    theight=sy+height-1;
    if(lcddev.id==0X9341||lcddev.id==0X5310||(lcddev.dir==1&&lcddev.id==0X1963))
    {
        LCD_WR_REG(lcddev.setxcmd); 
        LCD_WR_DATA(sx>>8); 
        LCD_WR_DATA(sx&0XFF);	 
        LCD_WR_DATA(twidth>>8); 
        LCD_WR_DATA(twidth&0XFF);  
        LCD_WR_REG(lcddev.setycmd); 
        LCD_WR_DATA(sy>>8); 
        LCD_WR_DATA(sy&0XFF); 
        LCD_WR_DATA(theight>>8); 
        LCD_WR_DATA(theight&0XFF); 
    }
    else if(lcddev.id==0X1963)//1963竖屏需要特殊处理
    {
        sx=lcddev.width-width-sx; 
        height=sy+height-1; 
        LCD_WR_REG(lcddev.setxcmd); 
        LCD_WR_DATA(sx>>8); 
        LCD_WR_DATA(sx&0XFF);	 
        LCD_WR_DATA((sx+width-1)>>8); 
        LCD_WR_DATA((sx+width-1)&0XFF);  
        LCD_WR_REG(lcddev.setycmd); 
        LCD_WR_DATA(sy>>8); 
        LCD_WR_DATA(sy&0XFF); 
        LCD_WR_DATA(height>>8); 
        LCD_WR_DATA(height&0XFF); 		
    }
    else if(lcddev.id==0X5510)
    {
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(sx>>8);  
        LCD_WR_REG(lcddev.setxcmd+1);
        LCD_WR_DATA(sx&0XFF);	  
        LCD_WR_REG(lcddev.setxcmd+2);
        LCD_WR_DATA(twidth>>8);   
        LCD_WR_REG(lcddev.setxcmd+3);
        LCD_WR_DATA(twidth&0XFF);   
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(sy>>8);   
        LCD_WR_REG(lcddev.setycmd+1);
        LCD_WR_DATA(sy&0XFF);  
        LCD_WR_REG(lcddev.setycmd+2);
        LCD_WR_DATA(theight>>8);   
        LCD_WR_REG(lcddev.setycmd+3);
        LCD_WR_DATA(theight&0XFF);  
    } 
}

void HAL_SRAM_MspInit(SRAM_HandleTypeDef *hsram)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_FMC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    //初始化PD0,1,4,5,7,8,9,10,13,14,15
    GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7|GPIO_PIN_8|\
                        GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;

    GPIO_Initure.Mode=GPIO_MODE_AF_PP;
    GPIO_Initure.Pull=GPIO_PULLUP;
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_Initure.Alternate=GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOD, &GPIO_Initure);

    //初始化PE7,8,9,10,11,12,13,14,15
    GPIO_Initure.Pin=GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|\
                        GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &GPIO_Initure);
}
    
void LCD_Init(void)
{ 	  
    GPIO_InitTypeDef GPIO_Initure;
    FMC_NORSRAM_TimingTypeDef FSMC_ReadWriteTim;
    FMC_NORSRAM_TimingTypeDef FSMC_WriteTim;

    lcddev.id = LTDC_PanelID_Read();	      // 检查是否是RGB屏幕接入
    if(lcddev.id!=0)
    {
        LTDC_Init();			              //ID不等于0表示RGB屏幕接入
    }
    else
    {    
        __HAL_RCC_GPIOB_CLK_ENABLE();
        GPIO_Initure.Pin=GPIO_PIN_5;          //PB5,背光控制
        GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;
        GPIO_Initure.Pull=GPIO_PULLUP;
        GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
        HAL_GPIO_Init(GPIOB,&GPIO_Initure); 
        
        SRAM_Handler.Instance=FMC_NORSRAM_DEVICE;  //BANK1
        SRAM_Handler.Extended=FMC_NORSRAM_EXTENDED_DEVICE;       
        
        SRAM_Handler.Init.NSBank=FMC_NORSRAM_BANK1;
        SRAM_Handler.Init.DataAddressMux=FMC_DATA_ADDRESS_MUX_DISABLE;
        SRAM_Handler.Init.MemoryType=FMC_MEMORY_TYPE_SRAM;
        SRAM_Handler.Init.MemoryDataWidth=FMC_NORSRAM_MEM_BUS_WIDTH_16;
        SRAM_Handler.Init.BurstAccessMode=FMC_BURST_ACCESS_MODE_DISABLE;
        SRAM_Handler.Init.WaitSignalPolarity=FMC_WAIT_SIGNAL_POLARITY_LOW;
        SRAM_Handler.Init.WaitSignalActive=FMC_WAIT_TIMING_BEFORE_WS;
        SRAM_Handler.Init.WriteOperation=FMC_WRITE_OPERATION_ENABLE;
        SRAM_Handler.Init.WaitSignal=FMC_WAIT_SIGNAL_DISABLE;
        SRAM_Handler.Init.ExtendedMode=FMC_EXTENDED_MODE_ENABLE;
        SRAM_Handler.Init.AsynchronousWait=FMC_ASYNCHRONOUS_WAIT_DISABLE;
        SRAM_Handler.Init.WriteBurst=FMC_WRITE_BURST_DISABLE;
        SRAM_Handler.Init.ContinuousClock=FMC_CONTINUOUS_CLOCK_SYNC_ASYNC;
        
        //FSMC读时序控制寄存器
        FSMC_ReadWriteTim.AddressSetupTime=0x11;    //地址建立时间(ADDSET)为17个HCLK 1/216M=4.6ns*17=78ns
        FSMC_ReadWriteTim.AddressHoldTime=0x00;
        FSMC_ReadWriteTim.DataSetupTime=0x55;       //数据保存时间(DATAST)为85个HCLK	=4.6*85=391ns
        FSMC_ReadWriteTim.AccessMode=FMC_ACCESS_MODE_A;
        //FSMC写时序控制寄存器
        FSMC_WriteTim.AddressSetupTime=0x15;        //地址建立时间(ADDSET)为21个HCLK=96ns
        FSMC_WriteTim.AddressHoldTime=0x00;
        FSMC_WriteTim.DataSetupTime=0x15;           //数据保存时间(DATAST)为4.6ns*21个HCLK=96ns
        FSMC_WriteTim.AccessMode=FMC_ACCESS_MODE_A;
        
        HAL_SRAM_Init(&SRAM_Handler,&FSMC_ReadWriteTim,&FSMC_WriteTim);		
        HAL_Delay(50); // delay 50 ms 
        
        //尝试9341 ID读取		
        LCD_WR_REG(0XD3);				   
        lcddev.id=LCD_RD_DATA();	//dummy read 	
        lcddev.id=LCD_RD_DATA();	//读到0X00
        lcddev.id=LCD_RD_DATA();   	//读取93								   
        lcddev.id<<=8;
        lcddev.id|=LCD_RD_DATA();  	//读取41 	   			   
        if(lcddev.id!=0X9341)		//非9341,看看是不是NT35310
        {	 
            LCD_WR_REG(0XD4);				   
            lcddev.id=LCD_RD_DATA();//dummy read  
            lcddev.id=LCD_RD_DATA();
            lcddev.id=LCD_RD_DATA();
            lcddev.id<<=8;	 
            lcddev.id|=LCD_RD_DATA();
            if(lcddev.id!=0X5310)		//不是NT35310,看看是不是NT35510
            {
                LCD_WR_REG(0XDA00);	
                lcddev.id=LCD_RD_DATA();
                LCD_WR_REG(0XDB00);	
                lcddev.id=LCD_RD_DATA();
                lcddev.id<<=8;	
                LCD_WR_REG(0XDC00);	
                lcddev.id|=LCD_RD_DATA();
                if(lcddev.id==0x8000)lcddev.id=0x5510; //NT35510读到的ID是8000H,为了方便区分，强制设置为5510
                if(lcddev.id!=0X5510)			//也不是NT5510,看看是不是SSD1963
                {
                    LCD_WR_REG(0XA1);
                    lcddev.id=LCD_RD_DATA();
                    lcddev.id=LCD_RD_DATA();
                    lcddev.id<<=8;	 
                    lcddev.id|=LCD_RD_DATA();
                    if(lcddev.id==0X5761)lcddev.id=0X1963;//SSD1963读回的ID是5761H,为方便区分，强制设置为1963
                } 
            }
        }   
        if(lcddev.id==0X9341)	//9341的初始化
        {	 
            LCD_WR_REG(0xCF);  
            LCD_WR_DATA(0x00); 
            LCD_WR_DATA(0xC1); 
            LCD_WR_DATA(0X30); 
            LCD_WR_REG(0xED);  
            LCD_WR_DATA(0x64); 
            LCD_WR_DATA(0x03); 
            LCD_WR_DATA(0X12); 
            LCD_WR_DATA(0X81); 
            LCD_WR_REG(0xE8);  
            LCD_WR_DATA(0x85); 
            LCD_WR_DATA(0x10); 
            LCD_WR_DATA(0x7A); 
            LCD_WR_REG(0xCB);  
            LCD_WR_DATA(0x39); 
            LCD_WR_DATA(0x2C); 
            LCD_WR_DATA(0x00); 
            LCD_WR_DATA(0x34); 
            LCD_WR_DATA(0x02); 
            LCD_WR_REG(0xF7);  
            LCD_WR_DATA(0x20); 
            LCD_WR_REG(0xEA);  
            LCD_WR_DATA(0x00); 
            LCD_WR_DATA(0x00); 
            LCD_WR_REG(0xC0);    //Power control 
            LCD_WR_DATA(0x1B);   //VRH[5:0] 
            LCD_WR_REG(0xC1);    //Power control 
            LCD_WR_DATA(0x01);   //SAP[2:0];BT[3:0] 
            LCD_WR_REG(0xC5);    //VCM control 
            LCD_WR_DATA(0x30); 	 //3F
            LCD_WR_DATA(0x30); 	 //3C
            LCD_WR_REG(0xC7);    //VCM control2 
            LCD_WR_DATA(0XB7); 
            LCD_WR_REG(0x36);    // Memory Access Control 
            LCD_WR_DATA(0x48); 
            LCD_WR_REG(0x3A);   
            LCD_WR_DATA(0x55); 
            LCD_WR_REG(0xB1);   
            LCD_WR_DATA(0x00);   
            LCD_WR_DATA(0x1A); 
            LCD_WR_REG(0xB6);    // Display Function Control 
            LCD_WR_DATA(0x0A); 
            LCD_WR_DATA(0xA2); 
            LCD_WR_REG(0xF2);    // 3Gamma Function Disable 
            LCD_WR_DATA(0x00); 
            LCD_WR_REG(0x26);    //Gamma curve selected 
            LCD_WR_DATA(0x01); 
            LCD_WR_REG(0xE0);    //Set Gamma 
            LCD_WR_DATA(0x0F); 
            LCD_WR_DATA(0x2A); 
            LCD_WR_DATA(0x28); 
            LCD_WR_DATA(0x08); 
            LCD_WR_DATA(0x0E); 
            LCD_WR_DATA(0x08); 
            LCD_WR_DATA(0x54); 
            LCD_WR_DATA(0XA9); 
            LCD_WR_DATA(0x43); 
            LCD_WR_DATA(0x0A); 
            LCD_WR_DATA(0x0F); 
            LCD_WR_DATA(0x00); 
            LCD_WR_DATA(0x00); 
            LCD_WR_DATA(0x00); 
            LCD_WR_DATA(0x00); 		 
            LCD_WR_REG(0XE1);    //Set Gamma 
            LCD_WR_DATA(0x00); 
            LCD_WR_DATA(0x15); 
            LCD_WR_DATA(0x17); 
            LCD_WR_DATA(0x07); 
            LCD_WR_DATA(0x11); 
            LCD_WR_DATA(0x06); 
            LCD_WR_DATA(0x2B); 
            LCD_WR_DATA(0x56); 
            LCD_WR_DATA(0x3C); 
            LCD_WR_DATA(0x05); 
            LCD_WR_DATA(0x10); 
            LCD_WR_DATA(0x0F); 
            LCD_WR_DATA(0x3F); 
            LCD_WR_DATA(0x3F); 
            LCD_WR_DATA(0x0F); 
            LCD_WR_REG(0x2B); 
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x01);
            LCD_WR_DATA(0x3f);
            LCD_WR_REG(0x2A); 
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xef);	 
            LCD_WR_REG(0x11); //Exit Sleep
            HAL_Delay(120);
            LCD_WR_REG(0x29); //display on	
        }else if(lcddev.id==0x5310)
        { 
            LCD_WR_REG(0xED);
            LCD_WR_DATA(0x01);
            LCD_WR_DATA(0xFE);

            LCD_WR_REG(0xEE);
            LCD_WR_DATA(0xDE);
            LCD_WR_DATA(0x21);

            LCD_WR_REG(0xF1);
            LCD_WR_DATA(0x01);
            LCD_WR_REG(0xDF);
            LCD_WR_DATA(0x10);

            //VCOMvoltage//
            LCD_WR_REG(0xC4);
            LCD_WR_DATA(0x8F);	  //5f

            LCD_WR_REG(0xC6);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xE2);
            LCD_WR_DATA(0xE2);
            LCD_WR_DATA(0xE2);
            LCD_WR_REG(0xBF);
            LCD_WR_DATA(0xAA);

            LCD_WR_REG(0xB0);
            LCD_WR_DATA(0x0D);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x0D);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x11);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x19);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x21);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x2D);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x3D);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x5D);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x5D);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xB1);
            LCD_WR_DATA(0x80);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x8B);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x96);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xB2);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x02);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x03);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xB3);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xB4);
            LCD_WR_DATA(0x8B);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x96);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xA1);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xB5);
            LCD_WR_DATA(0x02);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x03);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x04);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xB6);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xB7);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x3F);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x5E);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x64);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x8C);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xAC);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xDC);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x70);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x90);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xEB);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xDC);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xB8);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xBA);
            LCD_WR_DATA(0x24);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xC1);
            LCD_WR_DATA(0x20);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x54);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xFF);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xC2);
            LCD_WR_DATA(0x0A);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x04);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xC3);
            LCD_WR_DATA(0x3C);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x3A);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x39);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x37);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x3C);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x36);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x32);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x2F);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x2C);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x29);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x26);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x24);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x24);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x23);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x3C);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x36);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x32);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x2F);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x2C);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x29);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x26);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x24);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x24);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x23);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xC4);
            LCD_WR_DATA(0x62);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x05);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x84);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xF0);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x18);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xA4);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x18);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x50);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x0C);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x17);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x95);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xF3);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xE6);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xC5);
            LCD_WR_DATA(0x32);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x44);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x65);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x76);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x88);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xC6);
            LCD_WR_DATA(0x20);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x17);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x01);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xC7);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xC8);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xC9);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xE0);
            LCD_WR_DATA(0x16);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x1C);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x21);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x36);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x46);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x52);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x64);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x7A);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x8B);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x99);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xA8);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xB9);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xC4);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xCA);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xD2);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xD9);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xE0);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xF3);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xE1);
            LCD_WR_DATA(0x16);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x1C);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x22);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x36);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x45);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x52);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x64);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x7A);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x8B);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x99);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xA8);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xB9);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xC4);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xCA);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xD2);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xD8);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xE0);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xF3);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xE2);
            LCD_WR_DATA(0x05);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x0B);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x1B);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x34);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x44);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x4F);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x61);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x79);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x88);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x97);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xA6);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xB7);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xC2);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xC7);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xD1);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xD6);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xDD);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xF3);
            LCD_WR_DATA(0x00);
            LCD_WR_REG(0xE3);
            LCD_WR_DATA(0x05);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xA);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x1C);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x33);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x44);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x50);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x62);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x78);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x88);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x97);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xA6);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xB7);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xC2);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xC7);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xD1);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xD5);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xDD);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xF3);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xE4);
            LCD_WR_DATA(0x01);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x01);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x02);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x2A);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x3C);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x4B);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x5D);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x74);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x84);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x93);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xA2);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xB3);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xBE);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xC4);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xCD);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xD3);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xDD);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xF3);
            LCD_WR_DATA(0x00);
            LCD_WR_REG(0xE5);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x02);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x29);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x3C);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x4B);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x5D);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x74);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x84);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x93);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xA2);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xB3);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xBE);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xC4);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xCD);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xD3);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xDC);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xF3);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xE6);
            LCD_WR_DATA(0x11);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x34);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x56);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x76);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x77);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x66);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x88);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x99);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xBB);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x99);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x66);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x55);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x55);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x45);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x43);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x44);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xE7);
            LCD_WR_DATA(0x32);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x55);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x76);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x66);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x67);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x67);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x87);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x99);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xBB);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x99);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x77);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x44);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x56);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x23); 
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x33);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x45);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xE8);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x99);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x87);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x88);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x77);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x66);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x88);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xAA);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0xBB);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x99);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x66);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x55);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x55);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x44);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x44);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x55);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xE9);
            LCD_WR_DATA(0xAA);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0x00);
            LCD_WR_DATA(0xAA);

            LCD_WR_REG(0xCF);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xF0);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x50);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xF3);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0xF9);
            LCD_WR_DATA(0x06);
            LCD_WR_DATA(0x10);
            LCD_WR_DATA(0x29);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0x3A);
            LCD_WR_DATA(0x55);	//66

            LCD_WR_REG(0x11);
            HAL_Delay(100);
            LCD_WR_REG(0x29);
            LCD_WR_REG(0x35);
            LCD_WR_DATA(0x00);

            LCD_WR_REG(0x51);
            LCD_WR_DATA(0xFF);
            LCD_WR_REG(0x53);
            LCD_WR_DATA(0x2C);
            LCD_WR_REG(0x55);
            LCD_WR_DATA(0x82);
            LCD_WR_REG(0x2c);
        }else if(lcddev.id==0x5510)
        {
            LCD_WriteReg(0xF000,0x55);
            LCD_WriteReg(0xF001,0xAA);
            LCD_WriteReg(0xF002,0x52);
            LCD_WriteReg(0xF003,0x08);
            LCD_WriteReg(0xF004,0x01);
            //AVDD Set AVDD 5.2V
            LCD_WriteReg(0xB000,0x0D);
            LCD_WriteReg(0xB001,0x0D);
            LCD_WriteReg(0xB002,0x0D);
            //AVDD ratio
            LCD_WriteReg(0xB600,0x34);
            LCD_WriteReg(0xB601,0x34);
            LCD_WriteReg(0xB602,0x34);
            //AVEE -5.2V
            LCD_WriteReg(0xB100,0x0D);
            LCD_WriteReg(0xB101,0x0D);
            LCD_WriteReg(0xB102,0x0D);
            //AVEE ratio
            LCD_WriteReg(0xB700,0x34);
            LCD_WriteReg(0xB701,0x34);
            LCD_WriteReg(0xB702,0x34);
            //VCL -2.5V
            LCD_WriteReg(0xB200,0x00);
            LCD_WriteReg(0xB201,0x00);
            LCD_WriteReg(0xB202,0x00);
            //VCL ratio
            LCD_WriteReg(0xB800,0x24);
            LCD_WriteReg(0xB801,0x24);
            LCD_WriteReg(0xB802,0x24);
            //VGH 15V (Free pump)
            LCD_WriteReg(0xBF00,0x01);
            LCD_WriteReg(0xB300,0x0F);
            LCD_WriteReg(0xB301,0x0F);
            LCD_WriteReg(0xB302,0x0F);
            //VGH ratio
            LCD_WriteReg(0xB900,0x34);
            LCD_WriteReg(0xB901,0x34);
            LCD_WriteReg(0xB902,0x34);
            //VGL_REG -10V
            LCD_WriteReg(0xB500,0x08);
            LCD_WriteReg(0xB501,0x08);
            LCD_WriteReg(0xB502,0x08);
            LCD_WriteReg(0xC200,0x03);
            //VGLX ratio
            LCD_WriteReg(0xBA00,0x24);
            LCD_WriteReg(0xBA01,0x24);
            LCD_WriteReg(0xBA02,0x24);
            //VGMP/VGSP 4.5V/0V
            LCD_WriteReg(0xBC00,0x00);
            LCD_WriteReg(0xBC01,0x78);
            LCD_WriteReg(0xBC02,0x00);
            //VGMN/VGSN -4.5V/0V
            LCD_WriteReg(0xBD00,0x00);
            LCD_WriteReg(0xBD01,0x78);
            LCD_WriteReg(0xBD02,0x00);
            //VCOM
            LCD_WriteReg(0xBE00,0x00);
            LCD_WriteReg(0xBE01,0x64);
            //Gamma Setting
            LCD_WriteReg(0xD100,0x00);
            LCD_WriteReg(0xD101,0x33);
            LCD_WriteReg(0xD102,0x00);
            LCD_WriteReg(0xD103,0x34);
            LCD_WriteReg(0xD104,0x00);
            LCD_WriteReg(0xD105,0x3A);
            LCD_WriteReg(0xD106,0x00);
            LCD_WriteReg(0xD107,0x4A);
            LCD_WriteReg(0xD108,0x00);
            LCD_WriteReg(0xD109,0x5C);
            LCD_WriteReg(0xD10A,0x00);
            LCD_WriteReg(0xD10B,0x81);
            LCD_WriteReg(0xD10C,0x00);
            LCD_WriteReg(0xD10D,0xA6);
            LCD_WriteReg(0xD10E,0x00);
            LCD_WriteReg(0xD10F,0xE5);
            LCD_WriteReg(0xD110,0x01);
            LCD_WriteReg(0xD111,0x13);
            LCD_WriteReg(0xD112,0x01);
            LCD_WriteReg(0xD113,0x54);
            LCD_WriteReg(0xD114,0x01);
            LCD_WriteReg(0xD115,0x82);
            LCD_WriteReg(0xD116,0x01);
            LCD_WriteReg(0xD117,0xCA);
            LCD_WriteReg(0xD118,0x02);
            LCD_WriteReg(0xD119,0x00);
            LCD_WriteReg(0xD11A,0x02);
            LCD_WriteReg(0xD11B,0x01);
            LCD_WriteReg(0xD11C,0x02);
            LCD_WriteReg(0xD11D,0x34);
            LCD_WriteReg(0xD11E,0x02);
            LCD_WriteReg(0xD11F,0x67);
            LCD_WriteReg(0xD120,0x02);
            LCD_WriteReg(0xD121,0x84);
            LCD_WriteReg(0xD122,0x02);
            LCD_WriteReg(0xD123,0xA4);
            LCD_WriteReg(0xD124,0x02);
            LCD_WriteReg(0xD125,0xB7);
            LCD_WriteReg(0xD126,0x02);
            LCD_WriteReg(0xD127,0xCF);
            LCD_WriteReg(0xD128,0x02);
            LCD_WriteReg(0xD129,0xDE);
            LCD_WriteReg(0xD12A,0x02);
            LCD_WriteReg(0xD12B,0xF2);
            LCD_WriteReg(0xD12C,0x02);
            LCD_WriteReg(0xD12D,0xFE);
            LCD_WriteReg(0xD12E,0x03);
            LCD_WriteReg(0xD12F,0x10);
            LCD_WriteReg(0xD130,0x03);
            LCD_WriteReg(0xD131,0x33);
            LCD_WriteReg(0xD132,0x03);
            LCD_WriteReg(0xD133,0x6D);
            LCD_WriteReg(0xD200,0x00);
            LCD_WriteReg(0xD201,0x33);
            LCD_WriteReg(0xD202,0x00);
            LCD_WriteReg(0xD203,0x34);
            LCD_WriteReg(0xD204,0x00);
            LCD_WriteReg(0xD205,0x3A);
            LCD_WriteReg(0xD206,0x00);
            LCD_WriteReg(0xD207,0x4A);
            LCD_WriteReg(0xD208,0x00);
            LCD_WriteReg(0xD209,0x5C);
            LCD_WriteReg(0xD20A,0x00);

            LCD_WriteReg(0xD20B,0x81);
            LCD_WriteReg(0xD20C,0x00);
            LCD_WriteReg(0xD20D,0xA6);
            LCD_WriteReg(0xD20E,0x00);
            LCD_WriteReg(0xD20F,0xE5);
            LCD_WriteReg(0xD210,0x01);
            LCD_WriteReg(0xD211,0x13);
            LCD_WriteReg(0xD212,0x01);
            LCD_WriteReg(0xD213,0x54);
            LCD_WriteReg(0xD214,0x01);
            LCD_WriteReg(0xD215,0x82);
            LCD_WriteReg(0xD216,0x01);
            LCD_WriteReg(0xD217,0xCA);
            LCD_WriteReg(0xD218,0x02);
            LCD_WriteReg(0xD219,0x00);
            LCD_WriteReg(0xD21A,0x02);
            LCD_WriteReg(0xD21B,0x01);
            LCD_WriteReg(0xD21C,0x02);
            LCD_WriteReg(0xD21D,0x34);
            LCD_WriteReg(0xD21E,0x02);
            LCD_WriteReg(0xD21F,0x67);
            LCD_WriteReg(0xD220,0x02);
            LCD_WriteReg(0xD221,0x84);
            LCD_WriteReg(0xD222,0x02);
            LCD_WriteReg(0xD223,0xA4);
            LCD_WriteReg(0xD224,0x02);
            LCD_WriteReg(0xD225,0xB7);
            LCD_WriteReg(0xD226,0x02);
            LCD_WriteReg(0xD227,0xCF);
            LCD_WriteReg(0xD228,0x02);
            LCD_WriteReg(0xD229,0xDE);
            LCD_WriteReg(0xD22A,0x02);
            LCD_WriteReg(0xD22B,0xF2);
            LCD_WriteReg(0xD22C,0x02);
            LCD_WriteReg(0xD22D,0xFE);
            LCD_WriteReg(0xD22E,0x03);
            LCD_WriteReg(0xD22F,0x10);
            LCD_WriteReg(0xD230,0x03);
            LCD_WriteReg(0xD231,0x33);
            LCD_WriteReg(0xD232,0x03);
            LCD_WriteReg(0xD233,0x6D);
            LCD_WriteReg(0xD300,0x00);
            LCD_WriteReg(0xD301,0x33);
            LCD_WriteReg(0xD302,0x00);
            LCD_WriteReg(0xD303,0x34);
            LCD_WriteReg(0xD304,0x00);
            LCD_WriteReg(0xD305,0x3A);
            LCD_WriteReg(0xD306,0x00);
            LCD_WriteReg(0xD307,0x4A);
            LCD_WriteReg(0xD308,0x00);
            LCD_WriteReg(0xD309,0x5C);
            LCD_WriteReg(0xD30A,0x00);

            LCD_WriteReg(0xD30B,0x81);
            LCD_WriteReg(0xD30C,0x00);
            LCD_WriteReg(0xD30D,0xA6);
            LCD_WriteReg(0xD30E,0x00);
            LCD_WriteReg(0xD30F,0xE5);
            LCD_WriteReg(0xD310,0x01);
            LCD_WriteReg(0xD311,0x13);
            LCD_WriteReg(0xD312,0x01);
            LCD_WriteReg(0xD313,0x54);
            LCD_WriteReg(0xD314,0x01);
            LCD_WriteReg(0xD315,0x82);
            LCD_WriteReg(0xD316,0x01);
            LCD_WriteReg(0xD317,0xCA);
            LCD_WriteReg(0xD318,0x02);
            LCD_WriteReg(0xD319,0x00);
            LCD_WriteReg(0xD31A,0x02);
            LCD_WriteReg(0xD31B,0x01);
            LCD_WriteReg(0xD31C,0x02);
            LCD_WriteReg(0xD31D,0x34);
            LCD_WriteReg(0xD31E,0x02);
            LCD_WriteReg(0xD31F,0x67);
            LCD_WriteReg(0xD320,0x02);
            LCD_WriteReg(0xD321,0x84);
            LCD_WriteReg(0xD322,0x02);
            LCD_WriteReg(0xD323,0xA4);
            LCD_WriteReg(0xD324,0x02);
            LCD_WriteReg(0xD325,0xB7);
            LCD_WriteReg(0xD326,0x02);
            LCD_WriteReg(0xD327,0xCF);
            LCD_WriteReg(0xD328,0x02);
            LCD_WriteReg(0xD329,0xDE);
            LCD_WriteReg(0xD32A,0x02);
            LCD_WriteReg(0xD32B,0xF2);
            LCD_WriteReg(0xD32C,0x02);
            LCD_WriteReg(0xD32D,0xFE);
            LCD_WriteReg(0xD32E,0x03);
            LCD_WriteReg(0xD32F,0x10);
            LCD_WriteReg(0xD330,0x03);
            LCD_WriteReg(0xD331,0x33);
            LCD_WriteReg(0xD332,0x03);
            LCD_WriteReg(0xD333,0x6D);
            LCD_WriteReg(0xD400,0x00);
            LCD_WriteReg(0xD401,0x33);
            LCD_WriteReg(0xD402,0x00);
            LCD_WriteReg(0xD403,0x34);
            LCD_WriteReg(0xD404,0x00);
            LCD_WriteReg(0xD405,0x3A);
            LCD_WriteReg(0xD406,0x00);
            LCD_WriteReg(0xD407,0x4A);
            LCD_WriteReg(0xD408,0x00);
            LCD_WriteReg(0xD409,0x5C);
            LCD_WriteReg(0xD40A,0x00);
            LCD_WriteReg(0xD40B,0x81);

            LCD_WriteReg(0xD40C,0x00);
            LCD_WriteReg(0xD40D,0xA6);
            LCD_WriteReg(0xD40E,0x00);
            LCD_WriteReg(0xD40F,0xE5);
            LCD_WriteReg(0xD410,0x01);
            LCD_WriteReg(0xD411,0x13);
            LCD_WriteReg(0xD412,0x01);
            LCD_WriteReg(0xD413,0x54);
            LCD_WriteReg(0xD414,0x01);
            LCD_WriteReg(0xD415,0x82);
            LCD_WriteReg(0xD416,0x01);
            LCD_WriteReg(0xD417,0xCA);
            LCD_WriteReg(0xD418,0x02);
            LCD_WriteReg(0xD419,0x00);
            LCD_WriteReg(0xD41A,0x02);
            LCD_WriteReg(0xD41B,0x01);
            LCD_WriteReg(0xD41C,0x02);
            LCD_WriteReg(0xD41D,0x34);
            LCD_WriteReg(0xD41E,0x02);
            LCD_WriteReg(0xD41F,0x67);
            LCD_WriteReg(0xD420,0x02);
            LCD_WriteReg(0xD421,0x84);
            LCD_WriteReg(0xD422,0x02);
            LCD_WriteReg(0xD423,0xA4);
            LCD_WriteReg(0xD424,0x02);
            LCD_WriteReg(0xD425,0xB7);
            LCD_WriteReg(0xD426,0x02);
            LCD_WriteReg(0xD427,0xCF);
            LCD_WriteReg(0xD428,0x02);
            LCD_WriteReg(0xD429,0xDE);
            LCD_WriteReg(0xD42A,0x02);
            LCD_WriteReg(0xD42B,0xF2);
            LCD_WriteReg(0xD42C,0x02);
            LCD_WriteReg(0xD42D,0xFE);
            LCD_WriteReg(0xD42E,0x03);
            LCD_WriteReg(0xD42F,0x10);
            LCD_WriteReg(0xD430,0x03);
            LCD_WriteReg(0xD431,0x33);
            LCD_WriteReg(0xD432,0x03);
            LCD_WriteReg(0xD433,0x6D);
            LCD_WriteReg(0xD500,0x00);
            LCD_WriteReg(0xD501,0x33);
            LCD_WriteReg(0xD502,0x00);
            LCD_WriteReg(0xD503,0x34);
            LCD_WriteReg(0xD504,0x00);
            LCD_WriteReg(0xD505,0x3A);
            LCD_WriteReg(0xD506,0x00);
            LCD_WriteReg(0xD507,0x4A);
            LCD_WriteReg(0xD508,0x00);
            LCD_WriteReg(0xD509,0x5C);
            LCD_WriteReg(0xD50A,0x00);
            LCD_WriteReg(0xD50B,0x81);

            LCD_WriteReg(0xD50C,0x00);
            LCD_WriteReg(0xD50D,0xA6);
            LCD_WriteReg(0xD50E,0x00);
            LCD_WriteReg(0xD50F,0xE5);
            LCD_WriteReg(0xD510,0x01);
            LCD_WriteReg(0xD511,0x13);
            LCD_WriteReg(0xD512,0x01);
            LCD_WriteReg(0xD513,0x54);
            LCD_WriteReg(0xD514,0x01);
            LCD_WriteReg(0xD515,0x82);
            LCD_WriteReg(0xD516,0x01);
            LCD_WriteReg(0xD517,0xCA);
            LCD_WriteReg(0xD518,0x02);
            LCD_WriteReg(0xD519,0x00);
            LCD_WriteReg(0xD51A,0x02);
            LCD_WriteReg(0xD51B,0x01);
            LCD_WriteReg(0xD51C,0x02);
            LCD_WriteReg(0xD51D,0x34);
            LCD_WriteReg(0xD51E,0x02);
            LCD_WriteReg(0xD51F,0x67);
            LCD_WriteReg(0xD520,0x02);
            LCD_WriteReg(0xD521,0x84);
            LCD_WriteReg(0xD522,0x02);
            LCD_WriteReg(0xD523,0xA4);
            LCD_WriteReg(0xD524,0x02);
            LCD_WriteReg(0xD525,0xB7);
            LCD_WriteReg(0xD526,0x02);
            LCD_WriteReg(0xD527,0xCF);
            LCD_WriteReg(0xD528,0x02);
            LCD_WriteReg(0xD529,0xDE);
            LCD_WriteReg(0xD52A,0x02);
            LCD_WriteReg(0xD52B,0xF2);
            LCD_WriteReg(0xD52C,0x02);
            LCD_WriteReg(0xD52D,0xFE);
            LCD_WriteReg(0xD52E,0x03);
            LCD_WriteReg(0xD52F,0x10);
            LCD_WriteReg(0xD530,0x03);
            LCD_WriteReg(0xD531,0x33);
            LCD_WriteReg(0xD532,0x03);
            LCD_WriteReg(0xD533,0x6D);
            LCD_WriteReg(0xD600,0x00);
            LCD_WriteReg(0xD601,0x33);
            LCD_WriteReg(0xD602,0x00);
            LCD_WriteReg(0xD603,0x34);
            LCD_WriteReg(0xD604,0x00);
            LCD_WriteReg(0xD605,0x3A);
            LCD_WriteReg(0xD606,0x00);
            LCD_WriteReg(0xD607,0x4A);
            LCD_WriteReg(0xD608,0x00);
            LCD_WriteReg(0xD609,0x5C);
            LCD_WriteReg(0xD60A,0x00);
            LCD_WriteReg(0xD60B,0x81);

            LCD_WriteReg(0xD60C,0x00);
            LCD_WriteReg(0xD60D,0xA6);
            LCD_WriteReg(0xD60E,0x00);
            LCD_WriteReg(0xD60F,0xE5);
            LCD_WriteReg(0xD610,0x01);
            LCD_WriteReg(0xD611,0x13);
            LCD_WriteReg(0xD612,0x01);
            LCD_WriteReg(0xD613,0x54);
            LCD_WriteReg(0xD614,0x01);
            LCD_WriteReg(0xD615,0x82);
            LCD_WriteReg(0xD616,0x01);
            LCD_WriteReg(0xD617,0xCA);
            LCD_WriteReg(0xD618,0x02);
            LCD_WriteReg(0xD619,0x00);
            LCD_WriteReg(0xD61A,0x02);
            LCD_WriteReg(0xD61B,0x01);
            LCD_WriteReg(0xD61C,0x02);
            LCD_WriteReg(0xD61D,0x34);
            LCD_WriteReg(0xD61E,0x02);
            LCD_WriteReg(0xD61F,0x67);
            LCD_WriteReg(0xD620,0x02);
            LCD_WriteReg(0xD621,0x84);
            LCD_WriteReg(0xD622,0x02);
            LCD_WriteReg(0xD623,0xA4);
            LCD_WriteReg(0xD624,0x02);
            LCD_WriteReg(0xD625,0xB7);
            LCD_WriteReg(0xD626,0x02);
            LCD_WriteReg(0xD627,0xCF);
            LCD_WriteReg(0xD628,0x02);
            LCD_WriteReg(0xD629,0xDE);
            LCD_WriteReg(0xD62A,0x02);
            LCD_WriteReg(0xD62B,0xF2);
            LCD_WriteReg(0xD62C,0x02);
            LCD_WriteReg(0xD62D,0xFE);
            LCD_WriteReg(0xD62E,0x03);
            LCD_WriteReg(0xD62F,0x10);
            LCD_WriteReg(0xD630,0x03);
            LCD_WriteReg(0xD631,0x33);
            LCD_WriteReg(0xD632,0x03);
            LCD_WriteReg(0xD633,0x6D);
            //LV2 Page 0 enable
            LCD_WriteReg(0xF000,0x55);
            LCD_WriteReg(0xF001,0xAA);
            LCD_WriteReg(0xF002,0x52);
            LCD_WriteReg(0xF003,0x08);
            LCD_WriteReg(0xF004,0x00);
            //Display control
            LCD_WriteReg(0xB100, 0xCC);
            LCD_WriteReg(0xB101, 0x00);
            //Source hold time
            LCD_WriteReg(0xB600,0x05);
            //Gate EQ control
            LCD_WriteReg(0xB700,0x70);
            LCD_WriteReg(0xB701,0x70);
            //Source EQ control (Mode 2)
            LCD_WriteReg(0xB800,0x01);
            LCD_WriteReg(0xB801,0x03);
            LCD_WriteReg(0xB802,0x03);
            LCD_WriteReg(0xB803,0x03);
            //Inversion mode (2-dot)
            LCD_WriteReg(0xBC00,0x02);
            LCD_WriteReg(0xBC01,0x00);
            LCD_WriteReg(0xBC02,0x00);
            //Timing control 4H w/ 4-delay
            LCD_WriteReg(0xC900,0xD0);
            LCD_WriteReg(0xC901,0x02);
            LCD_WriteReg(0xC902,0x50);
            LCD_WriteReg(0xC903,0x50);
            LCD_WriteReg(0xC904,0x50);
            LCD_WriteReg(0x3500,0x00);
            LCD_WriteReg(0x3A00,0x55);  //16-bit/pixel
            LCD_WR_REG(0x1100);
            HAL_Delay(1);
            LCD_WR_REG(0x2900);  
        }else if(lcddev.id==0X1963)
        {
            LCD_WR_REG(0xE2);		//Set PLL with OSC = 10MHz (hardware),	Multiplier N = 35, 250MHz < VCO < 800MHz = OSC*(N+1), VCO = 300MHz
            LCD_WR_DATA(0x1D);		//param1 
            LCD_WR_DATA(0x02);		//param2 Divider M = 2, PLL = 300/(M+1) = 100MHz
            LCD_WR_DATA(0x04);		//param3 Validate M and N values   
            HAL_Delay(1);
            LCD_WR_REG(0xE0);		// Start PLL command
            LCD_WR_DATA(0x01);		// enable PLL
            HAL_Delay(10);
            LCD_WR_REG(0xE0);		// Start PLL command again
            LCD_WR_DATA(0x03);		// now, use PLL output as system clock	
            HAL_Delay(12);  
            LCD_WR_REG(0x01);		//软复位
            HAL_Delay(10);
            
            LCD_WR_REG(0xE6);		//设置像素频率,33Mhz
            LCD_WR_DATA(0x2F);
            LCD_WR_DATA(0xFF);
            LCD_WR_DATA(0xFF);
        
            LCD_WR_REG(0xB0);		//设置LCD模式
            LCD_WR_DATA(0x20);		//24为模式
            LCD_WR_DATA(0x00);		//TFT 模式
        
            LCD_WR_DATA((SSD_HOR_RESOLUTION-1)>>8);//设置LCD水平像素
            LCD_WR_DATA(SSD_HOR_RESOLUTION-1);		 
            LCD_WR_DATA((SSD_VER_RESOLUTION-1)>>8);//设置LCD垂直像素
            LCD_WR_DATA(SSD_VER_RESOLUTION-1);		 
            LCD_WR_DATA(0x00);		//RGB序列
            
            LCD_WR_REG(0xB4);		//Set horizontal period
            LCD_WR_DATA((SSD_HT-1)>>8);
            LCD_WR_DATA(SSD_HT-1);
            LCD_WR_DATA(SSD_HPS>>8);
            LCD_WR_DATA(SSD_HPS);
            LCD_WR_DATA(SSD_HOR_PULSE_WIDTH-1);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            LCD_WR_REG(0xB6);		//Set vertical period
            LCD_WR_DATA((SSD_VT-1)>>8);
            LCD_WR_DATA(SSD_VT-1);
            LCD_WR_DATA(SSD_VPS>>8);
            LCD_WR_DATA(SSD_VPS);
            LCD_WR_DATA(SSD_VER_FRONT_PORCH-1);
            LCD_WR_DATA(0x00);
            LCD_WR_DATA(0x00);
            
            LCD_WR_REG(0xF0);	//设置SSD1963与CPU接口为16bit  
            LCD_WR_DATA(0x03);	//16-bit(565 format) data for 16bpp 

            LCD_WR_REG(0x29);	//开启显示
            //设置PWM输出，背光通过占空比可调
            LCD_WR_REG(0xD0);	//设置自动白平衡DBC
            LCD_WR_DATA(0x00);	//disable
        
            LCD_WR_REG(0xBE);	//配置PWM输出
            LCD_WR_DATA(0x05);	//1设置PWM频率
            LCD_WR_DATA(0xFE);	//2设置PWM占空比
            LCD_WR_DATA(0x01);	//3设置C
            LCD_WR_DATA(0x00);	//4设置D
            LCD_WR_DATA(0x00);	//5设置E 
            LCD_WR_DATA(0x00);	//6设置F 
            
            LCD_WR_REG(0xB8);	//设置GPIO
            LCD_WR_DATA(0x03);	//2个IO设置为输出
            LCD_WR_DATA(0x01);	//GPIO使用正常的IO功能
            LCD_WR_REG(0xBA);
            LCD_WR_DATA(0X01);	//GPIO[1:0]=01,控制LCD方向
            
            LCD_SSD_BackLightSet(100);//背光设置为最亮
        }	    
    }	
    //初始化完成后，提速
    if(lcddev.id==0X9341||lcddev.id==0X5310||lcddev.id==0X5510||lcddev.id==0X1963)//如果是这几个IC,设置WR时序为最快
    {
        FSMC_WriteTim.AddressSetupTime=5;      
        FSMC_WriteTim.DataSetupTime=5;         
        FMC_NORSRAM_Extended_Timing_Init(SRAM_Handler.Extended,&FSMC_WriteTim,SRAM_Handler.Init.NSBank,SRAM_Handler.Init.ExtendedMode);
    } 
    printf("----LCD ID:0x%x\r\n", lcddev.id);
    LCD_Display_Dir(1);		//默认横屏
    LCD_LED(1);				//点亮背光
    LCD_Clear(WHITE);
}  

void LCD_Clear(uint32_t color)
{
    uint32_t index=0;      
    uint32_t totalpoint=lcddev.width;
    if(lcdltdc.pwidth!=0)	//RGB屏
    {
        LTDC_Clear(color);
    }else
    {
        totalpoint*=lcddev.height; 	//得到总点数
        LCD_SetCursor(0x00,0x0000);	//设置光标位置
        LCD_WriteRAM_Prepare();     //开始写GRAM	 	  
        for(index=0;index<totalpoint;index++)
        {
            LCD->LCD_RAM=color;	    // 显示颜色
        }
    }
}  

// 指定区域填充单个颜色
void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint32_t color)
{          
    uint16_t i,j;
    uint16_t xlen=0;
    if(lcdltdc.pwidth!=0)	//RGB
    {
        LTDC_Fill(sx,sy,ex,ey,color);
    }else
    {	
        xlen=ex-sx+1;	 
        for(i=sy;i<=ey;i++)
        {
            LCD_SetCursor(sx,i);
            LCD_WriteRAM_Prepare();
            for(j=0;j<xlen;j++)
            {
                LCD->LCD_RAM=color;
            }
        } 
    }
}  
void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color)
{  
    uint16_t height,width;
    uint16_t i,j;
    if(lcdltdc.pwidth!=0)	//RGB
    {
        LTDC_Color_Fill(sx,sy,ex,ey,color);
    }else
    {
        width=ex-sx+1;
        height=ey-sy+1;
        for(i=0;i<height;i++)
        {
            LCD_SetCursor(sx,sy+i);
            LCD_WriteRAM_Prepare();
            for(j=0;j<width;j++)
            {
                LCD->LCD_RAM=color[i*width+j]; 
            }
        }
    }	
}  

void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint16_t t; 
    int xerr=0,yerr=0,delta_x,delta_y,distance; 
    int incx,incy,uRow,uCol; 
    delta_x=x2-x1;  // 计算坐标增量
    delta_y=y2-y1; 
    uRow=x1; 
    uCol=y1; 
    if(delta_x>0)   // 设置单步方向
        incx=1;
    else if(delta_x==0)
        incx=0;     //垂直线 
    else 
    {
        incx=-1;
        delta_x = -delta_x;
    } 
    if(delta_y>0)
        incy=1; 
    else if(delta_y==0)
        incy=0;     //水平线
    else
    {
        incy=-1;
        delta_y = -delta_y;
    }

    if( delta_x>delta_y)
        distance=delta_x; //选取基本增量坐标轴
    else 
        distance=delta_y; 

    for(t=0;t<=distance+1;t++ )//画线输出
    {  
        LCD_DrawPoint(uRow,uCol);//画点
        xerr+=delta_x ; 
        yerr+=delta_y ; 
        if(xerr>distance) 
        { 
            xerr-=distance; 
            uRow+=incx; 
        } 
        if(yerr>distance) 
        { 
            yerr-=distance; 
            uCol+=incy; 
        } 
    }  
}    
//画矩形	  
//(x1,y1),(x2,y2):矩形的对角坐标
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    LCD_DrawLine(x1,y1,x2,y1);
    LCD_DrawLine(x1,y1,x1,y2);
    LCD_DrawLine(x1,y2,x2,y2);
    LCD_DrawLine(x2,y1,x2,y2);
}

// 圆心：(x0,y0), 半径：r
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r)
{
    int a,b;
    int di;
    a=0;b=r;	  
    di=3-(r<<1);
    while(a<=b)
    {
        LCD_DrawPoint(x0+a,y0-b);             //5
        LCD_DrawPoint(x0+b,y0-a);             //0           
        LCD_DrawPoint(x0+b,y0+a);             //4               
        LCD_DrawPoint(x0+a,y0+b);             //6 
        LCD_DrawPoint(x0-a,y0+b);             //1       
        LCD_DrawPoint(x0-b,y0+a);             
        LCD_DrawPoint(x0-a,y0-b);             //2             
        LCD_DrawPoint(x0-b,y0-a);             //7     	         
        a++;
        //使用Bresenham算法画圆
        if(di<0)di +=4*a+6;	  
        else
        {
            di+=10+4*(a-b);   
            b--;
        } 						    
    }
} 									  

//x,y:坐标
//num:要显示大的字符，ASCII码
//size:字体 12/16/24/32
//mode:叠加方式(1)，非叠加方式(0)
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode)
{  							  
    uint8_t temp,t1,t;
    uint16_t y0=y;
    uint8_t csize=(size/8+((size%8)?1:0))*(size/2);
    num=num-' ';
    for(t=0;t<csize;t++)
    {   
        if(size==12)temp=asc2_1206[num][t];
        else if(size==16)temp=asc2_1608[num][t];
        else if(size==24)temp=asc2_2412[num][t];
        else if(size==32)temp=asc2_3216[num][t];
        else return;
        for(t1=0;t1<8;t1++)
        {			    
            if(temp&0x80)LCD_Fast_DrawPoint(x,y,POINT_COLOR);
            else if(mode==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR);
            temp<<=1;
            y++;
            if(y>=lcddev.height)return;
            if((y-y0)==size)
            {
                y=y0;
                x++;
                if(x>=lcddev.width)return;
                break;
            }
        }  	 
    }  	    	   	 	  
}   

uint32_t LCD_Pow(uint8_t m,uint8_t n)
{
    uint32_t result=1;	 
    while(n--)result*=m;    
    return result;
}			 

void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size)
{         	
    uint8_t t,temp;
    uint8_t enshow=0;						   
    for(t=0;t<len;t++)
    {
        temp=(num/LCD_Pow(10,len-t-1))%10;
        if(enshow==0&&t<(len-1))
        {
            if(temp==0)
            {
                LCD_ShowChar(x+(size/2)*t,y,' ',size,0);
                continue;
            }else enshow=1; 
                
        }
        LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,0); 
    }
} 

void LCD_ShowxNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode)
{  
    uint8_t t,temp;
    uint8_t enshow=0;						   
    for(t=0;t<len;t++)
    {
        temp=(num/LCD_Pow(10,len-t-1))%10;
        if(enshow==0&&t<(len-1))
        {
            if(temp==0)
            {
                if(mode&0X80)LCD_ShowChar(x+(size/2)*t,y,'0',size,mode&0X01);  
                else LCD_ShowChar(x+(size/2)*t,y,' ',size,mode&0X01);  
                continue;
            }else enshow=1; 
                
        }
        LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,mode&0X01); 
    }
} 

void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,uint8_t *p)
{         
    uint8_t x0=x;
    width+=x;
    height+=y;
    while((*p<='~')&&(*p>=' '))
    {       
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;
        LCD_ShowChar(x,y,*p,size,0);
        x+=size/2;
        p++;
    }  
}

