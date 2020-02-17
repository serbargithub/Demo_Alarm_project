 #include <pic18.h> 
#include "name_port.h"

extern unsigned char cnt_clk,trub_on;


//===============================
void delay_ms(unsigned int wait)		//миллисекунды
{
	do
	{
	cnt_clk=16;			//1ms
	while (cnt_clk){CLRWDT();}
	}while (--wait);
}			
	
void delay_s(unsigned char wait)	// секунды
	{
	unsigned char i;
	while (wait)
	{
	for (i=0;i<=125;i++)
		{
		cnt_clk=128;			//1s
		while (cnt_clk){CLRWDT();}
		}
	wait--;
	}
	}
void delay_mks(unsigned char wait)		//микросекунды
{
while(wait--){asm("nop");}
}
//=====================	
void delay_ms_dso(unsigned int wait)		//миллисекунды c контролем
	{
	do
	{
	cnt_clk=16;			//1ms
	while (cnt_clk){CLRWDT();if(dso_vg)return;}
	}while (--wait);
	}		
//-----
void delay_s_dso(unsigned char wait)	// секунды
	{
	unsigned char i;
	do
	{
	for (i=0;i<=125;i++)
		{
		cnt_clk=128;			//1s
		while (cnt_clk){CLRWDT();if(dso_vg)return;}
		}
	}while (--wait);
	}
//---------------------------------
void delay_s_dso_tel(unsigned char wait)	// секунды
	{
	unsigned char i;
	do
	{
	for (i=0;i<=125;i++)
		{
		cnt_clk=128;			//1s
		while (cnt_clk){CLRWDT();if(dso_vg||trub_on)return;}
		}
	}while (--wait);
	}
