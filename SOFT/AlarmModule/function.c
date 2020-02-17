#include <pic18.h> 
#include "name_port.h"
#include "my_delay.h"
#include "usart.h" 
#include "usart_buf.h"
#include "fun_program.h"
#include "speech.h"
#include "name_konst.h"
#include "name_adres.h"
#include "my_spi_SST26.h"
#include "eeprom.h"
#include "terminal.h"
#include "Contact_id.h"
#include "voice_map.h"
#include "lexem_rutine.h"

extern unsigned char cnt_clk,SW_uart,trub_on;
extern  unsigned int cnt_secund;
extern  unsigned char  cnt_beep,per_beep_pl,per_beep_ot;
extern unsigned char fl_beep,term_razg,fl_first_start,fl_gudok,fl_ring_yes,fl_zvon,bit_end_time;
extern   unsigned char number_str[];



unsigned char load_pulse(void);
unsigned char delay_s_trub(unsigned char wait);
void sound_error(void);
void sound_ok(void);
void detect_command(void);
char set_dtmf(void);
unsigned char wait_reg(void);

//-------
char wait_dtmf (char time)
{ char i;
 for(i=0;i<time;i++)
	{
	delay_s_dso(1);
	if(dso_vg) {return set_dtmf();}
	}
	return 0;		//время вышло
}
//----------------------------------------
char cod_dtmf(void)
{
char numb=0;
	if (!dso_vg) {return 0;}
	if (q4_vg) {numb|=0b00001000;}
	if (q3_vg) {numb|=0b00000100;}
	if (q2_vg) {numb|=0b00000010;}
	if (q1_vg) {numb|=0b00000001;}
return numb;
}
//=============
char set_dtmf(void)
{
char numb;
numb=cod_dtmf();
	while(1)
	{	
	CLRWDT();
	if	(!dso_vg){delay_ms_dso(cost_wt_dtmf); if (!dso_vg) break;}
	}
if (!numb) {return 0;}
numb+='0';					
 if (numb==0x3A) numb=0x30;		//коррекция нуля
return numb;	
}
//----------------------------------
char detekt_razg(void)				// детектор состояния телефона 0-нет соединения, 1- вызов, 2-разговор
{
unsigned char work,fl_read_error;
		if (put_command("AT+CLCC\r")) {delay_s(1); if (put_command("AT+CLCC\r")) {return 0;}}
		if(getch_n(4,SW_uart,&fl_read_error)=='K') {return 0;}
		work=getch_n(10,SW_uart,&fl_read_error);
		if((work=='2')||(work=='3')) {return 1;}		//если соединение 
		if(work=='0') {return 2;}		//если разговор
	return 0;
}
//--------------------

unsigned char wait_razg(char time)			// ожидание разговора в течении времени
{
unsigned char work,fl_read_error;
time/=2;		//время поделить на 2
 do
	{
		CLRWDT();
		delay_s(2);
		put_command("AT+CLCC\r");
		work=getch_n(14,SW_uart,&fl_read_error);
		if((work!='2')&(work!='3')) break;		//если соединение то молчать
	time--;
	}while (time);
 if (!time){putst("ATH\r");delay_s(1);return 0;}
 if (work=='0')  {return 1;}  // соеденение установленно
return 0;	
}
//------------------------



unsigned char load_cif(unsigned char time_out)
{ 	unsigned char numb=0;
	cnt_secund=0;
for (;;)
{
	if (dso_vg)
				{
					if (q4_vg) {numb|=0b00001000;}
					if (q3_vg) {numb|=0b00000100;}
					if (q2_vg) {numb|=0b00000010;}
					if (q1_vg) {numb|=0b00000001;}
					while (dso_vg){CLRWDT();}
					break;
				}

	if (trub_on)	{numb=load_pulse();break;}
	if(time_out==15) {if (cnt_secund>=2) {return rehetka;}}	// время вышло - набор
	if (cnt_secund>600) {bit_end_time=1;return 0;}	// 10 минут ничего не происходит запонить и выйти
 CLRWDT();
}
if (!numb) {return 0;}
numb+='0';					
 if (numb==0x3A) numb=0x30;		//коррекция нуля
return numb;
}
//--------------
unsigned char load_pulse(void)
{
unsigned char i,numb;
numb=0;
for(;;)
{
 	for(i=0;i<100;i++)			//200ms
	{	
		delay_ms(2);
		if (!trub_on) break;		//трубка появилась стоп
	}
 if (trub_on) {return 0;}	//не прочитали ничего стоп(выход по таймауту
 if (i>15) numb++;		//если импульс больше 30 мс то запомнить его
	for (i=0;i<50;i++)	//100 мс пауза
	{
		delay_ms(2);
		if (trub_on) break;
	}
	if (numb>10) {return 0;}		// если переполнение то выход
	if (!trub_on) {return numb;}	// если трубка снята больше 100мс конец номера
}
}

//================

unsigned char load_num(unsigned char n_cif)		//прием номера из номеронабирателя результат - число принятых цифр
{									//параметр- с какой цифры начать прием
 unsigned char wr_prom,st_cif,max_cif;
	st_cif=n_cif;					//запомнить с какой начали
	max_cif=15;
	while(1)
	{
		wr_prom=number_str[n_cif]=load_cif(max_cif);
		if (!(wr_prom&&(wr_prom^0xFF)&&(wr_prom^zirochka))) {n_cif=0; break;}		//если номер принят с ошибкой, или начался набор, или нажата * выход 
		if (wr_prom==rehetka) if (n_cif!=st_cif) {n_cif--;break;}				//по решетке ускоренный набор
	n_cif++;
	if (n_cif>max_cif) {n_cif--;break;}
	} 
 return n_cif;
 }


