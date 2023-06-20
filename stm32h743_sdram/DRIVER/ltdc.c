#include "ltdc.h"
#include "lcd.h"

LTDC_HandleTypeDef  LTDC_Handler;
DMA2D_HandleTypeDef DMA2D_Handler;

//根据不同的颜色格式定义帧缓存数组
#if LCD_PIXFORMAT==LCD_PIXFORMAT_ARGB8888||LCD_PIXFORMAT==LCD_PIXFORMAT_RGB888
uint32_t ltdc_lcd_framebuf[1280][800] __attribute__((at(LCD_FRAME_BUF_ADDR)));
#else
//定义最大分辨率时， LCD所需要的帧缓存数组大小 
uint16_t ltdc_lcd_framebuf[1280][800] __attribute__((section(".bss.EXRAM1")));
#endif

uint32_t *ltdc_framebuf[2];	//LTDC LCD֡帧缓存数组指针，必须指向对应大小的内存区域
_ltdc_dev lcdltdc;

//lcd_switch:1 打开,0 关闭
void LTDC_Switch(uint8_t sw)
{
    if(sw==1) 
        __HAL_LTDC_ENABLE(&LTDC_Handler);
    else if(sw==0)
        __HAL_LTDC_DISABLE(&LTDC_Handler);
}

void LTDC_Layer_Switch(uint8_t layerx,uint8_t sw)
{
    if(sw==1)
        __HAL_LTDC_LAYER_ENABLE(&LTDC_Handler,layerx);
    else if(sw==0)
        __HAL_LTDC_LAYER_DISABLE(&LTDC_Handler,layerx);

    __HAL_LTDC_RELOAD_CONFIG(&LTDC_Handler);
}

void LTDC_Select_Layer(uint8_t layerx)
{
    lcdltdc.activelayer=layerx;
}

void LTDC_Display_Dir(uint8_t dir)
{
    lcdltdc.dir=dir;
    if(dir==0)			//竖屏
    {
        lcdltdc.width=lcdltdc.pheight;
        lcdltdc.height=lcdltdc.pwidth;	
    }else if(dir==1)	//横屏
    {
        lcdltdc.width=lcdltdc.pwidth;
        lcdltdc.height=lcdltdc.pheight;
    }
}

void LTDC_Draw_Point(uint16_t x,uint16_t y,uint32_t color)
{ 
#if LCD_PIXFORMAT==LCD_PIXFORMAT_ARGB8888||LCD_PIXFORMAT==LCD_PIXFORMAT_RGB888
    if(lcdltdc.dir)
    {
        *(uint32_t*)((uint32_t)ltdc_framebuf[lcdltdc.activelayer]+lcdltdc.pixsize*(lcdltdc.pwidth*y+x))=color;
    }else
    {
        *(uint32_t*)((uint32_t)ltdc_framebuf[lcdltdc.activelayer]+lcdltdc.pixsize*(lcdltdc.pwidth*(lcdltdc.pheight-x-1)+y))=color; 
    }
#else
    if(lcdltdc.dir)
    {
        *(uint16_t*)((uint32_t)ltdc_framebuf[lcdltdc.activelayer]+lcdltdc.pixsize*(lcdltdc.pwidth*y+x))=color;
    }else
    {
        *(uint16_t*)((uint32_t)ltdc_framebuf[lcdltdc.activelayer]+lcdltdc.pixsize*(lcdltdc.pwidth*(lcdltdc.pheight-x-1)+y))=color; 
    }
#endif
}

uint32_t LTDC_Read_Point(uint16_t x,uint16_t y)
{ 
#if LCD_PIXFORMAT==LCD_PIXFORMAT_ARGB8888||LCD_PIXFORMAT==LCD_PIXFORMAT_RGB888
    if(lcdltdc.dir)
    {
        return *(uint32_t*)((uint32_t)ltdc_framebuf[lcdltdc.activelayer]+lcdltdc.pixsize*(lcdltdc.pwidth*y+x));
    }else
    {
        return *(uint32_t*)((uint32_t)ltdc_framebuf[lcdltdc.activelayer]+lcdltdc.pixsize*(lcdltdc.pwidth*(lcdltdc.pheight-x-1)+y)); 
    }
#else
    if(lcdltdc.dir)
    {
        return *(uint16_t*)((uint32_t)ltdc_framebuf[lcdltdc.activelayer]+lcdltdc.pixsize*(lcdltdc.pwidth*y+x));
    }else
    {
        return *(uint16_t*)((uint32_t)ltdc_framebuf[lcdltdc.activelayer]+lcdltdc.pixsize*(lcdltdc.pwidth*(lcdltdc.pheight-x-1)+y)); 
    }
#endif 
}

