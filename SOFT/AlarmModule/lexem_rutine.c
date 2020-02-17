#include <pic18.h> 
#include "name_konst.h"
#include "usart.h" 
#include "name_port.h"
#include "lexem_rutine.h"

extern unsigned char SW_uart;		//переключатель рабочего COM порта
extern unsigned char buf_rx_command[],buffer_rx[], buffer_rx2[];
extern unsigned char end_bufrx,st_bufrx;
extern unsigned char end_bufrx2,st_bufrx2;
extern unsigned char st_bufRxblth,end_bufRxblth;
extern unsigned char cnt_clk,count_inbuf_file,count_inbuf_file_end;
extern  unsigned int cnt_secund;
//------------------------------------------------
char load_lexem(char time_wait)		// ожидание лексемы в течении т секунд, возвращается указатель на последнюю букву(дописаный ноль)
{
char temp,count_ukaz;
int stop_time;
count_ukaz=0;
stop_time=cnt_secund+time_wait;
				//
 do{
CLRWDT();

if(SW_uart==0){if (end_bufrx==st_bufrx){continue;}}// нет комманд выход
if(SW_uart==1){if (end_bufrx2==st_bufrx2){continue;}}
if(SW_uart==Bluetooth_CH){if (end_bufRxblth==st_bufRxblth){continue;}}
 temp=getch_us(SW_uart);
if (!temp) continue;
if (!count_ukaz) 
		{if ((temp=='\r')||(temp=='\n')||(!temp)) {continue;}
			else {buf_rx_command[0]=temp;count_ukaz++;}
		}
	else{
		buf_rx_command[count_ukaz]=temp;
		if((temp=='\r')||(temp=='\n')){buf_rx_command[count_ukaz]=0;break;}		// прием лексемы закончен
		if(count_ukaz>=(MAX_BUF_RX_COM-1)){count_ukaz=MAX_BUF_RX_COM-1;buf_rx_command[count_ukaz]=0;continue;}
		++count_ukaz;
		}
  }while(cnt_secund<=stop_time);
if (cnt_secund<=stop_time) {return count_ukaz;}
return 0;		// если время вышло- возврват с ошибкой
}
//---------------------------------------
char read_lexem	(char n_lex,char time)			// ожидание лексемы в течении т секунд, n лексем возвращается указатель на последнюю букву(дописаный ноль)
{
char temp=0;
clear_buf_RX(SW_uart);
while (n_lex)
{
temp=load_lexem(time);
n_lex--;
if(!temp) break;	// если не прочитана первая- не зачем ждать
}
return temp;
}
//-----------------------------------
void copy_lexem(char *buf1,char *buf2)		// скопировать лексему из буфера 2 в буфер 1
{
	while((*buf2)!=0)
	{
		*buf1=*buf2;
		buf1++;
		buf2++;
	}
		*buf1=0;				// дописать 0
}

//**********************************************************************
