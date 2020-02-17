#include <pic18.h>
#include "my_delay.h"
#include "name_konst.h"

extern unsigned char SW_uart;		//переключатель рабочего COM порта
extern unsigned char st_bufrx,end_bufrx;
extern unsigned char st_bufrx2,end_bufrx2;
extern unsigned char st_bufRxblth,end_bufRxblth,count_Txblth;
extern unsigned char fl_cary;			//счетчик из из прерывания

extern unsigned char cnt_wt;	//счетчик из из прерывания	
extern  unsigned int cnt_secund;
extern unsigned char buffer_rx[];
extern unsigned char buffer_rx2[];
extern unsigned char buffer_Rxblth[];
extern unsigned char buffer_Txblth[];

#define sp2400	0
#define sp4800	1
#define sp9600	2
#define sp19200	3
#define sp38400	4
#define sp57600	5
#define sp115200 6

#define PRI_US	0		//порт 1 usart
#define SEC_US	1		//порт 2 usart



void serial_setup(unsigned char ind_speed,unsigned char num_usart)	  // посчитано на 32 мгц
{
if(!num_usart)
	{
  	switch (ind_speed)
	{

	case 1:				// 4800		// посчитано на 40мгц
		SPBRG=207;		//103
		SPBRGH=0;
		BRGH1=0;
		BRG161=0;
		break;
	case 2:				//9600
		SPBRG=103;		//51
		SPBRGH=0;
		BRGH1=0;
		BRG161=0;
		break;
	case 5:				//57600
		SPBRG=21;		//138
		SPBRGH=1;
		BRGH1=1;
		BRG161=1;
		break;
	case 6:				//115200
		SPBRG=138;		//68
		SPBRGH=0;
		BRGH1=1;
		BRG161=1;
		break;
	default:
		break;
	}	
	SYNC1=0;						//asynchronous
	SPEN1=1;						//enable serial port pins
	CREN1=1;						//enable reception
	SREN1=0;						//no effect
	TX1IE=0;						//disable tx interrupts
	TX91=0;						//8-bit transmission
	RX91=0;						//8-bit reception
	TXEN1=0;						//reset transmitter
	TXEN1=1;						//enable the transmitter
	}
	else
	{
  	switch (ind_speed)
	{

	case 1:				// 4800		// посчитано на 40мгц
		SPBRG2=207;
		SPBRGH2=0;
		BRGH2=0;
		BRG162=0;
		break;
	case 2:				//9600
		SPBRG2=103;
		SPBRGH2=0;
		BRGH2=0;
		BRG162=0;
		break;
	case 5:				//57600
		SPBRG2=21;
		SPBRGH2=1;
		BRGH2=1;
		BRG162=1;
		break;
	case 6:				//115200
		SPBRG2=138;
		SPBRGH2=0;
		BRGH2=1;
		BRG162=1;
		break;
	default:
		break;
	}	
	SYNC2=0;						//asynchronous
	SPEN2=1;						//enable serial port pins
	CREN2=1;						//enable reception
	SREN2=0;						//no effect
	TX2IE=0;						//disable tx interrupts
	TX92=0;						//8-bit transmission
	RX92=0;						//8-bit reception
	TXEN2=0;						//reset transmitter
	TXEN2=1;						//enable the transmitter
	}
}

//=====================-==
unsigned char check_err(void)					//контроль- ликвидация ошибки
 {
	unsigned char work;
  static unsigned char fl_error=0;

	if (FERR1)	
	{	
		work=RCREG1;
		TXEN1=0;											
		TXEN1=1;	
		FERR1=0; 
		work=RCREG1;
		fl_error=1;
	}
	if (OERR1)
		{
			TXEN1=0;													
			TXEN1=1;	
			CREN1=0;
			CREN1=1;
			work=RCREG1;
			work=RCREG1;
			work=RCREG1;
		}
	if (FERR2)	
	{	
		work=RCREG2;
		TXEN2=0;											
		TXEN2=1;	
		FERR2=0; 
		work=RCREG2;
		fl_error=1;
	}
	if (OERR2)
		{
			TXEN2=0;													
			TXEN2=1;	
			CREN2=0;
			CREN2=1;
			work=RCREG2;
			work=RCREG2;
			work=RCREG2;
		}
 return 1;
 }

//========================
void clear_buf_RX(unsigned char num_usart)   // очистить побайтовый буфер
{
switch (num_usart)
	{
	case 0: st_bufrx=0;end_bufrx=0;break;
	case 1: st_bufrx2=0;end_bufrx2=0;break;	
	case 2: st_bufRxblth=0;end_bufRxblth=0;break;	
	}
}
//---------------------------
unsigned int read_buf_RX(unsigned char num_usart) //ask_geth_byte прочитать байт из побайтового буфера возвращает 0 если не принят байт и 01XX если  принят, байт сохраняется в глобальных переменных
{
unsigned int recived_byte=0;
switch (num_usart)
{
//ask_geth_byte
	case 0: if (end_bufrx!=st_bufrx){++st_bufrx;if(st_bufrx>=MAX_BUF_UART) {st_bufrx=0;};recived_byte=0x0100+buffer_rx[st_bufrx];}
			break;
//ask_geth_byte2
	case 1: if (end_bufrx2!=st_bufrx2){++st_bufrx2;if(st_bufrx2>=MAX_BUF_UART) {st_bufrx2=0;};recived_byte=0x0100+buffer_rx2[st_bufrx2];}
			break;
//ask_geth_byteRxblth
	case 2: if (end_bufRxblth!=st_bufRxblth) {++st_bufRxblth;if(st_bufRxblth>=MAX_BUF_BLTH) {st_bufRxblth=0;};recived_byte=0x0100+buffer_Rxblth[st_bufRxblth];}
			break;
	default:
			break;
}
return recived_byte;
}
//--------------------------------

