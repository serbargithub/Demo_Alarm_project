#include <pic18.h>
#include "name_port.h"
#include "my_delay.h"
//#include "I2C_sys.h"
#include "name_konst.h"
#include "name_adres.h"
#include "usart.h" 

extern unsigned char cnt_clk,random;
extern unsigned char flag_param;
extern unsigned int cnt_secund;



//+++++++++++++++++++++++++++++++++++
#define DS_read(CH_DL)   if(CH_DL==1){tris_sda|=mask_dallas1_in;};if(CH_DL==2){tris_sda|=mask_dallas2_in;};
#define DS_send(CH_DL)   if(CH_DL==1){tris_sda&=mask_dallas1_out;};if(CH_DL==2){tris_sda&=mask_dallas2_out;};
#define ms_dallas_out(set,CH_DL) 	if(CH_DL==1){ms_dallas1_out=set;}if(CH_DL==2){ms_dallas2_out=set;}
//------------------
void delay_ds(unsigned int t_wait)						//задержка  шаг 3.8 мкс 
{
unsigned int i;
t_wait*=2;
for (i=0;i<=t_wait;i++)
	{
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	}
}
//------------------------
unsigned char wait_zero (unsigned char t_wait,unsigned char CH_DL )			//задержка с поиском 0 шаг 2 мкс для канала 1 и 2
{
unsigned char i;
for (i=0;i<=t_wait;i++)
	{
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	if(CH_DL==1){if (!ms_dallas1_in) return 1;}
	if(CH_DL==2){if (!ms_dallas2_in) return 1;}
	}
return 0;
}

unsigned char wait_one (unsigned char t_wait,unsigned char CH_DL )			//задержка с поиском 1 шаг 2 мкс 
{
unsigned char i;
for (i=0;i<=t_wait;i++)
	{
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	if(CH_DL==1){if (ms_dallas1_in) return 1;}
	if(CH_DL==2){if (ms_dallas2_in) return 1;}
	}
return 0;
}
//-----------------------
unsigned char DS_init(unsigned char CH_DL)
{
CLRWDT();
GIE=0;				//выключить прерывания
ms_dallas_out(CH_DL,1);
DS_send(CH_DL);
delay_ds(20);
ms_dallas_out(CH_DL,0);
delay_ds(200);		//1 мс
DS_read(CH_DL)
if(!wait_one(100,CH_DL))  {GIE=1; return 0;}
if(!wait_zero(100,CH_DL)) {GIE=1; return 0;}
delay_ds(20);
if(!wait_one(250,CH_DL))  {GIE=1; return 0;}
GIE=1;				//запустить прерывания
return 1;
}
//----------------------
void DS_outbyte(unsigned char byte,unsigned char CH_DL)
{
unsigned char i;
CLRWDT();
	DS_send(CH_DL);
	ms_dallas_out(CH_DL,1);
	delay_ds(1);
for(i=0;i<8;i++)
	{
		ms_dallas_out(CH_DL,0);
		if(byte&0x01){	delay_ds(4);ms_dallas_out(CH_DL,1);delay_ds(8);}
			else {delay_ds(13);}
		ms_dallas_out(CH_DL,1);
		byte>>=1;
		delay_ds(1);
	}
return;
}
//----------------------
unsigned char DS_readbyte(unsigned char CH_DL )
{
unsigned char byte=0,i;
CLRWDT();
for(i=0;i<8;i++)
	{
		byte>>=1;
		DS_send(CH_DL);
		ms_dallas_out(CH_DL,0);
		delay_ds(1);
		DS_read(CH_DL);
		delay_ds(2);
		if(CH_DL==1){if(ms_dallas1_in){byte|=0x80;}else {byte&=0x7F;}}
		if(CH_DL==2){if(ms_dallas2_in){byte|=0x80;}else {byte&=0x7F;}}
		delay_ds(12);
	}
DS_read(CH_DL);
return byte;
}

//--------------------------
unsigned char read_temp_dallas (unsigned char CH_DL )
{
unsigned char rd_byte1,rd_byte2;

if(!DS_init(CH_DL)) return 0x7F;		//если датчик не обнаружен - ошибка
	GIE=0;				//выключить прерывания
	DS_outbyte(0xCC,CH_DL);
	DS_outbyte(0x44,CH_DL);				// запустить преобразование
	ms_dallas_out(CH_DL,1);
	GIE=1;				//запустить прерывания
	delay_s(1);						// поддержка на время преобразования
if(!DS_init(CH_DL)) return 0x7F;		//если датчик не обнаружен - ошибка
	GIE=0;				//выключить прерывания
	DS_outbyte(0xCC,CH_DL);
	DS_outbyte(0xBE,CH_DL);				// запрос температуры
	delay_ds(10);
	rd_byte1=DS_readbyte(CH_DL);
	rd_byte2=DS_readbyte(CH_DL);
	DS_read(CH_DL);
	GIE=1;				//запустить прерывания
	rd_byte1>>=4;
	rd_byte1&=0x0F;
	rd_byte2<<=4;
	rd_byte2&=0xF0;
	rd_byte2|=rd_byte1;
return rd_byte2;
}