void LTDC_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint32_t color)
{ 
    uint32_t psx,psy,pex,pey;	//以LCD面板为基准的坐标系，不随横屏竖屏而变化
    uint32_t timeout=0; 
    uint16_t offline;
    uint32_t addr; 
    //坐标系转换
    if(lcdltdc.dir)
    {
        psx=sx;psy=sy;
        pex=ex;pey=ey;
    }
    else
    {
        psx=sy;psy=lcdltdc.pheight-ex-1;
        pex=ey;pey=lcdltdc.pheight-sx-1;
    } 
    offline=lcdltdc.pwidth-(pex-psx+1);
    addr=((uint32_t)ltdc_framebuf[lcdltdc.activelayer]+lcdltdc.pixsize*(lcdltdc.pwidth*psy+psx));
    RCC->AHB1ENR|=1<<23;			//使能DM2D时钟
    DMA2D->CR=3<<16;				//寄存器到存储器模式
    DMA2D->OPFCCR=LCD_PIXFORMAT;	//颜色格式
    DMA2D->OOR=offline;				//设置行偏移 
    DMA2D->CR&=~(1<<0);				//先停止DMA2D
    DMA2D->OMAR=addr;				//输出存储器地址
    DMA2D->NLR=(pey-psy+1)|((pex-psx+1)<<16);	//设定行数寄存器
    DMA2D->OCOLR=color;				//设定颜色输出寄存器
    DMA2D->CR|=1<<0;				//启动DMA2D
    while((DMA2D->ISR&(1<<1))==0)	//等待传输完成
    {
        timeout++;
        if(timeout>0X1FFFFF)break;
    }  
    DMA2D->IFCR|=1<<1;				//清楚传输完成标志	
}
//void LTDC_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint32_t color)
//{
//	uint32_t psx,psy,pex,pey;
//	uint32_t timeout=0; 
//	uint16_t offline;
//	uint32_t addr;  
//    if(ex>=lcdltdc.width)ex=lcdltdc.width-1;
//	if(ey>=lcdltdc.height)ey=lcdltdc.height-1;
//	if(lcdltdc.dir)
//	{
//		psx=sx;psy=sy;
//		pex=ex;pey=ey;
//	}else
//	{
//		psx=sy;psy=lcdltdc.pheight-ex-1;
//		pex=ey;pey=lcdltdc.pheight-sx-1;
//	}
//	offline=lcdltdc.pwidth-(pex-psx+1);
//	addr=((uint32_t)ltdc_framebuf[lcdltdc.activelayer]+lcdltdc.pixsize*(lcdltdc.pwidth*psy+psx));
//    if(LCD_PIXFORMAT==LCD_PIXEL_FORMAT_RGB565)  
//    {
//        color=((color&0X0000F800)<<8)|((color&0X000007E0)<<5)|((color&0X0000001F)<<3);
//    }

//	DMA2D_Handler.Instance=DMA2D;
//	DMA2D_Handler.Init.Mode=DMA2D_R2M;
//	DMA2D_Handler.Init.ColorMode=LCD_PIXFORMAT;
//	DMA2D_Handler.Init.OutputOffset=offline;
//	HAL_DMA2D_Init(&DMA2D_Handler);
//    HAL_DMA2D_ConfigLayer(&DMA2D_Handler,lcdltdc.activelayer);
//    HAL_DMA2D_Start(&DMA2D_Handler,color,(uint32_t)addr,pex-psx+1,pey-psy+1);
//    HAL_DMA2D_PollForTransfer(&DMA2D_Handler,1000);
//    while((__HAL_DMA2D_GET_FLAG(&DMA2D_Handler,DMA2D_FLAG_TC)==0)&&(timeout<0X5000))
//    {
//        timeout++;
//    }
//    __HAL_DMA2D_CLEAR_FLAG(&DMA2D_Handler,DMA2D_FLAG_TC);
//}

