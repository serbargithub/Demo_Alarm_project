#include <pic18.h>
//#include "ctype.h"
#include "name_port.h"
#include "my_delay.h"
#include "usart.h" 
#include "my_spi_SST26.h"
#include "function.h" 
#include "fun_program.h"
#include "speech.h"
#include "flash_const.h"
#include "name_konst.h"
#include "name_adres.h"
#include "eeprom.h"
#include "terminal.h"
#include "fun_abon.h"
#include "signal.h"
#include "SMS_command.h"

//18F46K22
__CONFIG (1, PLLCFG_ON & IESO_OFF & FCMEN_OFF & PRICLKEN_ON & FOSC_INTIO67);   
__CONFIG (2, PWRTEN_ON & WDTEN_ON &WDTPS_256& BOREN_ON & BORV_250);
__CONFIG (3, MCLRE_INTMCLR & PBADEN_OFF);
__CONFIG (4, DEBUG_OFF & STVREN_ON & LVP_OFF & XINST_OFF);
__CONFIG (5, CP0_ON & CP1_ON & CP2_ON & CP3_ON & CPB_ON & CPD_ON);

//============================================

const char st_adr_abon=Abonent1;

//переменные внешние
unsigned char cnt_clk,synhr,cnt_wt,glob_knopka_system,sw_opov_post_snyat,SW_uart,trub_on;
unsigned char cnt_secund1,per_beep_pl,per_beep_ot,cnt_beep,led_inform,temp_int_zone;
unsigned char cnt_lov_bat,fl_bort_napr,dist_knop,Connection_ID;
unsigned int  cnt_secund,cnt_siren,cnt_bezdejstvie,const_dat_wait,CNT_time_auto_set,mask_out_siren,Lenght_bt_mes;
unsigned char fl_cary,fl_clip,fl_gudok,fl_ring_yes,fl_first_start,Sync_Voice,fl_start_test,fl_bridge_to_RX,fl_bridge_to_TX;
unsigned char fl_beep,fl_faza_beep,sost_trevog;
unsigned char opros_dat_enable,fl_sec_code,fl_no_carier,fl_no_carier_yes,bit_end_time,fl_busy,fl_count_siren,fl_SMS_rd_yes,fl_bt_DATA_yes,fl_bt_CONNECT_yes,fl_bt_DISCONN_yes;
unsigned char st_bufrx,end_bufrx;
unsigned char st_bufrx2,end_bufrx2;
unsigned char st_bufRxblth,end_bufRxblth,count_Txblth;
unsigned char post_snjat;

signed  char temperatura_dallas[2],temperatura_save[2];

unsigned char number_str[MAX_NUMB];
unsigned char buffer_clip[16];
unsigned char buffer_rx[MAX_BUF_UART];
unsigned char buffer_rx2[MAX_BUF_UART];
unsigned char buffer_Rxblth[MAX_BUF_BLTH];
unsigned char buffer_Txblth[MAX_BUF_BLTH];
struct info_datchik
{
unsigned char sost;
unsigned char tek_sost;
unsigned int wait;
unsigned char hold;				// зоны оповещенные
unsigned char triger;			//в тригере младшая тетерада- охраняемое положение старшая защелка
unsigned char alarm;				// зоны в состоянии тревоги
unsigned char konfig;			// конфигурация зоны 0-обычная 1-входная-проходная	2-круглосуточная,3-зона бездействия
}

struct info_datchik buf_dat[last_zon+1];		// 2датчика  + 0- вход постановки снятия

const char sinusoida[]={85,111,134,152,163,165,158,144,123,98,72,47,26,12,5,7,18,36,59,0x00}; 

