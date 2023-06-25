#include "ctiic.h"

extern void delay_us(uint32_t nus);

void CT_Delay(void)
{
	delay_us(2);
} 

void CT_IIC_Init(void)
{					     
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOH_CLK_ENABLE();			
	__HAL_RCC_GPIOI_CLK_ENABLE();			
    
    GPIO_Initure.Pin=GPIO_PIN_6;            //PH6
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  
    GPIO_Initure.Pull=GPIO_PULLUP;          
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOH,&GPIO_Initure);
	
    GPIO_Initure.Pin=GPIO_PIN_3;       
    HAL_GPIO_Init(GPIOI,&GPIO_Initure);
}

void CT_IIC_Start(void)
{
	CT_SDA_OUT();
	CT_IIC_SDA(1);	  	  
	CT_IIC_SCL(1);
	delay_us(30);
 	CT_IIC_SDA(0);//START:when CLK is high,DATA change form high to low 
	CT_Delay();
	CT_IIC_SCL(0);
}	  

void CT_IIC_Stop(void)
{
	CT_SDA_OUT();
	CT_IIC_SCL(1);
	delay_us(30);
	CT_IIC_SDA(0);//STOP:when CLK is high DATA change form low to high
	CT_Delay();
	CT_IIC_SDA(1);
}

uint8_t CT_IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
	CT_SDA_IN();
	CT_IIC_SDA(1);	   
	CT_IIC_SCL(1);
	CT_Delay();
	while(CT_READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			CT_IIC_Stop();
			return 1;
		} 
		CT_Delay();
	}
	CT_IIC_SCL(0);
	return 0;  
} 

void CT_IIC_Ack(void)
{
	CT_IIC_SCL(0);
	CT_SDA_OUT();
	CT_Delay();
	CT_IIC_SDA(0);
	CT_Delay();
	CT_IIC_SCL(1);
	CT_Delay();
	CT_IIC_SCL(0);
}

void CT_IIC_NAck(void)
{
	CT_IIC_SCL(0);
	CT_SDA_OUT();
	CT_Delay();
	CT_IIC_SDA(1);
	CT_Delay();
	CT_IIC_SCL(1);
	CT_Delay();
	CT_IIC_SCL(0);
}					 				     

void CT_IIC_Send_Byte(uint8_t txd)
{                        
    uint8_t t;   
	CT_SDA_OUT(); 	    
    CT_IIC_SCL(0);
	CT_Delay();
	for(t=0;t<8;t++)
    {              
        CT_IIC_SDA((txd&0x80)>>7);
        txd<<=1; 	      
		CT_IIC_SCL(1); 
		CT_Delay();
		CT_IIC_SCL(0);	
		CT_Delay();
    }	 
} 	    

uint8_t CT_IIC_Read_Byte(unsigned char ack)
{
	uint8_t i,receive=0;
 	CT_SDA_IN();
	CT_Delay();
	for(i=0;i<8;i++ )
	{ 
		CT_IIC_SCL(0); 	    	   
		CT_Delay();
		CT_IIC_SCL(1);	 
		receive<<=1;
		if(CT_READ_SDA)receive++;   
	}	  				 
	if (!ack)CT_IIC_NAck();
	else CT_IIC_Ack();  
 	return receive;
}




