void LTDC_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color)
{
    uint32_t psx,psy,pex,pey;
    uint32_t timeout=0; 
    uint16_t offline;
    uint32_t addr; 

    if(lcdltdc.dir)
    {
        psx=sx;psy=sy;
        pex=ex;pey=ey;
    }else
    {
        psx=sy;psy=lcdltdc.pheight-ex-1;
        pex=ey;pey=lcdltdc.pheight-sx-1;
    }
    offline=lcdltdc.pwidth-(pex-psx+1);
    addr=((uint32_t)ltdc_framebuf[lcdltdc.activelayer]+lcdltdc.pixsize*(lcdltdc.pwidth*psy+psx));
    RCC->AHB1ENR|=1<<23;
    DMA2D->CR=0<<16;
    DMA2D->FGPFCCR=LCD_PIXFORMAT;
    DMA2D->FGOR=0;
    DMA2D->OOR=offline;
    DMA2D->CR&=~(1<<0);
    DMA2D->FGMAR=(uint32_t)color;
    DMA2D->OMAR=addr;
    DMA2D->NLR=(pey-psy+1)|((pex-psx+1)<<16);
    DMA2D->CR|=1<<0;
    while((DMA2D->ISR&(1<<1))==0)
    {
        timeout++;
        if(timeout>0X1FFFFF)break;
    } 
    DMA2D->IFCR|=1<<1;
} 

void LTDC_Clear(uint32_t color)
{
    LTDC_Fill(0,0,lcdltdc.width-1,lcdltdc.height-1,color);
}