void putch_ln(unsigned long c);
//=====================================
static void interrupt
isr(void)			// 
{
 const char ring_str[]={"RING\r\n"};
 const char no_carier_str[]={"NO CARRIER\r\n"};
 const char err_str[]={"ERROR\r"};
 const char rd_SMS_str[]={"+CMTI:"};
 const char bt_DATA[]={"+BTSPPDATA: "};
 const char bt_CONNECT[]={"+BTCONNECTING:"};
 const char bt_DISCONN[]={"+BTDISCONN:"};
static unsigned char count_br,work_int,work_int2,count_per,n_zone,temp_zn1,svetik_synhr,count_noc,cnt_busy,count_rd_sms,count_rd_Rxblth,count_bt_connect,count_bt_disconn;	//счетчик номера периода;


if(TMR2IF)
	{
	TMR2IF =0;
	cnt_clk--;
	synhr++;
	if(!(cnt_wt--)) {fl_cary=1;} 	//внутренний счетчик

	if(!synhr)		{
					if(!(--cnt_secund1))
							{cnt_secund1=63;++cnt_secund;				//счетчик секунд
							if(fl_count_siren){if(cnt_siren){cnt_siren--;if(!cnt_siren) {set_out_siren_int(0);}}}
							if(cnt_bezdejstvie){cnt_bezdejstvie--;}
							if(CNT_time_auto_set) {CNT_time_auto_set--;}
							}
					svetik_synhr++;
					if(!(--cnt_busy)){cnt_busy=19;if(fl_busy){fl_gudok^=1;}}			//формирователь занято
					}

	if(fl_gudok&&(synhr&0x01))    										//поделить частоту на 2
						{	
							CCPR2L=sinusoida[count_per];						//вывести частоту
							count_per++;  
							if	(sinusoida[count_per]==0) {count_per=0;}
						} 

	if(fl_beep)
				{
				cnt_beep++;
				if(fl_faza_beep)
						{if(cnt_beep>=per_beep_pl){CCPR2L=5;fl_faza_beep=0;cnt_beep=0;}}
					else{if(cnt_beep>=per_beep_ot){CCPR2L=150;fl_faza_beep=1;cnt_beep=0;}}
				}

//--------------------------------------
			led_sost=0;
		switch(led_inform)	
				{

					case 0:
						led_sost=0;									//снята с охраны
					break;
					case 1:
						if(svetik_synhr&0x40) {led_sost=1;}			// задержки
						break;
					case 2:
						if(svetik_synhr&0x10) {led_sost=1;}			// тревога
					break;
					case 3:
						 led_sost=1;								//под охраной
					break;
					case 4:
						if((svetik_synhr&0xA0)==0xA0) {led_sost=1;}			// снята трубка
						break;
					case 5:
						if((svetik_synhr&0xA4)==0xA4) {led_sost=1;}			//передача данных
					break;
					case 6:
						if(svetik_synhr&0x14) {led_sost=1;}			// тревога
					break;
					case 7:
							if(svetik_synhr&0x04) {led_sost=1;}		//режим программирования
					break;
					default:
							led_sost=0; 
						break;
				}

//--

if (opros_dat_enable)
		{temp_int_zone++;
		temp_int_zone&=0x07;
		switch (temp_int_zone)
		 {
			case 0:
				++n_zone;
				if((n_zone)>last_zon){n_zone=0;}		
				set_zone_int(n_zone);					//выставить номер зоны
				break;
			case 2:
				GODONE=1;
				break;

			case 4:
				work_int=ADRESH;
				ADIF=0;
				temp_zn1=zon_mid;
				if(work_int<55) {temp_zn1=zon_lo;}			// проанализировать напряжение
					else{ 
						if(work_int>135){temp_zn1=zon_hi;}
						}
				if(buf_dat[n_zone].wait>const_dat_wait) {buf_dat[n_zone].wait=const_dat_wait;}		//защита на 1-е включение
				set_zone_int(in_trub);					//выставить ацп на вход датчика снятия трубки
				break;
			case 5:					//реакция датчиков
				if(buf_dat[n_zone].tek_sost!=temp_zn1){	buf_dat[n_zone].tek_sost=temp_zn1;buf_dat[n_zone].wait=const_dat_wait;}
					else {if	(!(--buf_dat[n_zone].wait)) {buf_dat[n_zone].sost=temp_zn1;buf_dat[n_zone].wait=const_dat_wait;}}
				GODONE=1;			// запустить АЦП
				break;
			case 6:					//опрос тригеров датчиков
				if((!(0xf0&buf_dat[n_zone].triger))&&(buf_dat[n_zone].sost-(0x0f&buf_dat[n_zone].triger)))	//если тригер не сработан
						{																//и значение отличается от охраняемого
							buf_dat[n_zone].triger|=(buf_dat[n_zone].sost<<4);  // установить тригер в сработанное положение
						}
				work_int=ADRESH;
				ADIF=0;
				trub_on=0;
				if(work_int>77) {trub_on=1;}			// проанализировать напряжение и выставить значение
				break;

			default:
			break;
		 }
		}
if((post_snjat==1)||(post_snjat==3))					// режим 1 и 3 (режим кнопки)
	{
		if(buf_dat[0].sost!=(buf_dat[0].triger&0x0F))	// если вход постановки не норма то остановить сирену
			{				// остановить сирену
				if((fl_count_siren)&&(glob_knopka_system)){set_out_siren_int(0);fl_count_siren=0;cnt_siren=0;}	//:) компромис, тушить только оди раз, и только при охране от кнопки
			}
	}
	else{    										   //все остальные режимы
		if(buf_dat[0].sost==(buf_dat[0].triger&0x0F))	// если вход постановки норма то остановить сирену
			{				// остановить сирену
				if((fl_count_siren)&&(glob_knopka_system)){set_out_siren_int(0);fl_count_siren=0;cnt_siren=0;}	//:) компромис, тушить только оди раз, и только при охране от кнопки
			}
	    }

	if (!(synhr&0x01)) {Sync_Voice=1;}					// 8kHz синхронизация голоса

}


//--UART1

	if (RC1IF){						//если принят байт то
 				work_int=RCREG1;
	//			if(fl_ring) 
						{	if (fl_ring_yes) { fl_clip=0;count_br=0;}  // если прием защелкнут то просто прочитать байт
								else
								{	if (!fl_clip)
										{
										if (ring_str[count_br]!=work_int) {count_br=0;}
 											else {if ((count_br++)>=5) {fl_clip=1;}}
										}
									else						//запись номера
										{
										if ((++count_br)>=17)
											{ buffer_clip[count_br-17]=work_int;	// отступить 16 байт
											  if (((count_br-17)>=15)||(work_int=='"')) {fl_ring_yes=1;fl_clip=0;count_br=0;} //запомнить 14 цифр 	
											}
										}
								}
						}
				if(fl_no_carier) 
						{	if (fl_no_carier_yes) {count_noc=0;}  // если прием защелкнут то просто прочитать байт
								else
								{
									if (no_carier_str[count_noc]!=work_int) {count_noc=0;}
 											else{if ((count_noc++)>=8) {fl_no_carier_yes=1;}}
								}
								
						}
						
				if (fl_SMS_rd_yes) {count_rd_sms=0;}  // если прием защелкнут то просто прочитать байт
								else
								{
									if (rd_SMS_str[count_rd_sms]!=work_int) {count_rd_sms=0;}
 											else if ((count_rd_sms++)>=5) fl_SMS_rd_yes=1;
								}
				if (fl_bt_CONNECT_yes) {count_bt_connect=0;}  // если прием защелкнут то просто прочитать байт
								else
								{if (bt_CONNECT[count_bt_connect]!=work_int) {count_bt_connect=0;}
 											else if ((count_bt_connect++)>=12) fl_bt_CONNECT_yes=1;
								}
				if (fl_bt_DISCONN_yes) {count_bt_disconn=0;}  // если прием защелкнут то просто прочитать байт
								else
								{if (bt_DISCONN[count_bt_disconn]!=work_int) {count_bt_disconn=0;}
 											else if ((count_bt_disconn++)>=10) fl_bt_DISCONN_yes=1;
								}


				switch (fl_bt_DATA_yes)						// поток приема от блютуз (пример +BTSPPDATA: 1,17,SIMCOMSPPFORAPP)
					{
						case 0: if (bt_DATA[count_rd_Rxblth]!=work_int) {count_rd_Rxblth=0;}	// детекция строки данных блютуз
 									else if ((count_rd_Rxblth++)>=11) fl_bt_DATA_yes=1;
							break;
						case 1: Connection_ID=work_int;fl_bt_DATA_yes=2;	//сохраннить идентификатор соединения
							break;
						case 2: fl_bt_DATA_yes=3;count_rd_Rxblth=0;Lenght_bt_mes=0;			//пропустить запятую
							break;
						case 3: if (work_int!=',')											//принять длинну сообщения
									{Lenght_bt_mes*=10;Lenght_bt_mes+=(work_int-'0');}
									else {fl_bt_DATA_yes=4;if(Lenght_bt_mes>=1024){fl_bt_DATA_yes=0;count_rd_Rxblth=0;}} //если ошибка то сбросить буфер
							break;
						case 4:
								if((++end_bufRxblth)>=MAX_BUF_BLTH) {end_bufRxblth=0;}			//буфер полон-  кольцевой буфер
									buffer_Rxblth[end_bufRxblth]=work_int;							// принять сообщение в кольцевой буфер
									Lenght_bt_mes--;
									if(!Lenght_bt_mes){fl_bt_DATA_yes=0;count_rd_Rxblth=0;}
							break;

						default: fl_bt_DATA_yes=0; break;
					}


				if((++end_bufrx)>=MAX_BUF_UART) {end_bufrx=0;}			//буфер полон-  кольцевой буфер
				buffer_rx[end_bufrx]=work_int;
			}

							

//--UART2

	if (RC2IF){						//если принят байт то
 				work_int2=RCREG2;
					
				if((++end_bufrx2)>=MAX_BUF_UART) {end_bufrx2=0;}			//буфер полон-  кольцевой буфер
				buffer_rx2[end_bufrx2]=work_int2;
			//	goto end_int;
				if (fl_bridge_to_TX) {while(!TX1IF){};TX1REG=work_int;}    // если включено внешнее управление GSM то проключить канал TX на COM1
			}



end_int:
	asm("nop");				  			
}// конец прерывания



