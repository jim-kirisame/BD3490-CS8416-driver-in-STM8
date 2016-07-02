/* MAIN.C file
 * 
 * Copyright (c) 2002-2005 STMicroelectronics
 */
#include "stm8s_conf.h"
#ifdef DEBUG
	#include "1602.h"
#endif
#include "i2c.h"
void init_basic(void){
#ifdef DEBUG
	GPIO_Init(GPIOC, GPIO_PIN_HNIB, GPIO_MODE_OUT_PP_HIGH_FAST);
	GPIO_Init(GPIOD, GPIO_PIN_LNIB, GPIO_MODE_OUT_PP_HIGH_FAST);
	GPIO_Init(GPIOD, GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_FAST);
#endif
	GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_IN_PU_NO_IT);
	GPIO_Init(GPIOD, GPIO_PIN_6, GPIO_MODE_IN_PU_NO_IT);
}

void init_i2c(void){
	/*!< sEE_I2C Peripheral clock enable */
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_I2C, ENABLE);
	
	/* I2C configuration */
  /* sEE_I2C Peripheral Enable */
  I2C_Cmd( ENABLE);
  /* sEE_I2C configuration after enabling it */
  I2C_Init(100000, 0xA0, I2C_DUTYCYCLE_2, I2C_ACK_CURR, 
           I2C_ADDMODE_7BIT, 16);
}

void sEE_EnterCriticalSection_UserCallback(void)
{
  disableInterrupts();  
}

void sEE_ExitCriticalSection_UserCallback(void)
{
  enableInterrupts();
}

uint32_t sEE_TIMEOUT_UserCallback(uint8_t errorCode)
{
#ifdef DEBUG
	LCD1602_printl(0,0,"IIC_TIMEOUT");
	LCD1602_print(15,0,errorCode);
#endif
	return sEE_FAIL;
}