/*
* LTDC时钟(Fdclk)设置
* PLL3_VCO Input=HSE_VALUE/PLL3M
* PLL3_VCO Output=PLL3_VCO Input * PLL3N
* PLLLCDCLK = PLL3_VCO Output/PLL3R
* HSE_VALUE=25MHz,PLL3M=5,PLL3N=160,PLL3R=88
* LTDCLK=PLLLCDCLK=25/5*160/88=9MHz
*/
uint8_t LTDC_Clk_Set(uint32_t pll3m,uint32_t pll3n,uint32_t pll3r)
{
    RCC_PeriphCLKInitTypeDef PeriphClkIniture;
    PeriphClkIniture.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    PeriphClkIniture.PLL3.PLL3M = pll3m;    
    PeriphClkIniture.PLL3.PLL3N = pll3n;
    PeriphClkIniture.PLL3.PLL3P = 2;
    PeriphClkIniture.PLL3.PLL3Q = 2;  
    PeriphClkIniture.PLL3.PLL3R = pll3r;
    if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkIniture)==HAL_OK) //配置像素时钟，这里配置为18.75MHZ
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void LTDC_Layer_Window_Config(uint8_t layerx,uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
{
    HAL_LTDC_SetWindowPosition(&LTDC_Handler,sx,sy,layerx);
    HAL_LTDC_SetWindowSize(&LTDC_Handler,width,height,layerx); 
}

/*
* @brief		 : 设置层的基本参数，必须在函数 LTDC_Layer_Window_Config之前调用
* @param layerx : 层选择 0/1
* @param bufaddr: 层颜色帧缓存始地址
* @parma pixformat:颜色格式：0-ARGB8888;1-RGB888;2-RGB565;3-ARGB1555;4-ARGB4444;5-L8;6-AL44;7-AL88
* @param alpha	 : 层颜色alpha值，0-全透明； 255-不透明
* @param alpha0 : 默认颜色alpha值，0-全透明； 255-不透明
* @param bfac1	 : 混合系数1,4(100),恒定的alpha； 6(101),像素alpha*恒定alpha值
* @param bfac2	 : 混合系数2,5(101),恒定的alpha； 7(111),像素alpha*恒定alpha值
* @param bkcolor: 层默认颜色，32位，低24位有效，RGB888格式
* @return	NONE
*/
void LTDC_Layer_Parameter_Config(uint8_t layerx,uint32_t bufaddr,uint8_t pixformat,uint8_t alpha,uint8_t alpha0,uint8_t bfac1,uint8_t bfac2,uint32_t bkcolor)
{
    LTDC_LayerCfgTypeDef pLayerCfg;

    pLayerCfg.WindowX0=0;                       //窗口起始X坐标
    pLayerCfg.WindowY0=0;                       //窗口起始Y坐标
    pLayerCfg.WindowX1=lcdltdc.pwidth;          //窗口终止X坐标
    pLayerCfg.WindowY1=lcdltdc.pheight;         //窗口终止Y坐标
    pLayerCfg.PixelFormat=pixformat;		    //像素格式
    pLayerCfg.Alpha=alpha;				        //Alphaֵ值设置
    pLayerCfg.Alpha0=alpha0;			        //默认Alphaֵ值
    pLayerCfg.BlendingFactor1=(uint32_t)bfac1<<8;//设置混合系数
    pLayerCfg.BlendingFactor2=(uint32_t)bfac2<<8;
    pLayerCfg.FBStartAdress=bufaddr;			//设置颜色帧缓存起始地址
    pLayerCfg.ImageWidth=lcdltdc.pwidth;        //设置颜色帧缓冲区宽度
    pLayerCfg.ImageHeight=lcdltdc.pheight;      //设置颜色帧缓冲区高度
    pLayerCfg.Backcolor.Red=(uint8_t)(bkcolor&0X00FF0000)>>16;   //背景颜色红色部分
    pLayerCfg.Backcolor.Green=(uint8_t)(bkcolor&0X0000FF00)>>8;  //背景颜色绿色部分
    pLayerCfg.Backcolor.Blue=(uint8_t)bkcolor&0X000000FF;        //背景颜色蓝色部分
    HAL_LTDC_ConfigLayer(&LTDC_Handler,&pLayerCfg,layerx);   //设置选中的层参数
}  

//PG6=R7(M0);PI2=G7(M1);PI7=B7(M2);
//M2:M1:M0
//0 :0 :0	//4.3 吋 480*272 RGB屏,ID=0X4342
//0 :0 :1	//7   吋 800*480 RGB屏,ID=0X7084
//0 :1 :0	//7   吋1024*600 RGB屏,ID=0X7016
//0 :1 :1	//7   吋1280*800 RGB屏,ID=0X7018
//1 :0 :0	//4.3 吋 800*480 RGB屏,ID=0X4384
//1 :0 :1   //10.1吋1280*800,RGB屏,ID=0X1018
//1 :1 :1   //VGA显示器
//返回值:LCD ID:0非法值
uint16_t LTDC_PanelID_Read(void)
{
    uint8_t idx=0;
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();

    GPIO_Initure.Pin=GPIO_PIN_6;        //PG6
    GPIO_Initure.Mode=GPIO_MODE_INPUT;
    GPIO_Initure.Pull=GPIO_PULLUP;
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOG,&GPIO_Initure);

    GPIO_Initure.Pin=GPIO_PIN_2|GPIO_PIN_7; //PI2,7
    HAL_GPIO_Init(GPIOI,&GPIO_Initure);

    idx=(uint8_t)HAL_GPIO_ReadPin(GPIOG,GPIO_PIN_6); 	//读取M0
    idx|=(uint8_t)HAL_GPIO_ReadPin(GPIOI,GPIO_PIN_2)<<1;//读取M1
    idx|=(uint8_t)HAL_GPIO_ReadPin(GPIOI,GPIO_PIN_7)<<2;//读取M2
    if(idx==0) return 0X4342;	    //4.3吋,480*272分辨率
    else if(idx==1)return 0X7084;	//7吋,800*480分辨率
    else if(idx==2)return 0X7016;	//7吋,1024*600分辨率
    else if(idx==3)return 0X7018;	//7吋,1280*800分辨率
    else if(idx==4)return 0X4384;	//4.3吋,800*480分辨率
    else if(idx==5)return 0X1018;	//10.1吋,1280*800分辨率		
    else return 0;
}

