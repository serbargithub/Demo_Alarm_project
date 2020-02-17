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
#include "SMS_rutine.h"

extern unsigned char fl_bridge_to_RX,SW_uart;		// SW_uart переключатель рабочего COM порта
unsigned char terminal_echo_ON;
unsigned char buf_rx_command[MAX_BUF_RX_COM];   // буфер хранения принятых команд
unsigned char work_string[MAX_BUF_RX_COM];
extern unsigned char end_bufrx,st_bufrx;
extern unsigned char st_bufrx2,end_bufrx2;
extern unsigned char st_bufRxblth,end_bufRxblth,count_Txblth;
extern unsigned int cnt_secund;
extern  unsigned char led_inform;
extern unsigned char opros_dat_enable,fl_sec_code,fl_no_carier_yes,fl_no_carier;
void detect_command_terminal(char ukaz,char);
char detect_command_programmator(char ukaz);
unsigned char	multi_write(unsigned char);
unsigned char	multi_read(unsigned char);
//--------------------------------------------------------------

unsigned char load_command_terminal(unsigned char echo_answer)		// приемник комманд  начинающихся с АТ результат - указатель на последий байт строки  
																	// параметр 1 эхо включено 0 выключено
{
char temp;
static char count_ukaz; 
static unsigned char start_ld_comm;
 CLRWDT();
if(SW_uart==0){if (end_bufrx==st_bufrx){ return 0; }}// нет комманд выход
if(SW_uart==1){if (end_bufrx2==st_bufrx2){ return 0;}}
if(SW_uart==Bluetooth_CH){if (end_bufRxblth==st_bufRxblth){ return 0;}}
 temp=getch_us(SW_uart);
if(!temp) return 0;
if (!start_ld_comm) {count_ukaz=0; if(!((temp=='A')||(temp=='a'))) { return 0;}}
	start_ld_comm=1;
  	buf_rx_command[count_ukaz]=temp;
if ((count_ukaz==1)&&(!((temp=='T')||(temp=='t')))){start_ld_comm=0;return 0;} //если не AT
if(echo_answer){	putch(temp);}			//ехо  
	if (temp=='\b') {if(echo_answer){putst(" \b");}count_ukaz--; return 0;}
	if ((temp==0x0d)||(temp==0x0a))
			{
			  start_ld_comm=0;
				if(echo_answer)	// дополнить возвратом
				{if (temp==0x0d){putch(0x0a);}
					else {putch(0x0d);}
				}	
				buf_rx_command[count_ukaz+1]=0;// признак конца
			 return	count_ukaz;
			}
	if (count_ukaz<(MAX_BUF_RX_COM-2)) count_ukaz++;
	return 0;
}
//---------

unsigned char uncode_com(register const char *str)		//сравнить комманду
{
char ukaz=0;
	while((*str)!=0)
	{
    if ((*str)!=buf_rx_command[ukaz]) {return 0;}	// не совпала комманда
		ukaz++;
		str++;
	}
return 1;		// полное совпадение
}