//==========================
//СТАРТ
void
main(void)
{
unsigned char temp,temp1,cnt_time_test,time_last,i,const_obnovl;
unsigned char fl_read_error;
	INTCON=0b01000000;
	OSCCON= 0b11111100;    //16*4=64Мгц частота
	OSCCON2=0b10000000;
    OSCTUNE=0b01000000;		// PLL enable
	PORTA=0;
	LATA=0;
	PORTB=0;
	LATB=0;
	PORTC=0;
	LATC=0;
	PORTD=0;
	LATD=0;
	PORTE=0;
	LATE=0;
	TMR0=0;
	TXREG=0;

	ADCON0=0b00000001;		//настроить ацп
	ADCON1=0b00000000;		//
	ADCON2=0b01000010;		//t/32 левое выравнивание

	ANSELA=0b00000001;
	ANSELB=0b00010111;
	ANSELC=0;
	ANSELD=0;
	ANSELE=0b00000100;

	CM1CON0=0;
	CM2CON0=0;
	SLRCON=0;
	PR2=249;		 //+1 250- 16 килобита
	PIE1=0b00100010; //разрешить прерывания от TMR2 и ком порта1
	PIE3=0b00100000; //разрешить прерывания от TMR2 и ком порта2


	TRISA = mask_a;
	TRISB = mask_b;	
	TRISC = mask_c;
	TRISD = mask_d;	
	TRISE = mask_e;


	serial_setup(sp115200,PRI_US);		//проинициализировать usart
	serial_setup(sp115200,SEC_US);
	
	CCP1CON=CCP2CON=0b00001100;	// включить PWM
	CCPR1L=CCPR2L=0x20;	
	CCPR1H=CCPR2H=0x20;
	T2CON=0b00011000;	//пост дел/4
	TMR2IF=0;
	TMR2ON=1;
	count_Txblth=0;		//сбросить буфер отправки Блютуз
	GIE=1;				//запустить прерывания
	pwr_key	=1;			//reset spi и PWR в исходное положение
	fl_first_start=1;
	initSPI();			//запустить SPI	
//---------------------------------
//reset:
	led_inform=3;
	for(i=0;i<50;i++)
	{	
	delay_ms(100);		
	if(!sys_key) 	{terminal_on();}		//ожидание программатора
	}
	led_inform=0;
	ch_usart(mod_GSM);	
start:
for(;;)
 {
	fl_beep=0;
	ch_usart(mod_GSM);				// подключить  usart  к GSM модулю
//=========RESET==============
	if (put_command("AT\r")) {delay_ms(250); if ( put_command("AT\r")) {initSPI();power_on();}}  // reset_module();}
	if (fl_first_start)
		{	delay_ms(255);
			delay_s(2);
			init_mask_siren();	// проинициаоизировать маску выходов сирены
			init_datchik();		//проинициализировать датчики
			buf_dat[0].triger|=(buf_dat[0].sost<<4);
			time_last=0;		//инициализация времени последней отправки тестового сообщения
			fl_start_test=0;	// признак старта теста в 0
			sost_trevog=0;				// сбросить состояние тревоги
			cnt_siren=EE_READ(time_SIREN)*60;siren_on(0);
			siren_on(0);
			cnt_lov_bat=0;
			sw_opov_post_snyat=0;				// не оповещать при включении
			fl_bort_napr=EE_READ(save_bort_napr);		//флаг бортового напряжения прочитать
			fl_bridge_to_TX=fl_bridge_to_RX=0;
			const_dat_wait=9*(unsigned int)EE_READ(Save_const_dat_wait);
		}

	fl_first_start=0;
	led_inform=0;
	delay_ms(500);
	putst("ATH\r");
	delay_ms(500);
	putst("AT+CLIP=1\r");			//отображать номер 	
	fl_ring_yes=0;						//ожидание снятия трубки	
	fl_no_carier=0;				// не детектить NO CARRIER
	fl_sec_code=0;					//сбросить детектор кода
	dist_knop=0;
	opros_dat_enable=1;
	while(1)
	{
		cnt_secund=0;
		do  
		{
		led_inform=0;						// никакой информации на светодиод не выводить
		if	(!trub_on) 
					{if(!bit_end_time){led_inform=4;fl_busy=0;telef_on();goto start;}		// если сняли трубку то отработать телефонию
						else {fl_busy=1;}		// если трубка снята 5 мин то занято
					}		
				else {bit_end_time=0;fl_gudok=0;fl_busy=0;}			// если трубка лежит то сбросить бит снятой трубки
		if(!sys_key){programm_abon(1);goto start;} // если нажата кнопка программирования то обработать ее
		if(dist_knop){dist_knop=0;programm_abon(2);goto start;} // если включена реакция на кнопку- перейти в режим программирования

 	check_datchik();		// если включен контроль за датчиками- контролировать
		if(fl_ring_yes) 
			{
				if(!check_in_ring()) 
						{fl_ring_yes=0;cnt_secund=0;opros_dat_enable=1;
						if(dist_knop){dist_knop=0;programm_abon(2);} // если включена реакция на кнопку- перейти в режим программирования
						 goto start;
						}
						else{delay_ms(500);put_command("ATH\r");delay_s(1);fl_ring_yes=0;}
			}

		if(sw_opov_post_snyat){opov_postanovka_snjatie(sw_opov_post_snyat-1);sw_opov_post_snyat=0;}  //если нужно оповестить о постановке снятиии то оповестить и снять флаг
		if(fl_SMS_rd_yes){fl_SMS_rd_yes=0;delay_ms(500);break;}		//если принята SMS сразу стоп
		delay_ms(255);
		check_err();


		}while(cnt_secund<60);			// раз в 1 минуту полный опрос системы


		if (!check_reg_GSM()) {reboot_GSM();initSPI();}		//если зарегистрировались перезагрузка
		const_obnovl++;
		if(const_obnovl>10)
				{
					const_obnovl=0;
					const_dat_wait=9*(unsigned int)EE_READ(Save_const_dat_wait);		//oбновление константы наслучай сбоя
					delay_ms(250);
					putst("AT+CLIP=1\r");			//отображать номер 
					delay_ms(250);
				}

		check_sms_command();				//проверить , нет ли SMS комманды

		if(opros_dat_enable){while (temp_int_zone!=5) {CLRWDT();}}		// дождаться окончания опрося датчиков в прерывании
		opros_dat_enable=0;							// остановить опросдатчиков
		delay_ms(10);
		ADCON0=0b00000001;		//подключить АЦП
		delay_ms(10);
		GODONE=1;
		delay_ms(20);
		temp=ADRESH;
		opros_dat_enable=1;

	if(fl_bort_napr)
			{
			if(temp<LOW_bort_napr)
								{	if((cnt_lov_bat++)>5)
										{fl_bort_napr=0;cnt_lov_bat=0;
										EE_WRITE(save_bort_napr,0);
										send_opov_napr();				
										}
								}
							else {cnt_lov_bat=0;}
			}
		 else
			{
			if(temp>=HIGH_bort_napr)
								{	if((cnt_lov_bat++)>5)
										{fl_bort_napr=1;cnt_lov_bat=0;
										EE_WRITE(save_bort_napr,1);
										send_opov_napr();
										}
								}	
							else {cnt_lov_bat=0;}
			}	
	temp=EE_READ(Conf_TEMP_DAT);
	temp1=EE_READ(Inf_TEMP_DAT);
	if(temp) {send_opov_temperatura (temp,temp1,1);	}	// если датчик 1 температуры включен - контроль температуры
	temp=EE_READ(Conf_TEMP_DAT+1);
	temp1=EE_READ(Inf_TEMP_DAT+1);
	if(temp) {send_opov_temperatura (temp,temp1,2);	}	// если датчик температуры включен - контроль температуры
	//	putst("at+napr->");putchdec(temp);putst("\r");
		delay_ms(500);
		if (!EE_READ(regim_TEST)) {continue;}		//если тест не отсылать , то сразу дальше
		if (put_command("AT+CCLK?\r")) {break;}	// если в комманде ошибка - перезапуск
		temp=(getch_n(20,SW_uart,&fl_read_error)-'0')*10;temp+=getch_n(1,SW_uart,&fl_read_error)-'0';	//вычислить время(часы)
		if (!fl_start_test){if(EE_READ(start_TEST)!=temp) {continue;}	//	дождаться время старта
								else {fl_start_test=1;time_last=temp+1;cnt_time_test=1;}// обеспечить параметры для старта теста
							}
		if (temp!=time_last)
			{time_last=temp;
			if(!(--cnt_time_test)){cnt_time_test=EE_READ(period_TEST);send_TEST();}	// время подошло- отправить тест
			}
	}
 }


}		//конец программы



void put_TX(unsigned char c)
{
while(!TX2IF){CLRWDT();}TXREG2=c;
}

void putch_ln(unsigned long c)		// отображение long числа
{
	static unsigned char st_razr;
	unsigned char temp;
	unsigned long  delit;
	if (!c) {put_TX('0');put_TX(';');return;}
	st_razr=0;
	for (delit=1000000000L;delit>=1;delit/=10)
		{	
			if(((c/delit)>0)||st_razr) 
					{
					temp=(c/delit)+'0';
					st_razr=1;
					put_TX(temp);
					}
			c-=(c/delit)*delit;
			CLRWDT();
		}
put_TX(';');
}
//**********************************************************************
