//утилиты  для работы с USART с помощью буфера( формирпование и вывод строки)

#include <pic18.h>
#include "usart.h" 
#include "name_konst.h"
#include "usart_buf.h"

extern unsigned char buf_rx_command[];

#define MAX_STR_Buf MAX_BIG_BUF_STR   //длинна буфера для вормирования длинныз сообщений SMS и т.д.
unsigned char STR_Buf[MAX_STR_Buf];
unsigned int point_buf;					//счетчик позиции в буфере

//-----
void putst_buf(char *str)		//послать строку из буфера
{
	check_err();
	while((*str)!=0)
	{
    	if (*str==0xFF) {break;}
		putch(*str);
		str++;
	}
}

//----------------------------
void wr_strbuf(register const char *str, char point_n)				//запись строки в буфер если 0 сначала нет продолжить
{
if (!point_n) point_buf=0;
	while((*str)!=0)
	{
		if (!(point_buf<MAX_STR_Buf)){STR_Buf[point_buf-1]=0;return;}   //если конец буфера то выставить признак конца и выйти
		STR_Buf[point_buf]=*str;
		point_buf++;
		str++;
	}
		STR_Buf[point_buf]=0;							//дописать признак конца строки
}
//----------------------------------------------------

void wr_strbuf_buf( char *str, char point_n)			//запись строки в буфер из буфера если 0 сначала нет продолжить
{
if (!point_n) point_buf=0;
	while((*str)!=0)
	{
		if (!(point_buf<MAX_STR_Buf)){STR_Buf[point_buf-1]=0;return;}   //если конец буфера то выставить признак конца и выйти
		STR_Buf[point_buf]=*str;
		point_buf++;
		str++;
	}
		STR_Buf[point_buf]=0;							//дописать признак конца строки
}
//----------------------------------------------------
void wr_ptbuf(unsigned char c, char point_n)						//запись байта в буфер если 0 сначала нет продолжить
{
if (!point_n) point_buf=0;
		if (!(point_buf<MAX_STR_Buf)){STR_Buf[point_buf-1]=0;return;}   //если конец буфера то выставить признак конца и выйти
		STR_Buf[point_buf]=c;
		point_buf++;
		STR_Buf[point_buf]=0;							//дописать признак конца строки
}
//-----------
void wr_ptbuf_dec(unsigned char c, char point_n)
{
	unsigned char temp;
	if (!point_n) point_buf=0;
	temp=c;
	//hundreds
	if ((c/100)>0) wr_ptbuf((c/100)+'0',1);
	c-=(c/100)*100;

	//tens
	if (((temp/10)>0) || ((temp/100)>0)) wr_ptbuf((c/10)+'0',1);
	c-=(c/10)*10;

	//ones
	wr_ptbuf((c/1)+'0',1);
}

//-----------
void wr_ptbuf_long(unsigned long c, char point_n)
{
	static unsigned char st_razr;
	unsigned char temp;
	unsigned long  delit;
	if (!c) {wr_ptbuf('0',1);return;}
	st_razr=0;
	for (delit=1000000000L;delit>=1;delit/=10)
		{	
			if(((c/delit)>0)||st_razr) 
					{
					temp=(c/delit)+'0';
					st_razr=1;
					wr_ptbuf(temp,1);
					}
			c-=(c/delit)*delit;
			CLRWDT();
		}
}
//-------------------------------------------------
unsigned char find_N_char(unsigned char *massive,unsigned char simvol,unsigned char n_cnt,unsigned int MAX_STR) //Поиск N символа в массиве
{
unsigned int ukaz;
ukaz=0;
	if (*massive==simvol){ --n_cnt;}
	while(n_cnt)
	{
	ukaz++;	
	massive++;
	if(simvol)	{if((ukaz>=MAX_STR)||(!(*massive))) {return 0xFF;}}
		else 	{if(ukaz>=MAX_STR) {return 0xFF;}}
		
	if (*massive==simvol){ --n_cnt;continue;}
	}
return ukaz;
}
//--------------------------------------------
void set_ukaz_zap(char*ukz,char zap)		// установка указателя на запятую по счету
{
//char i;
*ukz=0;

do
	{
	++*ukz;
	if(*ukz>=MAX_BUF_RX_COM){*ukz=0;break;}
	if(buf_rx_command[*ukz]==0x2C) {zap--;}		//запятая
	}while (zap);
}
//----------------------------------------
unsigned char compare_buf(char *bufer1,char *bufer2,char end_symb,char max_buf2)	// сравнить буфер 2 с буфером 1 до 0 или end_symb
{
char i;
for (i=0;i<max_buf2;i++)
	{
		if ((!*bufer2)&&(!*bufer1)) return 1;   // нашли нули - конец 
		if  (*bufer2==end_symb)		return 1;	// признак конца
		if (*bufer2!=*bufer1) return 0;
		bufer1++;
		bufer2++;	
	}
return 0;
}

//---------------------------
unsigned char compare_str_buf(register const char *str,unsigned char *bufer)		//сравнить строку( комманду) c содержимым буфера до конца строки
{
	while((*str)!=0)
	{
    if ((*str)!=*bufer) return 0;	// не совпала комманда
		bufer++;
		str++;
	}
return 1;		// полное совпадение
}
//---------------------------
char copy_buf(char *bufer1,char *bufer2,char end_symb,char max_buf2)	// скопировать буфер 2 в буфер 1 до 0 или end_symb вконце дописуется 0
			//результат указатель на последний байт
{
char i;
for (i=0;i<max_buf2;i++)
	{
		*bufer1=*bufer2;
		if (!*bufer2) {return i;}   // нашли нули - конец 
		if  (*bufer2==end_symb)	{return i;}	// признак конца
		bufer1++;
		bufer2++;	
	}
return 0;
}

//---------------------------------
unsigned long calk_parametr_buf(unsigned char  *bufer,unsigned char end_symb)		//вычисление параметра из строки значение вычисляется от переданного указателя с лево на право до end_symb
{
unsigned char *bufer_st,i;
unsigned long mnog,work_long;
bufer_st=bufer-1;		// запомнить значение
for(i=0;i<11;i++)	// максимальное значение колличества цифр
	{
	if (*bufer==end_symb) break;			// найти end_symb
	if (*bufer==0x00) break;			// найти end_symb
	bufer++;
	}
	if (i==11) {return 0;}		// защита от ошибки
	bufer--;  		//коррекция
work_long=0;
mnog=1;						// множитель разряда
while (bufer!=bufer_st)
	{
	work_long+=(*bufer-'0')*mnog;	
	mnog*=10;
	bufer--;
	}
return work_long;
}