unsigned char getch_n(unsigned char cnt_byte,unsigned char num_usart,unsigned char *fl_read_error) //чтение цепочки байт результат-последний прочитанный
 {
	unsigned char prom;
	unsigned int getch_byte;
	*fl_read_error=0;
	do
   {
		prom=200; // задержка ожидания байта 200мс
		while (1)
		{	 
		 CLRWDT();
		 getch_byte=read_buf_RX(num_usart);
		 if(getch_byte)break;
		 if (fl_cary){cnt_wt=255; fl_cary=0;if (!prom--) {*fl_read_error=1; return 0xFF;}}
	    };
	 }while(--cnt_byte);
 return (unsigned char)(0x00FF&getch_byte);
 }
//=====================================
unsigned char getch_us(unsigned char num_usart)	//принять байт 
{
	return (unsigned char)(0x00FF&read_buf_RX(num_usart));
}
//========================

void putch(unsigned char c)	//послать 1 байт (для print)
{
// unsigned char work;
//	check_err();
switch (SW_uart)
	{case 0: while(!TX1IF){CLRWDT();}TXREG1=c;break;			//дождаться освобождения буфера
	 case 1: while(!TX2IF){CLRWDT();}TXREG2=c;break;			//дождаться освобождения буфера
	 case 2: if(count_Txblth>=MAX_BUF_BLTH){count_Txblth=(MAX_BUF_BLTH-1);};buffer_Txblth[count_Txblth]=c;count_Txblth++; break;			//записать в буфер отправки Блютуз	
	}
//	delay_ms(1);
}
//==========================

void putst(register const char *str)		//послать строку
{
//unsigned char wrk=0;
	check_err();
	while((*str)!=0)
	{
		putch(*str);
    if (*str==13) {putch(10);}
    if (*str==10) {putch(13);}
		str++;
	}
}

//----------------------
void putst_noenter(register const char *str)		//послать строку
{
//unsigned char wrk=0;
	check_err();
	while((*str)!=0)
	{
		putch(*str);
		str++;
	}
}
//=====================

unsigned char put_command(register const char *str)
{
unsigned char fl_read_error;
unsigned char wrk=0,count=0;
	check_err();
	clear_buf_RX(SW_uart);
	while((*str)!=0)
	{
		putch(*str);
		str++;
		count++;
	}
	count+=1;
	while (count--)	
	{	wrk=getch_n(1,SW_uart,&fl_read_error);
		if(wrk==0x0d) {return 0;}	
		if(wrk==0xFF) {return 1;}
	}
	return 1;
}


//====================================


void putchhex(unsigned char c)
{
	unsigned char temp;
	temp=c;

	c=(c >> 4);
	if (c<10) c+=48; else c+=55;
	putch(c);

	c=temp;

	c=(c & 0x0F);
	if (c<10) c+=48; else c+=55;
	putch(c);
}
void putchdec(unsigned char c)
{
	unsigned char temp;

	temp=c;
	//hundreds
	if ((c/100)>0) putch((c/100)+'0');
	c-=(c/100)*100;

	//tens
	if (((temp/10)>0) || ((temp/100)>0)) putch((c/10)+'0');
	c-=(c/10)*10;

	//ones
	putch((c/1)+'0');
}


void putch_long(unsigned long c)		// отображение long числа
{
	static unsigned char st_razr;
	unsigned char temp;
	unsigned long  delit;
	if (!c) {putch('0');return;}
	st_razr=0;
	for (delit=1000000000L;delit>=1;delit/=10)
		{	
			if(((c/delit)>0)||st_razr) 
					{
					temp=(c/delit)+'0';
					st_razr=1;
					putch(temp);
					}
			c-=(c/delit)*delit;
			CLRWDT();
		}
}

//------------------------------------
void monitor_hex(char temp)
{
putst("at+Monitor_HEX :");putchhex(temp);putst("\r\n");
delay_s(2);

}
//---------------------------------------

unsigned char check_OK(unsigned int time)							//ожидание ок в течении заданного времени в сек
{
unsigned char fl_read_error,temp;
	check_err();
	clear_buf_RX(SW_uart);						//очистить буфер
	time+=cnt_secund;
	if(cnt_secund>time){time-=cnt_secund; cnt_secund=0;}
	while(1)
	{
	temp=getch_n(1,SW_uart,&fl_read_error); 
	if(temp=='O')
		{
		temp=getch_n(1,SW_uart,&fl_read_error); 
		if (temp=='K') return 1;
		}
	if (cnt_secund>=time) return 0;
	}
	return 1;
}