//--------------------------------------------------------------
unsigned long calk_parametr(char ukaz)					//вычисление параметра из строки ( аргумент последий байт строки 0xOD)
{
unsigned long mnog,work_long;
ukaz--;			//коррекция
work_long=0;
mnog=1;						// множитель разряда
while ((buf_rx_command[ukaz]!='=')&&(buf_rx_command[ukaz]!=',')&&(buf_rx_command[ukaz]!=0x20))
	{
	work_long+=(buf_rx_command[ukaz]-'0')*mnog;	
	mnog*=10;
	if (!(ukaz--)) {return 0;}		// защита от ошибки
	}
return work_long;
}
//----------------------------------
void save_parametr(unsigned long Adres)
{
char ukaz_tek,ukaz;
		ukaz_tek=find_N_char(buf_rx_command,'=',1,MAX_BUF_RX_COM);	// найти =
		ukaz_tek++;
		ukaz=copy_buf(Bufer_SPI,&buf_rx_command[ukaz_tek],0x0D,MAX_BUF_RX_COM);//скопировать по 0D
		Bufer_SPI[ukaz]=0;		//признак конца
		Frame_Buf_Write(Adres,ukaz+1);
		putst("\rWrite OK\r");

}
//------------------------
void read_parametr(unsigned long Adres)
{
		Frame_Buf_Read(Adres,0);					// прочитать до нуля
		putst("\r");putst_buf(Bufer_SPI);putst("\r");

}
//------------------------
void print_parametr(unsigned long Adres)
{
		Frame_Buf_Read(Adres,0);					// прочитать до нуля
		putst_buf(Bufer_SPI);

}
//------------------------
void change_uart_init(char napravl)
{
if (napravl==mod_GSM)
	{
	GIE=0;
	T2CON=0b00011000;	//пост дел/4
	TMR2IF=0;
	TMR2ON=1;
	GIE=1;
	ch_usart(mod_GSM);
	serial_setup(sp115200,PRI_US);		//проинициализировать usart
	}
if (napravl==komputer)
	{
	GIE=0;
	T2CON=0b00001000;	//пост дел/2
	TMR2IF=0;
	TMR2ON=1;
	GIE=1;
	ch_usart(komputer);
	serial_setup(sp115200,SEC_US);		//проинициализировать usart
	}
if (napravl==Bluetooth_CH)
	{
	GIE=0;
	T2CON=0b00011000;	//пост дел/4
	TMR2IF=0;
	TMR2ON=1;
	GIE=1;
	ch_usart(mod_GSM);
	serial_setup(sp115200,PRI_US);		//проинициализировать usart
	ch_usart(Bluetooth_CH);
	}
}

//======================================== Детектор комманд
void terminal_on(void)
{
char work,i;
//if(!in_tel)	return;
opros_dat_enable=0;
led_inform=0;delay_s(1);
while(!sys_key){led_inform=3;delay_ms(100);}
change_uart_init(komputer);

putst("\rSystem Ready\r");
led_inform=5;					// индицировать компьютер на светодиод не выводить
terminal_echo_ON=1;				// включить эхо 
clear_buf_RX(SW_uart);
while (1)					// пока лежит трубка
{
	work=load_command_terminal(terminal_echo_ON);
	if (work)for(i=0;i<=work;i++) 
			{buf_rx_command[i]=(char)toupper(buf_rx_command[i]);
			if(buf_rx_command[i]=='=')break;	// привести к верхнему регистру
			}
	if(work){
			detect_command_terminal(work,0);			//поиск комманды на at начинающейся
			}
	check_err();
	CLRWDT();
		if(!sys_key)
				{
				 delay_ms(500);
				if(!sys_key) 
					{
					while(!sys_key){led_inform=3;delay_ms(100);}
					break;	
					}
				}
}
putst("\rSystem Close\r");

change_uart_init(mod_GSM);

opros_dat_enable=1;
}

//---------------------------------
void terminal_PDP(void)
{
char work,i;
fl_no_carier_yes=0;
fl_no_carier=1;
putst("\rSystem Ready\r");
led_inform=5;					// индицировать компьютер на светодиод не выводить
terminal_echo_ON=1;				// включить эхо 
cnt_secund=0;
while (!fl_no_carier_yes)					// пока лежит трубка
{
	work=load_command_terminal(terminal_echo_ON);
	if (work)for(i=0;i<=work;i++) 
			{buf_rx_command[i]=(char)toupper(buf_rx_command[i]);
			if(buf_rx_command[i]=='=')break;	// привести к верхнему регистру
			}
	if(work) detect_command_terminal(work,1);			//поиск комманды на at начинающейся
	check_err();
	CLRWDT();
	if (cnt_secund>300)			// 5 минут не больше
		{
		putst("\rSystem Close\r"); 
		putst("+++");delay_s(1);putst("\rATH\r");	// время вышло
		break;
		}
}
fl_no_carier=0;
opros_dat_enable=1;
}

