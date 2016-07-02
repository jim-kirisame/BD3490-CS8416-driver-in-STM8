#ifndef __LCD1602__
#define __LCD1602__

#define LCD1602_CLS 0x01			//screen clear
#define LCD1602_RSTPOS 0x02		//reset cursor posistion

extern void LCD1602_writeCommand(unsigned char command);
extern void LCD1602_writeText(unsigned char text);
extern void LCD1602_init(void);
extern void LCD1602_print(int x, int y, unsigned char text);
extern void LCD1602_printl(int x, int y, char *data);


#endif