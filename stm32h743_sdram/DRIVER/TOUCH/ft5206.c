#include "ft5206.h"
#include "touch.h"
#include "ctiic.h"
#include "usart.h"
#include "string.h" 
#include "lcd.h"

uint8_t FT5206_WR_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
	uint8_t i;
	uint8_t ret=0;
	CT_IIC_Start();	 
	CT_IIC_Send_Byte(FT_CMD_WR);
	CT_IIC_Wait_Ack(); 	 		
	CT_IIC_Send_Byte(reg&0XFF);   
	CT_IIC_Wait_Ack();  
	for(i=0;i<len;i++)
	{	   
    	CT_IIC_Send_Byte(buf[i]);  
		ret=CT_IIC_Wait_Ack();
		if(ret)break;  
	}
    CT_IIC_Stop();					 
	return ret; 
}

void FT5206_RD_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
	uint8_t i; 
 	CT_IIC_Start();	
 	CT_IIC_Send_Byte(FT_CMD_WR);	 
	CT_IIC_Wait_Ack(); 					  		   
 	CT_IIC_Send_Byte(reg&0XFF);
	CT_IIC_Wait_Ack();  
 	CT_IIC_Start();  	 	   
	CT_IIC_Send_Byte(FT_CMD_RD);	   
	CT_IIC_Wait_Ack();	   
	for(i=0;i<len;i++)
	{	   
    	buf[i]=CT_IIC_Read_Byte(i==(len-1)?0:1);  
	} 
    CT_IIC_Stop(); 
} 

uint8_t FT5206_Init(void)
{
	uint8_t temp[2]; 
    GPIO_InitTypeDef GPIO_Initure;
 
    __HAL_RCC_GPIOH_CLK_ENABLE();			
    __HAL_RCC_GPIOI_CLK_ENABLE();			
                
    //PH7
    GPIO_Initure.Pin=GPIO_PIN_7;            
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      
    GPIO_Initure.Pull=GPIO_PULLUP;          
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;     
    HAL_GPIO_Init(GPIOH,&GPIO_Initure);    
            
    //PI8
    GPIO_Initure.Pin=GPIO_PIN_8;            
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  
    HAL_GPIO_Init(GPIOI,&GPIO_Initure);     
        
	CT_IIC_Init();
	FT_RST(0);
	HAL_Delay(20);
 	FT_RST(1);
	HAL_Delay(50);  	
	temp[0]=0;
	FT5206_WR_Reg(FT_DEVIDE_MODE,temp,1);	//进入正常操作模式
	FT5206_WR_Reg(FT_ID_G_MODE,temp,1);		//查询模式
	temp[0]=22;								//触摸有效值，22， 越小越灵敏
	FT5206_WR_Reg(FT_ID_G_THGROUP,temp,1);	//设置触摸有效值
	temp[0]=12;								//激活周期，不能小于12，最大14
	FT5206_WR_Reg(FT_ID_G_PERIODACTIVE,temp,1); 
	//读取版本号
	FT5206_RD_Reg(FT_ID_G_LIB_VERSION,&temp[0],2);  
	if((temp[0]==0X30&&temp[1]==0X03)||temp[1]==0X01||temp[1]==0X02)//参考值:0X3003/0X0001/0X0002
	{
		printf("CTP ID:%x\r\n",((uint16_t)temp[0]<<8)+temp[1]);
		return 0;
	} 
	return 1;
}
const uint16_t FT5206_TPX_TBL[5]={FT_TP1_REG,FT_TP2_REG,FT_TP3_REG,FT_TP4_REG,FT_TP5_REG};

/*
 * 扫描触摸屏，采用查询方式，mode = 0 表示正常扫描
 * 返回值，0-无触摸， 1-有触摸
 */
uint8_t FT5206_Scan(uint8_t mode)
{
	uint8_t buf[4];
	uint8_t i=0;
	uint8_t res=0;
	uint8_t temp;
	static uint8_t t=0;//控制查询间隔，降低cpu使用率  
	t++;
	if((t%10)==0||t<10)//降低10倍
	{
		FT5206_RD_Reg(FT_REG_NUM_FINGER,&mode,1);//读取触摸点状态  
		if((mode&0XF)&&((mode&0XF)<6))
		{
			temp=0XFF<<(mode&0XF);//将点的个数转换为1的个数，匹配tp_dev.sta 
			tp_dev.sta=(~temp)|TP_PRES_DOWN|TP_CATH_PRES; 
			for(i=0;i<5;i++)
			{
				if(tp_dev.sta&(1<<i))		//触摸有效？
				{
					FT5206_RD_Reg(FT5206_TPX_TBL[i],buf,4);	//读取xy坐标
					if(tp_dev.touchtype&0X01)//横屏
					{
						tp_dev.y[i]=((uint16_t)(buf[0]&0X0F)<<8)+buf[1];
						tp_dev.x[i]=((uint16_t)(buf[2]&0X0F)<<8)+buf[3];
					}else
					{
						tp_dev.x[i]=lcddev.width-(((uint16_t)(buf[0]&0X0F)<<8)+buf[1]);
						tp_dev.y[i]=((uint16_t)(buf[2]&0X0F)<<8)+buf[3];
					}  
					if((buf[0]&0XF0)!=0X80)tp_dev.x[i]=tp_dev.y[i]=0; // 必须时contact时间才有效
					//printf("x[%d]:%d,y[%d]:%d\r\n",i,tp_dev.x[i],i,tp_dev.y[i]);
				}			
			} 
			res=1;
			if(tp_dev.x[0]==0 && tp_dev.y[0]==0)mode=0;
			t=0;
		}
	}
	if((mode&0X1F)==0)//无触摸点按下
	{ 
		if(tp_dev.sta&TP_PRES_DOWN)	//֮之前是被按下的
		{
			tp_dev.sta&=~(1<<7);	//标记按键松开
		}
        else						//֮之前就没有被按下
		{ 
			tp_dev.x[0]=0xffff;
			tp_dev.y[0]=0xffff;
			tp_dev.sta&=0XE0;	//清除点有效标志
		}	 
	} 	
	if(t>240)t=10;
	return res;
}
 



