//-----------------------------------
void detect_command_terminal(char ukaz,char marker_viz)
{
unsigned char fl_read_error,ukaz_tek,wrk_char,i,SW_save;
unsigned long wrk_long;
unsigned int wrk_int;
static unsigned long set_adr;
static unsigned int set_adr_eep;
if (uncode_com("AT\r")) 	{goto	end_ok;	} 
if (uncode_com("ATI1")) 				//чтение версии ПО
		{
		putst(Ver_Build);	//впечатать версию
		putst(" Build: ");putst(time_build);
		putst(" ");putst(date_build);	//впечатать версию 
		goto	end_ok;
		}  
if (uncode_com("ATI")) 				//чтение идентификатора устройства
		{putst(Ver_Signal);goto	end_ok;} //впечатать версию 
if (uncode_com("ATE0")) 				//отключить эхо 
		{terminal_echo_ON=0;goto	end_ok;}  
if (uncode_com("ATE1")) 				//включить эхо 
		{terminal_echo_ON=1;goto	end_ok;}  

if (uncode_com("AT+WR=")) 					//комманда записи мультипараметра
		{
		multi_write(0);
		return;
		} 
if (uncode_com("AT+RD=")) 					//комманда чтения мультипараметра
		{
		multi_read(0);
		return;
		}
if (uncode_com("AT+ADR=")) 					//выставить рабочий адрес
		{
		set_adr=calk_parametr(ukaz);
		goto	end_ok;
		} 
if (uncode_com("AT+ADR_EEP=")) 					//выставить рабочий адрес EEP
		{
		set_adr_eep=calk_parametr(ukaz);
		goto	end_ok;
		} 
if (uncode_com("AT+WRPAKET=")) 				//записать N байт пакетами по 64
		{
		wrk_long=calk_parametr(ukaz);
		putst("\r>");
		Frame512_Fl_Write(set_adr,wrk_long);			// принять и записать  фрейм по 64 байта в пакете
		goto	end_ok;
		} 
if (uncode_com("AT+RDPAKET=")) 				//прочитать N байт пакетами по 64
		{
		wrk_long=calk_parametr(ukaz);
		putst("\r>");
		if (Frame512_Fl_Read(set_adr,wrk_long))	goto	end_ok;				// прочитать и выдать  фрейм по 64 байта в пакете
		goto	end_ok;
		}
if (uncode_com("AT+WR_EEP=")) 				//запись фрейма N байт в EEPROM по адресу
		{
		ukaz_tek=find_N_char(buf_rx_command,',',1,MAX_BUF_RX_COM);	// найти ,
		set_adr_eep=(int)calk_parametr(ukaz_tek);	// вычислить адрес
		wrk_int=calk_parametr(ukaz);			// вычислить значение
		if(!write_eep_frame(set_adr_eep,wrk_int))return; // если ошибка (timeout)
		goto	end_ok;			//	
		}  
if (uncode_com("AT+RD_EEP=")) 				//чтение фрейма N байт из EEPROM по адресу
		{
		ukaz_tek=find_N_char(buf_rx_command,',',1,MAX_BUF_RX_COM);	// найти ,
		set_adr_eep=(int)calk_parametr(ukaz_tek);	// вычислить адрес
		wrk_int=calk_parametr(ukaz);			// вычислить значение
		read_eep_frame(set_adr_eep,wrk_int);
		goto	end_ok;			//	
		} 
if (uncode_com("AT+CHIP_ERASE")) 				//стирание всей микросхемы
		{
		Chip_Erase();
		goto	end_ok;			//	
		} 
if (uncode_com("AT+SECTOR_ERASE=")) 				//стирание сектора 4кб по адресу
		{
		wrk_long=calk_parametr(ukaz);			// вычислить значение
		Sector_Erase(wrk_long);
		goto	end_ok;			//	
		} 

if (uncode_com("AT+BRIDGE=")) 					//ввключить ретрансляцию com1 на com2 для контроля
		{
		fl_bridge_to_RX=calk_parametr(ukaz);
		goto	end_ok;
		}
//-------------------------------------
if ((marker_viz)&&uncode_com("AT+TIME?")) 				//чтение системного времени
		{
		SW_save=SW_uart;
		change_uart_init(mod_GSM);
		putst("AT\r");
		delay_ms(250);
		put_command("AT+CCLK?\r");						// если в комманде ошибка - перезапуск
		wr_ptbuf(getch_n(11,SW_uart,&fl_read_error),0);
		for(i=0;i<13;i++){wr_ptbuf(getch_n(1,SW_uart,&fl_read_error),1);}	//вычитать время
		delay_ms(200);
		change_uart_init(SW_save);
		putst_buf(STR_Buf);
		clear_buf_RX(SW_uart);
		delay_ms(250);
		goto	end_ok;
		}  
if ((marker_viz)&&uncode_com("AT+TIME=")) 				//установка системного времени
		{
		SW_save=SW_uart;
		change_uart_init(mod_GSM);
		ukaz=find_N_char(buf_rx_command,'=',1,MAX_BUF_RX_COM);	// найти =
		ukaz++;
		putst("AT\r");
		delay_ms(250);
		putst("AT+CCLK=\"");
		for(i=0;i<14;i++){putch(buf_rx_command[ukaz]);ukaz++;}	//вычитать время
		putst(":00+00\"\r");
		delay_ms(500);
		change_uart_init(SW_save);
		clear_buf_RX(SW_uart);
		goto	end_ok;
		}  
if ((marker_viz)&&uncode_com("AT+BALANCE?")) 				//заброс баланса на карте
		{
		SW_save=SW_uart;
		change_uart_init(mod_GSM);
		wr_strbuf(" ",0);									//обнулить буфер записи сообщения
		bild_sms_balans();
		change_uart_init(SW_save);
		STR_Buf[120]=0;			//обрезать сообщение о балансе до 120 символов
		putst_buf(STR_Buf);
		clear_buf_RX(SW_uart);
		delay_ms(250);
		goto	end_ok;
		}  
if ((marker_viz)&&uncode_com("AT+STATUS?")) 				//чтение статуса сети (регистрация и уровень сигнала)
		{
		SW_save=SW_uart;
		change_uart_init(mod_GSM);
		if(put_command("AT+CREG?\r")){change_uart_init(SW_save);goto end_error;}
		wr_ptbuf(getch_n(12,SW_uart,&fl_read_error),0);
		wr_ptbuf(':',1);
		delay_ms(250);
		if (put_command("AT+CSQ\r")) {change_uart_init(SW_save);goto end_error;}
		getch_n(8,SW_uart,&fl_read_error);
		for(i=0;i<5;i++) 
		{	wrk_char=getch_n(1,SW_uart,&fl_read_error);
			if ((wrk_char=='\r')||(wrk_char==',')) break;
			wr_ptbuf(wrk_char,1);
		}
		change_uart_init(SW_save);
		putst_buf(STR_Buf);
		clear_buf_RX(SW_uart);
		delay_ms(250);
		goto	end_ok;
		} 
//------------------------------------

if (uncode_com("AT+SERV_IP1=")) 				//IP адрес первого сервера строка по ОD
		{
		save_parametr(ADR_IP1);
		goto	end_ok;
		} 
if (uncode_com("AT+SERV_IP1?")) 				//IP адрес первого сервера строка по ОD
		{
		read_parametr(ADR_IP1);
		goto	end_ok;
		} 
if (uncode_com("AT+GET_ST=")) 				//путь в посылке GET строка по ОD
		{
		save_parametr(GET_ST);
		goto	end_ok;
		} 
if (uncode_com("AT+GET_ST?")) 				//IP адрес первого сервера строка по ОD
		{
		read_parametr(GET_ST);					// прочитать до нуля
		goto	end_ok;
		} 
if (uncode_com("AT+APN=")) 				//строка APN по ОD
		{
		save_parametr(APN);
//		putst("\rWrite APN OK\r");

		goto	end_ok;
		} 
if (uncode_com("AT+APN?")) 				//строка APN по ОD
		{
		read_parametr(APN);					// прочитать до нуля
		goto	end_ok;
		} 
if (uncode_com("AT+IND_NUM=")) 				//индивидуальный номер устройства 4 цифры 
		{
		ukaz_tek=find_N_char(buf_rx_command,'=',1,MAX_BUF_RX_COM);	// найти =
		ukaz_tek++;
		ukaz=copy_buf(Bufer_SPI,&buf_rx_command[ukaz_tek],0x0D,MAX_BUF_RX_COM);//скопировать по 0D
		Bufer_SPI[ukaz]=0;		//признак конца
		if(ukaz<4) 	{putst("\rWrong Parametr\r");goto	end_ok;}
		Frame_Buf_Write(IND_NUM,ukaz+1);
		putst("\rWrite IND_NUM OK\r");
		goto	end_ok;
		} 
if (uncode_com("AT+IND_NUM?")) 				//индивидуальный номер устройства 
		{
		read_parametr(IND_NUM);					// прочитать до нуля
		goto	end_ok;
		} 
if (uncode_com("AT+WR_STR_AT=")) 				//запись строки в at45 по адресу
		{
		ukaz_tek=find_N_char(buf_rx_command,',',1,MAX_BUF_RX_COM);	// найти ,
		wrk_long=calk_parametr(ukaz_tek);
		ukaz_tek++;
		ukaz=copy_buf(Bufer_SPI,&buf_rx_command[ukaz_tek],0x0D,MAX_BUF_RX_COM);//скопировать по 0D
		Bufer_SPI[ukaz]=0;		//признак конца
		Frame_Buf_Write(wrk_long,ukaz+1);
		putst("\rWrite OK\r");
		goto	end_ok;			//	
		}  
if (uncode_com("AT+RD_STR_AT=")) 				//чтение строки в АТ45 по адресу
		{
		wrk_long=calk_parametr(ukaz);
		read_parametr(wrk_long);
		goto	end_ok;			//	
		}  
if (uncode_com("AT+WR_BYTE_EEP=")) 				//запись байта в EEPROM по адресу
		{
		ukaz_tek=find_N_char(buf_rx_command,',',1,MAX_BUF_RX_COM);	// найти ,
		wrk_long=calk_parametr(ukaz_tek);		// вычислить адрес
		wrk_char=calk_parametr(ukaz);			// вычислить значение
		EE_WRITE((int)wrk_long,wrk_char);
		putst("\rWrite OK\r");
		goto	end_ok;			//	
		}  
if (uncode_com("AT+RD_BYTE_EEP=")) 				//чтение байта из EEPROM по адресу
		{
		wrk_long=calk_parametr(ukaz);
		putst("\r");putchdec(EE_READ((int)wrk_long));putst("\r");
		goto	end_ok;			//	
		}  

end_error:
putst("\rERROR\r");
CLRWDT();
return;
end_ok:
putst("\rOK\r");
}