void LTDC_Init(void)
{   
    uint16_t lcdid=0;

    lcdid=LTDC_PanelID_Read();
    if(lcdid==0X4342)
    {
        lcdltdc.pwidth=480;			    //面板宽度，单位:像素
        lcdltdc.pheight=272;		    //面板高度，单位:像素
        lcdltdc.hsw=1;				    //水平同步宽度
        lcdltdc.vsw=1;				    //垂直同步宽度
        lcdltdc.hbp=40;				    //水平后廊
        lcdltdc.vbp=8;				    //垂直后廊
        lcdltdc.hfp=5;				    //水平前廊
        lcdltdc.vfp=8;				    //垂直前廊
        LTDC_Clk_Set(5,160,88);			//设置像素时钟:9Mhz 
    }else if(lcdid==0X7084)
    {
        lcdltdc.pwidth=800;
        lcdltdc.pheight=480;
        lcdltdc.hsw=1;
        lcdltdc.vsw=1;
        lcdltdc.hbp=46;
        lcdltdc.vbp=23;
        lcdltdc.hfp=210;
        lcdltdc.vfp=22;
        LTDC_Clk_Set(5,160,28);			//设置像素时钟33M(如果开双显，需要降低DCLK到:18.75Mhz  pll3r=43,才会好)
    }else if(lcdid==0X7016)		
    {
        lcdltdc.pwidth=1024;
        lcdltdc.pheight=600;
        lcdltdc.hsw=20;
        lcdltdc.vsw=3;
        lcdltdc.hbp=140;
        lcdltdc.vbp=20;
        lcdltdc.hfp=160;
        lcdltdc.vfp=12;
        LTDC_Clk_Set(5,160,20);			//40Mhz 
    }else if(lcdid==0X7018)		
    {
        lcdltdc.pwidth=1280;
        lcdltdc.pheight=800;
    }else if(lcdid==0X4384)		
    {
        lcdltdc.pwidth=800;
        lcdltdc.pheight=480;
        lcdltdc.hbp=88;
        lcdltdc.hfp=40;
        lcdltdc.hsw=48;
        lcdltdc.vbp=32;
        lcdltdc.vfp=13;
        lcdltdc.vsw=3;
        LTDC_Clk_Set(5,160,24);			//33M
    }else if(lcdid==0X1018)
    {
        lcdltdc.pwidth=1280;
        lcdltdc.pheight=800;
        lcdltdc.hbp=140;
        lcdltdc.hfp=10;
        lcdltdc.hsw=10;
        lcdltdc.vbp=10;
        lcdltdc.vfp=10;
        lcdltdc.vsw=3;
        LTDC_Clk_Set(5,160,16);			//50MHz
    }	else if(lcdid==0XA001)          //接VGA显示器
        { 
        lcdltdc.pwidth=1366;
        lcdltdc.pheight=768;
        lcdltdc.hsw=143;
        lcdltdc.hfp=70;
        lcdltdc.hbp=213;
        lcdltdc.vsw=3;
        lcdltdc.vbp=24;
        lcdltdc.vfp=3;
        LTDC_Clk_Set(5,160,28);			//28.5Mhz
        
        }else if(lcdid==0XA002)            
        {  
        lcdltdc.pwidth=1280;	
        lcdltdc.pheight=800;	
        lcdltdc.hsw=32;			
        lcdltdc.hfp=48;			
        lcdltdc.hbp=80;			
        lcdltdc.vsw=6;			
        lcdltdc.vbp=14;			
        lcdltdc.vfp=3;			
        LTDC_Clk_Set(5,160,22);
        }else if(lcdid==0XA003)	
        {
        lcdltdc.pwidth=1280;	
        lcdltdc.pheight=768;	
        lcdltdc.hsw=32;			
        lcdltdc.hbp=80;			
        lcdltdc.hfp=48;			
        lcdltdc.vsw=7;			
        lcdltdc.vbp=12;			
        lcdltdc.vfp=3;			
        LTDC_Clk_Set(5,160,23);
        }else if(lcdid==0XA004)   
        {
        lcdltdc.pwidth=1024;	
        lcdltdc.pheight=768;	
        lcdltdc.hsw=136;		
        lcdltdc.hfp=24;			
        lcdltdc.hbp=160;		
        lcdltdc.vsw=6;			
        lcdltdc.vbp=29;			
        lcdltdc.vfp=3;			
        LTDC_Clk_Set(5,160,18);
        }else if(lcdid==0XA005)   
        {
        lcdltdc.pwidth=848;		
        lcdltdc.pheight=480;	
        lcdltdc.hsw=112;		
        lcdltdc.hbp=112;		
        lcdltdc.hfp=16;			
        lcdltdc.vsw=8;			
        lcdltdc.vbp=23;			
        lcdltdc.vfp=6;			
        LTDC_Clk_Set(5,160,47);
        }else if(lcdid==0XA006)   
        {
        lcdltdc.pwidth=800;	
        lcdltdc.pheight=600;
        lcdltdc.hsw=128;	
        lcdltdc.hbp=88;		
        lcdltdc.hfp=40;		
        lcdltdc.vsw=4;		
        lcdltdc.vbp=23;		
        lcdltdc.vfp=1;		
        LTDC_Clk_Set(5,160,40);		
        }else if(lcdid==0XA007)
        {
        lcdltdc.pwidth=640;
        lcdltdc.pheight=480;
        lcdltdc.hsw=96;			
        lcdltdc.hfp=8;			
        lcdltdc.hbp=56;			
        lcdltdc.vsw=2;			
        lcdltdc.vbp=41;			
        lcdltdc.vfp=2;			
        LTDC_Clk_Set(5,160,62);
        } 

    lcddev.width=lcdltdc.pwidth;
    lcddev.height=lcdltdc.pheight;

#if LCD_PIXFORMAT==LCD_PIXFORMAT_ARGB8888||LCD_PIXFORMAT==LCD_PIXFORMAT_RGB888 
    ltdc_framebuf[0]=(uint32_t*)&ltdc_lcd_framebuf;
    lcdltdc.pixsize=4;				//每个像素占4字节
#else 
    lcdltdc.pixsize=2;				//每个像素占2字节
    ltdc_framebuf[0]=(uint32_t*)&ltdc_lcd_framebuf;
#endif 	

    LTDC_Handler.Instance=LTDC;
    LTDC_Handler.Init.HSPolarity=LTDC_HSPOLARITY_AL;         	//水平同步极性
    LTDC_Handler.Init.VSPolarity=LTDC_VSPOLARITY_AL;         	//垂直同步极性
    LTDC_Handler.Init.DEPolarity=LTDC_DEPOLARITY_AL;         	//数据使能极性
    LTDC_Handler.Init.PCPolarity=LTDC_PCPOLARITY_IPC;        	//像素时钟极性
    LTDC_Handler.Init.HorizontalSync=lcdltdc.hsw-1;          	//水平同步宽度
    LTDC_Handler.Init.VerticalSync=lcdltdc.vsw-1;            	//垂直同步宽度
    LTDC_Handler.Init.AccumulatedHBP=lcdltdc.hsw+lcdltdc.hbp-1; //水平同步后沿宽度
    LTDC_Handler.Init.AccumulatedVBP=lcdltdc.vsw+lcdltdc.vbp-1; //垂直同步后沿宽度
    LTDC_Handler.Init.AccumulatedActiveW=lcdltdc.hsw+lcdltdc.hbp+lcdltdc.pwidth-1;		//有效宽度
    LTDC_Handler.Init.AccumulatedActiveH=lcdltdc.vsw+lcdltdc.vbp+lcdltdc.pheight-1;		//有效高度
    LTDC_Handler.Init.TotalWidth=lcdltdc.hsw+lcdltdc.hbp+lcdltdc.pwidth+lcdltdc.hfp-1;  //总宽度
    LTDC_Handler.Init.TotalHeigh=lcdltdc.vsw+lcdltdc.vbp+lcdltdc.pheight+lcdltdc.vfp-1; //总高度
    LTDC_Handler.Init.Backcolor.Red=0;           //屏幕背景层红色部分
    LTDC_Handler.Init.Backcolor.Green=0;         //屏幕背景层绿色部分
    LTDC_Handler.Init.Backcolor.Blue=0;          //屏幕背景层蓝色部分
    HAL_LTDC_Init(&LTDC_Handler);

    LTDC_Layer_Parameter_Config(0,(uint32_t)ltdc_framebuf[0],LCD_PIXFORMAT,255,0,6,7,0X000000);
    LTDC_Layer_Window_Config(0,0,0,lcdltdc.pwidth,lcdltdc.pheight);
        
    LTDC_Display_Dir(0);			//默认竖屏
    LTDC_Select_Layer(0); 			//选择第一层
    LCD_LED(1);         		    //点亮背光
    LTDC_Clear(0XFFFFFFFF);			//清屏
}

void HAL_LTDC_MspInit(LTDC_HandleTypeDef* hltdc)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_LTDC_CLK_ENABLE(); 
    __HAL_RCC_DMA2D_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();

    GPIO_Initure.Pin=GPIO_PIN_5;                //PB5背光控制引脚
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;      
    GPIO_Initure.Pull=GPIO_PULLUP;                  
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);

    GPIO_Initure.Pin=GPIO_PIN_10; 
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;          
    GPIO_Initure.Pull=GPIO_NOPULL;              
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_Initure.Alternate=GPIO_AF14_LTDC;      
    HAL_GPIO_Init(GPIOF,&GPIO_Initure);

    GPIO_Initure.Pin=GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_11;
    HAL_GPIO_Init(GPIOG,&GPIO_Initure);

    GPIO_Initure.Pin=GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|\
                        GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    HAL_GPIO_Init(GPIOH,&GPIO_Initure);

    GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_5|\
                        GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_9|GPIO_PIN_10;
    HAL_GPIO_Init(GPIOI,&GPIO_Initure); 
}

