#include <pic18.h>
#include "ctype.h"
#include "usart.h" 
#include "usart_buf.h" 
#include "my_spi_SST26.h"
#include "my_delay.h"
#include "function.h"
#include "lexem_rutine.h"
#include "name_konst.h"
#include "name_adres.h"
#include "name_port.h"
#include "eeprom.h"
#include "terminal.h"

extern unsigned char terminal_echo_ON;
extern  unsigned char led_inform,fl_ring_yes,SW_uart;
extern unsigned char st_bufRxblth,end_bufRxblth,count_Txblth,fl_bt_DISCONN_yes;
extern unsigned char buf_rx_command[];
extern unsigned char buffer_Txblth[];
#define echo_ON   1
#define echo_OFF  0
//----------------------------------------------------------------

void Bluetooth_Connecting (void)
{
unsigned char fl_read_error,i,work;
	put_command("AT+BTACPT=1\r");			// включить Bluetooth
	read_lexem(1,1);
	led_inform=5;					// индицировать подключение по Bluetooth
	SW_uart=Bluetooth_CH;
	clear_buf_RX(SW_uart);
	count_Txblth=0;
	fl_bt_DISCONN_yes=0;
while (1)					// 
{
	SW_uart=Bluetooth_CH;
	if(fl_ring_yes)  {SW_uart=mod_GSM;put_command("ATH\r");delay_s(2);fl_ring_yes=0;SW_uart=Bluetooth_CH;}  // если звонок то отбить его
	work=load_command_terminal(echo_OFF);   // терминал без эха
	if (work)
			{
			for(i=0;i<=work;i++) 
				{buf_rx_command[i]=(char)toupper(buf_rx_command[i]);if(buf_rx_command[i]=='=')break;}	// привести к верхнему регистру

			detect_command_terminal(work,1);			//поиск комманды на at начинающейся+ доступ к модулю

				if(count_Txblth)						// если в буфере отправки bluetooth что то есть то передать
					{
					SW_uart=mod_GSM;
					put_command("AT+BTSPPSEND\r");getch_n(2,SW_uart,&fl_read_error);
					for(i=0;i<count_Txblth;i++) {putch(buffer_Txblth[i]);}
					putch(0x1A);			//cntrol Z
					if(!check_OK(2))break;			//2 секунд ждать OK
					count_Txblth=0;
					SW_uart=Bluetooth_CH;
					}
			}
	check_err();
	CLRWDT();
		if(!sys_key){delay_ms(500);if(!sys_key) {while(!sys_key){led_inform=3;delay_ms(100);}break;}}//проверить кнопку если нажата возврат
		if(fl_bt_DISCONN_yes){ break;} //если соединение nluetooth отвалилось то выйти

 }
SW_uart=mod_GSM;
}
//-------------------------------------------------------------
