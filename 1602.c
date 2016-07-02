#include "stm8s_conf.h"

void LCD1602_delay(int ms){
	unsigned int j,k;
	while(ms--)
		for(j=0;j<255;j++)
			for(k=0;k<1;k++);
}

unsigned char swapbit4(unsigned char value){
	unsigned char nv=0x00;
	int i;
	
	value=value/0x10;
	for(i=0;i<4;i++){
		nv<<=1;
		if(value&0x01)
			nv|=0x01;
		value>>=1;
	}
	nv<<=4;
	return nv;
}


void LCD1602_write(unsigned char data){
	unsigned int port_value,newdata,result;
	port_value = GPIO_ReadOutputData(GPIOC);
	newdata = swapbit4(data);
	result = port_value%0x10 + newdata;
	//LCD1602_delay(1);
	GPIO_WriteLow(GPIOD, GPIO_PIN_4); //E=0
	//LCD1602_delay(1);
	GPIO_Write(GPIOC, result); //写出高四位
	//LCD1602_delay(1);
	GPIO_WriteHigh(GPIOD, GPIO_PIN_4); //E=1
	
	data<<=4;
	newdata = swapbit4(data);
	result = port_value%0x10 + newdata;
	
	LCD1602_delay(1);
	GPIO_WriteLow(GPIOD, GPIO_PIN_4); //E=0
	//LCD1602_delay(1);
	GPIO_Write(GPIOC, result); //写出低四位
	//LCD1602_delay(1);
	GPIO_WriteHigh(GPIOD, GPIO_PIN_4); //E=1
	LCD1602_delay(1);
	GPIO_WriteLow(GPIOD, GPIO_PIN_4); //E=0
}

void LCD1602_waitBusy(void){
	GPIO_WriteHigh(GPIOD, GPIO_PIN_2); //RW=1
	GPIO_WriteLow(GPIOD, GPIO_PIN_3); //RS=0
	GPIO_WriteHigh(GPIOC, GPIO_PIN_4); //D7=1
	GPIO_WriteHigh(GPIOD, GPIO_PIN_4); //E=1
	while(GPIO_ReadInputPin(GPIOC, GPIO_PIN_4)==RESET);
	GPIO_WriteLow(GPIOD, GPIO_PIN_4); //E=1
}

void LCD1602_writeCommand( unsigned char command ){

  LCD1602_waitBusy();
	GPIO_WriteLow(GPIOD, GPIO_PIN_2); //RW=0
	GPIO_WriteLow(GPIOD, GPIO_PIN_3); //RS=0
	LCD1602_write(command);
	
}

void LCD1602_writeText(unsigned char text ){
	
	GPIO_WriteLow(GPIOD, GPIO_PIN_2); //RW=0
	GPIO_WriteHigh(GPIOD, GPIO_PIN_3); //RS=1
	LCD1602_write(text);
	
}

/**
 * init IO first!
 *
 **/
void LCD1602_init(){
	LCD1602_writeCommand(0x28);
	LCD1602_writeCommand(0x01);
	LCD1602_writeCommand(0x02);
	LCD1602_writeCommand(0x06);
	LCD1602_writeCommand(0x0f);
}

void LCD1602_print(int x, int y, unsigned char text){
	LCD1602_writeCommand(0x80+x%0x10+y%2*0x40);
	LCD1602_writeText(text);
}

void LCD1602_printl(int x, int y, char *data){
 while (*data!='\0') //若到达字串尾则退出
  {
   if (x <= 0xF) //X坐标应小于0xF
    {
     LCD1602_print(x, y, *data); //显示单个字符
     data++;
     x++;
    }
		else break;
  }
}