uint32_t I2C_write(uint8_t mainAddr, uint16_t WriteAddr, uint8_t data)
{
  
  /* 检测总线是否忙 */
  uint32_t sEETimeout = sEE_LONG_TIMEOUT;
  while(I2C_GetFlagStatus(I2C_FLAG_BUSBUSY))
  {
    if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback('B');
  }
  
  /* Send START condition */
  I2C_GenerateSTART( ENABLE);
  
  /* Test on EV5 and clear it */
  sEETimeout = sEE_FLAG_TIMEOUT;
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT))
  {
    if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback('5');
  }
  
  /* Send EEPROM address for write */
  sEETimeout = sEE_FLAG_TIMEOUT;
  I2C_Send7bitAddress((uint8_t)mainAddr, I2C_DIRECTION_TX);

  /* Test on EV6 and clear it */
  sEETimeout = sEE_FLAG_TIMEOUT;
  while(!I2C_CheckEvent( I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
  {
    if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback('6');
  }

  
  /* Send the EEPROM's internal address to write to : only one byte Address */
  I2C_SendData( WriteAddr);
  
  /* Test on EV8 and clear it */
  sEETimeout = sEE_FLAG_TIMEOUT; 
  while(!I2C_CheckEvent( I2C_EVENT_MASTER_BYTE_TRANSMITTED))
  {
    if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback('8');
  }
  
  /* While there is data to be written */
	/* Send the byte to be written */
	I2C_SendData( data);

	/* Test on EV8 and clear it */
	/* Wait till all data have been physically transferred on the bus */
	sEETimeout = sEE_LONG_TIMEOUT;
	while(!I2C_GetFlagStatus( I2C_FLAG_TRANSFERFINISHED))
	{
		if((sEETimeout--) == 0)  sEE_TIMEOUT_UserCallback('D');
	}
  
	/* Send STOP condition */
	I2C_GenerateSTOP(ENABLE);
    
	/* Perform a read on SR1 and SR3 register to clear eventually pending flags */
	(void)I2C->SR1;
	(void)I2C->SR3;

  /* If all operations OK, return sEE_OK (0) */
  return sEE_OK;  
}

uint32_t I2C_read(uint8_t mainAddr, uint16_t ReadAddr, uint8_t* pBuffer)
{

  /* While the bus is busy */
  /* 检测I2C总线是否忙 */
  uint32_t sEETimeout = sEE_LONG_TIMEOUT;
  while(I2C_GetFlagStatus( I2C_FLAG_BUSBUSY))
  {
    if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback('B');
  }

  /* Send START condition */
  /* 发送开始帧 */
  I2C_GenerateSTART(ENABLE);

  /* Test on EV5 and clear it (cleared by reading SR1 then writing to DR) */
  /* 测试EV5并清除 */
  sEETimeout = sEE_FLAG_TIMEOUT;
  while(!I2C_CheckEvent( I2C_EVENT_MASTER_MODE_SELECT))
  {
    if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback('5');
  }

  /* Send EEPROM address for write */
  /* 发送地址 */
  I2C_Send7bitAddress( (uint8_t)mainAddr, I2C_DIRECTION_TX);

  /* Test on EV6 and clear it */
  /* 测试EV6并清除 */
  sEETimeout = sEE_FLAG_TIMEOUT;
  while(!I2C_CheckEvent( I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
  {
    if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback('6');
  } 
	
	  
  /* Send the EEPROM's internal address to write to : only one byte Address */
  I2C_SendData( ReadAddr);
  

  /* Test on EV8 and clear it */
	/* 测试EV8并清除 */
  sEETimeout = sEE_FLAG_TIMEOUT;
  while(I2C_GetFlagStatus(I2C_FLAG_TRANSFERFINISHED) == RESET)
  {
    if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback('8');
  }
  
  /* Send START condition a second time */  
	/* 第二次发送开始帧 */
  I2C_GenerateSTART( ENABLE);
  
  /* Test on EV5 and clear it (cleared by reading SR1 then writing to DR) */
	/* 测试EV5并清除 */
  sEETimeout = sEE_FLAG_TIMEOUT;
  while(!I2C_CheckEvent( I2C_EVENT_MASTER_MODE_SELECT))
  {
    if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback('S');
  } 
  
  /* Send EEPROM address for read */
	/* 发送地址 */
  I2C_Send7bitAddress((uint8_t)mainAddr, I2C_DIRECTION_RX);

  /* Wait on ADDR flag to be set (ADDR is still not cleared at this level */
	/* 等待ADDR标志被设置。(ADDR并没有被清除) */
  sEETimeout = sEE_FLAG_TIMEOUT;
  while(I2C_GetFlagStatus( I2C_FLAG_ADDRESSSENTMATCHED) == RESET)
  {
    if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback('A');
  } 
     
  /* Disable Acknowledgement */
	/* 禁用ACK */
  I2C_AcknowledgeConfig(I2C_ACK_NONE);   

  /* Call User callback for critical section start (should typically disable interrupts) */
	/* 进入临界区 */
  sEE_EnterCriticalSection_UserCallback();
  
  /* Clear ADDR register by reading SR1 then SR3 register (SR1 has already been read) */
	/* 这里是清除 EV6 */
  (void)I2C->SR3;
  
  /* Send STOP Condition */
	/* 重设STOP/START帧 */
  I2C_GenerateSTOP( ENABLE);
  
	/* Call User callback for critical section end (should typically re-enable interrupts) */
	/* 退出临界区 */
  sEE_ExitCriticalSection_UserCallback();
    
  /* Wait for the byte to be received */
	/* 等待 */
  sEETimeout = sEE_FLAG_TIMEOUT;
  while(I2C_GetFlagStatus( I2C_FLAG_RXNOTEMPTY) == RESET)
  {
    if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback('G');
  }
    
  /* Read the byte received from the EEPROM */
  *pBuffer = I2C_ReceiveData();
    
  /* Wait to make sure that STOP control bit has been cleared */
  sEETimeout = sEE_FLAG_TIMEOUT;
  while(I2C->CR2 & I2C_CR2_STOP)
  {
    if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback('s');
  }  
    
  /* Re-Enable Acknowledgement to be ready for another reception */
  I2C_AcknowledgeConfig( I2C_ACK_CURR);    

  /* If all operations OK, return sEE_OK (0) */
  return sEE_OK;  
}

void init_BD3490(uint8_t volume){
	#ifdef DEBUG
		LCD1602_printl(0,0,"Initing BD3490  ");
		LCD1602_printl(0,1,"                ");
	#endif
	
		I2C_write(0x80, 0xfe, 0x81); //Restart BD3490
		I2C_write(0x80, 0x04, 0x00); //Select input from Channel A
		I2C_write(0x80, 0x06, 0x00); //Set input gain to min
		I2C_write(0x80, 0x21, volume); //Set volume to 0db
		I2C_write(0x80, 0x22, volume); //Set volume to 0db
	#ifdef DEBUG
		LCD1602_printl(9,1,"done");
	#endif
}
void init_cs8416(void){
	I2C_write(0x20,0x00,0x08);      //// more wideband jitter, less inband
	I2C_write(0x20,0x01,0x80);      //// INT active high, zero SDOUT on Rx error, RMCK=128Fs
	I2C_write(0x20,0x02,0x00);      //// automatic de-emphasis enabled

	I2C_write(0x20,0x03,0xc0);        // GPO1 = VCC output
	I2C_write(0x20,0x04,0x38);        // RUN=0, optical selected,channel=7
	
	I2C_write(0x20,0x05,0xc5);      //// LJ master mode; SOJUST=0, SODEL=0, SOSPOL=0, SOLRPOL=0
	I2C_write(0x20,0x06,0x10);      //// UNLOCK unmasked in RERR register
	
	I2C_write(0x20,0x07,0x04);        // RERR register unmasked for interrupt
	I2C_write(0x20,0x08,0x7F);        // all interrupts level-active
	I2C_write(0x20,0x09,0x00);        // all interrupts level-active
	I2C_write(0x20,0x04,0xb8);        // RUN=1 / optical
}
main()
{
	uint8_t data,ldata;
	uint32_t i;
	init_basic();
	init_i2c();
#ifdef DEBUG
	LCD1602_init();
	LCD1602_printl(0,0,"Booting...");
#endif
	data = FLASH_ReadByte(0x004000);
	ldata=data;
	init_BD3490(data);
	init_cs8416();

	while (1){
		while (GPIO_ReadInputPin(GPIOA, GPIO_PIN_3) == RESET)  //SET or RESET
		{
			data--;
			if(data<0x80)data=0x80;
			I2C_write(0x80, 0x21, data);
			I2C_write(0x80, 0x22, data);
			i = sEE_FLAG_TIMEOUT;
			while(i--);
		}

		while (GPIO_ReadInputPin(GPIOD, GPIO_PIN_6) == RESET)  //SET or RESET
		{
			data++;
			if(data>0xFE)data=0xFF;
			I2C_write(0x80, 0x21, data);
			I2C_write(0x80, 0x22, data);
			i = sEE_FLAG_TIMEOUT;
			while(i--);
		}
		
		if(ldata!=data){
			FLASH_Unlock(FLASH_MEMTYPE_DATA); //解锁 EEPROM
			FLASH_ProgramByte(0x004000, data);
			FLASH_Lock(FLASH_MEMTYPE_DATA);
			ldata=data;
		}		
		
	}	
}