//----------
void beep_on( char pluse,char minuse)
{
	cnt_beep=0;
	per_beep_pl=pluse;
	per_beep_ot=minuse;
	fl_beep=1;
}
	
void sound_error(void)
{
delay_s(1);
voice_speech(oshibka,PWM1);
}
//-----
void sound_ok(void)
{
unsigned char i;
		delay_ms(100);
	for(i=1;i<=3;i++)
	{
		beep_on(10,10);
		delay_ms(40);
		fl_beep=0;
		delay_ms(100);
	}
}		
		

//=============
unsigned char delay_s_trub(unsigned char wait)
{
		unsigned char i;
	while (wait)
	{
	for (i=0;i<=125;i++)
		{
		cnt_clk=255;			//1s
		while (cnt_clk){CLRWDT();}
		if(trub_on) break;		//если трубка положена - стоп задержка 
		}
	wait--;
		if(trub_on) break;
	}
 delay_ms(100);
	if (trub_on) {return 1;} 
return 0;
}
//==========================
void busy_on(void)
{
cnt_secund=0;
	while (!trub_on) 
	{
	fl_gudok=1;
		delay_ms(150);
		delay_ms(150);
	fl_gudok=0;
		delay_ms(150);
		delay_ms(150);
	if (cnt_secund>300) {bit_end_time=1;return;}	// 5 минут ничего не происходит запонить и выйти
	}
}

void power_on(void)
{
	delay_s(4);
	pwr_key=0;
	delay_s(3);
	pwr_key=1;
	wait_reg();
	fl_first_start=1;
	asm("sleep");
}
void reset_module(void)	
{
	delay_s(1);
	pwr_key=0;
	delay_ms(1500);
	pwr_key=1;
	power_on();						//включить модуль
	fl_first_start=1;
	asm("sleep");
} 
//---------------------------
void telef_on(void)
{
char work;
	delay_ms(250);
	putst("AT+CHFA=1\r");
	while(1)
 {
	cnt_secund=0; 		// начало отсчета бездействия
	while (!trub_on) 
	{
	fl_gudok=1;
	CLRWDT();
 if (dso_vg) break;
	if (cnt_secund>300) {bit_end_time=1;return;}	// 5 минут ничего не происходит запонить и выйти
	}
	fl_gudok=0;					//пошел набор
	work=load_num(0);							//прочитать номер
	if(work) break;			//если номер прочитан с ошибкой то еще раз 
		else return;		//сброс
  }
	if(!chek_num_central())  {busy_on();return;}		//проверить не набрана ли как централь
	bit_end_time=1;
	busy_on();

}

//-------------------------------------------
void ch_usart(char ch)
{
switch(ch)
{
case mod_GSM:
	SW_uart=0;
	break;
case komputer:
	SW_uart=1;
	break;
case Bluetooth_CH:
	SW_uart=2;
	break;
default:
	break;
}
}
//-------------------------------
void change_in (char num)
{
switch(num)
	{
		case voice_in:
			SW_vc_mic=0;
			putst("AT+CMIC=0,5\r");
			delay_ms(500);
			putst("AT+CLVL=90\r");
			delay_ms(500);
			break;
		case mik_in:
			SW_vc_mic=1;
			putst("AT+CMIC=0,8\r");
			delay_ms(500);
			break;
		default:
			break;
	}
}
//----------------------------------
unsigned char wait_reg(void)			// ожидание  в течении времени 60 сек
{
unsigned char work,fl_read_error,time;
time=30/4;
 do
	{
		CLRWDT();
			delay_s(4);
		 if (put_command("AT+CREG?\r")) 
			{delay_ms(250); 
			if (put_command("AT\r")) {pwr_key=0;delay_s(3);pwr_key=1;delay_s(3);}	// если модуль не живет, попробовать поднять его
			continue;
			}
		work=getch_n(12,SW_uart,&fl_read_error);
		if((work=='1')||(work=='5')) {return 1;}		//если зарегистрировались то стоп
	}while ((time--)>0);
return 0;	
}
//-----------------------
unsigned char check_reg_GSM(void)				// проверка регистрации всети
{
unsigned char work,fl_read_error;
		put_command("AT+CREG?\r");
		work=getch_n(12,SW_uart,&fl_read_error);
		if((work=='1')||(work=='5')) {return 1;}		//если зарегистрировались перезагрузка
	return 0;
}
//---------------------
void reboot_GSM(void)
{
	delay_s(1);
	if (check_reg_GSM()) return;

	putst("AT+CPOWD\r");			// потушить GSM
									//включить модуль
	delay_s(5);
	pwr_key=0;
	delay_s(3);
	pwr_key=1;				//разбудить модуль
	wait_reg(); 

}
//---------------------------
extern unsigned char work_buf[];
void read_imei(void)
{
unsigned char ukaz,i,work;
	for(i=0;i<5;i++)
	{
	delay_ms(1000);
	clear_buf_RX(SW_uart);
	putst("AT+GSN\r");
	read_lexem(2,3);		// прочитать 2 лексемы
	if((buf_rx_command[0]>='0')&&(buf_rx_command[0]<='9')) break;    //вычитывание правильного имея без ошибки на всякий случай
	}
		ukaz=copy_buf(work_buf,&buf_rx_command[0],0x0D,MAX_BUF_RX_COM);//скопировать по 0D
		work_buf[ukaz]=0;		//признак конца
		Frame_Buf_Read(IMEI_Save,0);
		if(!compare_buf(Bufer_SPI,work_buf,0,16))
			{
			ukaz=copy_buf(Bufer_SPI,work_buf,0,16);
			Frame_Buf_Write(IMEI_Save,ukaz+1);
			}
}


