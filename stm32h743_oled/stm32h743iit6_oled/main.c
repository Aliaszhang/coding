#include "sys.h"
#include "delay.h" 
#include "led.h"  
#include "key.h"
#include "usart.h" 
#include "oled.h"  

   
int main(void)
{  
	u8 t=0,led0sta=1, led1sta = 0, tmp = 0;
	Stm32_Clock_Init(160,5,2,4);	//����ʱ��,400Mhz
    delay_init(400);				//��ʱ��ʼ��  
	uart_init(100,115200);			//���ڳ�ʼ��Ϊ115200 
	LED_Init();						//��ʼ����LED���ӵ�Ӳ���ӿ� 
    KEY_Init();
	OLED_Init();					//��ʼ��OLED
  	OLED_ShowString(0,0,"Alias",24);  
	OLED_ShowString(0,24, "0.96' OLED TEST",16);  
 	OLED_ShowString(0,40,"KEY press:",12);  
 	OLED_ShowString(0,52,"ASCII:",12);  
 	OLED_ShowString(64,52,"CODE:",12);  
	OLED_Refresh_Gram();			//������ʾ��OLED	
    t=' ';
    
//    delay_ms(500);
//    OLED_ShowChinese(0,0,32,1);
//    delay_ms(500);
    
    
    printf("\r\nstart test\r\n");
	while(1) 
	{		
		OLED_ShowChar(36,52,t,12,1);//��ʾASCII�ַ�	
		OLED_ShowNum(94,52,t,3,12);	//��ʾASCII�ַ�����ֵ    
		OLED_Refresh_Gram();		//������ʾ��OLED
		t++;
		if(t>'~')t=' ';  
		delay_ms(500);
		LED0(led0sta^=1);			//LED0ȡ��
        LED1(led1sta^=1);
        
        tmp = KEY_Scan(1);
        printf("key press = %d\r\n", tmp);
        OLED_ShowNum(66,40,tmp,3,12);
	}
}


