//=================================================================================

unsigned char check_str(register const char *str,unsigned char ukaz)		//сравнить строку с коммандой
{
	while((*str)!=0)
	{
    if ((*str)!=buf_rx_command[ukaz]) {return 0;}	// не совпала комманда
		ukaz++;
		str++;
	}
return 1;		// полное совпадение
}
//----------------------------------------
unsigned char multi_write(unsigned char type_ask)					// запись мультипараметра в память
																	//type_ask параметр направления запроса 0 это терминал 1- SMS команда 
{																	// 	1 - обработана, не требует отчета
																	//	2- обработана, требует отчета
																	//	3-обработана но в параметрах ошибка
unsigned long adres_save,parametr_save;
unsigned char ukaz,ukaz_tek,ukaz_param,type_memory,type_parametr,sig;
ukaz_tek=find_N_char(buf_rx_command,'=',1,MAX_BUF_RX_COM);		// найти =
ukaz_param=find_N_char(buf_rx_command,',',2,MAX_BUF_RX_COM);		// найти вторую запятую, на параметр
if(ukaz_param==0xFF) if(!type_ask){{putst("#ERR PARAMETR\r");}return 3;}      // ошибка параметра
type_memory=buf_rx_command[ukaz_tek+1];					// прочитать тип записываемой памяти
if((type_memory!='f')&&(type_memory!='e')) {if(!type_ask){putst("#ERR ADRES\r");}return 3;}      // ошибка адреса
adres_save=calk_parametr_buf(&buf_rx_command[ukaz_tek+2],',');  //высчитать адрес записи
ukaz_tek=find_N_char(buf_rx_command,',',1,MAX_BUF_RX_COM);		// найти ,
if(ukaz_tek==0xFF) {if(!type_ask){putst("#ERR TYPE\r");}return 3;}      // ошибка типа
ukaz_tek++;											// стать на тип параметра и найти соответствующий
type_parametr=0;
if (check_str("bt",ukaz_tek)) {type_parametr=1;}				//тип данных byte
		else
	 if (check_str("sbt",ukaz_tek)) {type_parametr=2;}				//тип данных sbyte	
		else
	 if (check_str("int",ukaz_tek)) {type_parametr=3;}				//тип данных 
 		else
	 if (check_str("sint",ukaz_tek)) {type_parametr=4;}				//тип данных		
		else
	 if (check_str("ln",ukaz_tek)) {type_parametr=5;}				//тип данных	
		else
	 if (check_str("sln",ukaz_tek)) {type_parametr=6;}				//тип данных
		else
	 if (check_str("str",ukaz_tek)) {type_parametr=7;}				//тип данных 
if(!type_parametr) {if(!type_ask){putst("#ERR TYPE\r");}return 3;}      // ошибка типа

ukaz_param++;  // перейти на сдедующий после запятой 
if((type_parametr==1)||(type_parametr==3)||(type_parametr==5)) {parametr_save=calk_parametr_buf(&buf_rx_command[ukaz_param],0x0D);}  //высчитать параметр беззнаковый
if((type_parametr==2)||(type_parametr==4)||(type_parametr==6)) 
		{
			sig=buf_rx_command[ukaz_param];
			if((sig=='+')||(sig=='-')) {ukaz_param++;}		//запомнить знак числа, если нет то +
				else {sig='+';}
			parametr_save=calk_parametr_buf(&buf_rx_command[ukaz_param],0x0D);  //высчитать параметр знаковый
			if(sig=='-'){parametr_save=0-parametr_save;}      // если отрицательно то получить отрицательное
		}
	// обработка закончена, записываем результаты
switch (type_parametr)
	{
		case 1:
		case 2:
				if(type_memory=='f')								//сохранить BYTE во FLASH
				{ 
			  		if(!type_ask){putst("#ERR ADRES\r");}return 3;      // ошибка адреса					
				}
				if(type_memory=='e')								//сохранить BYTE во EEPROM
				{ 
			  		EE_WRITE((unsigned int)adres_save,(unsigned char)parametr_save);		
				}
			break;
		case 3:
		case 4:
				if(type_memory=='f')								//сохранить INT во FLASH
				{ 
			  		if(!type_ask){putst("#ERR ADRES\r");}return 3;      // ошибка адреса					
				}
				if(type_memory=='e')								//сохранить INT во EEPROM
				{ 
			  		EE_WRITE_Int((unsigned int)adres_save,(unsigned int)parametr_save);		
				}	
			break;
		case 5:
		case 6:
				if(type_memory=='f')								//сохранить LONG во FLASH
				{ 
			  	if(!type_ask){putst("#ERR ADRES\r");}return 3;      // ошибка адреса					
				}
				if(type_memory=='e')								//сохранить LONG во EEPROM
				{ 
			  	EE_WRITE_Long((unsigned int)adres_save,(unsigned long)parametr_save);		
				}
			break;
		case 7:
				if(type_memory=='f')								//сохранить строку во FLASH
				{ 
				 ukaz=copy_buf(Bufer_SPI,&buf_rx_command[ukaz_param],0x0D,MAX_BUF_RX_COM);//скопировать по 00
				 Bufer_SPI[ukaz]=0;		//признак конца
				 Frame_Buf_Write(adres_save,ukaz+1);					
				}
				if(type_memory=='e')								//сохранить строку во EEPROM
				{ 
				 ukaz=copy_buf(work_string,&buf_rx_command[ukaz_param],0x0D,MAX_BUF_RX_COM);//скопировать по 00
				 work_string[ukaz]=0;		//признак конца
					EE_WRITE_String(adres_save,work_string);
				}
			break;
		default:
				if(!type_ask){putst("#ERR TYPE\r");}return 3;     // ошибка типа
				break;
	}

if(!type_ask){putst("#OK\r");}      // 	
return 2;		
}
//-----------------------------------
unsigned char multi_read(unsigned char type_ask)					// чтение мультипараметра из памяти
												// type_ask параметр направления запроса 0 это терминал 1- SMS команда 
												// 	1 - обработана, не требует отчета
												//	2- обработана, требует отчета
												//	3-обработана но в параметрах ошибка
{
unsigned long adres_read,parametr_read;
unsigned char ukaz,ukaz_tek,type_memory,type_parametr,sig,i;
signed char sigbyte;
signed int sigint;
signed long siglong;

ukaz_tek=find_N_char(buf_rx_command,'=',1,MAX_BUF_RX_COM);		// найти =
type_memory=buf_rx_command[ukaz_tek+1];					// прочитать тип записываемой памяти
if((type_memory!='f')&&(type_memory!='e')) {if(!type_ask){putst("#ERR ADRES\r");}return 3;}      // ошибка адреса
adres_read=calk_parametr_buf(&buf_rx_command[ukaz_tek+2],',');  //высчитать адрес записи
ukaz_tek=find_N_char(buf_rx_command,',',1,MAX_BUF_RX_COM);		// найти ,
if(ukaz_tek==0xFF) {if(!type_ask){putst("#ERR TYPE\r");}return 3;}      // ошибка типа
ukaz_tek++;											// стать на тип параметра и найти соответствующий
type_parametr=0;
if (check_str("bt",ukaz_tek)) {type_parametr=1;}				//тип данных byte
		else
	 if (check_str("sbt",ukaz_tek)) {type_parametr=2;}				//тип данных sbyte	
		else
	 if (check_str("int",ukaz_tek)) {type_parametr=3;}				//тип данных 
 		else
	 if (check_str("sint",ukaz_tek)) {type_parametr=4;}				//тип данных		
		else
	 if (check_str("ln",ukaz_tek)) {type_parametr=5;}				//тип данных	
		else
	 if (check_str("sln",ukaz_tek)) {type_parametr=6;}				//тип данных
		else
	 if (check_str("str",ukaz_tek)) {type_parametr=7;}				//тип данных 

if(!type_parametr) {if(!type_ask){putst("#ERR TYPE\r");}return 3;}      // ошибка типа
	// обработка закончена, записываем результаты
switch (type_parametr)
	{
		case 1:
				if(type_memory=='f')								//прочитать BYTE во FLASH
				{ 
			  		if(!type_ask){putst("#ERR ADRES\r");}return 3;      // ошибка адреса					
				}
				if(type_memory=='e')								//прочитать BYTE во EEPROM
				{ 
					if(!type_ask){putchdec(EE_READ((unsigned int)adres_read));putst("\r");}
						else
						{wr_strbuf(buf_rx_command,1);wr_ptbuf('\r',1);wr_ptbuf_dec(EE_READ((unsigned int)adres_read),1);wr_ptbuf('\r',1);}
				}
			break;
		case 2:
				if(type_memory=='f')								//прочитать SBYTE во FLASH
				{ 
			  		if(!type_ask){putst("#ERR ADRES\r");}return 3;      // ошибка адреса					
				}
				if(type_memory=='e')								//прочитать SBYTE во EEPROM
				{ 
					sigbyte=EE_READ((unsigned int)adres_read);
					if(!type_ask)
						{
						if(sigbyte<0){putch('-');sigbyte=0-sigbyte;}
							else{if(sigbyte>0){putch('+');}}			
							putchdec(sigbyte);putst("\r");
						}
						else
						{
							wr_strbuf(buf_rx_command,1);wr_ptbuf('\r',1);
							if(sigbyte<0){wr_ptbuf('-',1);sigbyte=0-sigbyte;}
								else{if(sigbyte>0){wr_ptbuf('+',1);}}
							wr_ptbuf_dec(EE_READ((unsigned int)adres_read),1);wr_ptbuf('\r',1);
						}
				}
			break;
		case 3:
				if(type_memory=='f')								//прочитать INT во FLASH
				{ 
			  		if(!type_ask){putst("#ERR ADRES\r");}return 3;      // ошибка адреса					
				}
				if(type_memory=='e')								//прочитать INT во EEPROM
				{ 
					if(!type_ask){putch_long(EE_READ_Int((unsigned int)adres_read));putst("\r");}
						else {wr_strbuf(buf_rx_command,1);wr_ptbuf('\r',1);wr_ptbuf_long(EE_READ_Int((unsigned int)adres_read),1);wr_ptbuf('\r',1);}		
				}	
			break;
		case 4:
				if(type_memory=='f')								//прочитать SINT во FLASH
				{ 
			  		if(!type_ask){putst("#ERR ADRES\r");}return 3;      // ошибка адреса					
				}
				if(type_memory=='e')								//прочитать SINT во EEPROM
				{ 
						sigint=EE_READ_Int((unsigned int)adres_read);
					if(!type_ask)
						{
						if(sigint<0){putch('-');sigint=0-sigint;}
							else{if(sigint>0  ){putch('+');}}			
						putch_long(sigint);putst("\r");
					    }
						else
						{
						wr_strbuf(buf_rx_command,1);wr_ptbuf('\r',1);
						if(sigint<0){wr_ptbuf('-',1);sigint=0-sigint;}
							else{if(sigint>0  ){wr_ptbuf('+',1);}}	
						wr_ptbuf_long(sigint,1);wr_ptbuf('\r',1);		
						}
				}	
			break;
		case 5:
				if(type_memory=='f')								//прочитать LONG во FLASH
				{ 
			  	if(!type_ask){putst("#ERR ADRES\r");}return 3;      // ошибка адреса					
				}
				if(type_memory=='e')								//прочитать LONG во EEPROM
				{ 
				if(!type_ask)
					   {parametr_read=EE_READ_Long((unsigned int)adres_read);putch_long(parametr_read);putst("\r");	}
					else {wr_strbuf(buf_rx_command,1);wr_ptbuf('\r',1);wr_ptbuf_long(EE_READ_Long((unsigned int)adres_read),1);wr_ptbuf('\r',1);}		
				}
			break;
		case 6:
				if(type_memory=='f')								//прочитать SLONG во FLASH
				{ if(!type_ask){putst("#ERR ADRES\r");}return 3; }    // ошибка адреса					
				if(type_memory=='e')								//прочитать SLONG во EEPROM
				{ 
					siglong=EE_READ_Long((unsigned int)adres_read);
					if(!type_ask)
						{
						if(siglong<0){putch('-');siglong=0-siglong;}
							else{if(siglong>0){putch('+');}}			
						putch_long(siglong);putst("\r");
						}
						else
						{
						wr_strbuf(buf_rx_command,1);wr_ptbuf('\r',1);
						if(siglong<0){wr_ptbuf('-',1);siglong=0-siglong;}
							else{if(siglong>0  ){wr_ptbuf('+',1);}}	
						wr_ptbuf_long(siglong,1);wr_ptbuf('\r',1);		
						}
						
				}
			break;
		case 7:
				if(type_memory=='f')								//прочитать строку во FLASH
				{ 
					Frame_Buf_Read(adres_read,0);					// прочитать до нуля
					if(!type_ask){putst_buf(Bufer_SPI);putst("\r");	}
					   else
						{wr_strbuf(buf_rx_command,1);wr_ptbuf('\r',1);wr_strbuf(Bufer_SPI,1);wr_ptbuf('\r',1);}					
				}
				if(type_memory=='e')								//прочитать строку во EEPROM
				{ 
				  EE_READ_String(adres_read,work_string);
					if(!type_ask){putst_buf(work_string);putst("\r");}
					   else
						{wr_strbuf(buf_rx_command,1);wr_ptbuf('\r',1);wr_strbuf(work_string,1);wr_ptbuf('\r',1);}		
				}
			break;
		default:
				if(!type_ask){putst("#ERR TYPE\r");}return 3;      // ошибка типа
				break;
	}	
return 1;	
}


//**********************************************************************
