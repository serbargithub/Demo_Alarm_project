#include <pic18.h> 
#include "name_port.h"
#include "my_delay.h"
#include "usart.h" 
#include "fun_abon.h"
#include "function.h"
#include "speech.h"
#include "name_konst.h"
#include "name_adres.h"
#include "eeprom.h"
#include "terminal.h"
#include "SMS_rutine.h"
//#include "I2C_my.h"
#include "DS18_root.h"
#include "signal.h"
#include "lexem_rutine.h"
#include "voice_map.h"


unsigned char fl_uspeh,step_voice;									// флаг  успешного отчета
unsigned int time_cnt_bezdejstvie;
extern signed char temperatura_dallas[2],temperatura_save[2];

extern  unsigned char number_str[];
extern  unsigned char Bufer_SPI[];
extern  unsigned char buffer_clip[];
extern  unsigned char opros_dat_enable,sost_trevog,fl_count_siren,fl_start_test;
extern  unsigned char led_inform,glob_knopka_system,fl_bort_napr,dist_knop,sw_opov_post_snyat,post_snjat;
extern  unsigned int cnt_secund,cnt_siren,cnt_bezdejstvie,const_dat_wait,CNT_time_auto_set,mask_out_siren;

extern struct info_datchik
{
unsigned char sost;
unsigned char tek_sost;
unsigned int wait;
unsigned char hold;				// зоны оповещенные
unsigned char triger;			//в тригере младшая тетерада- охраняемое положение старшая защелка
unsigned char alarm;				// зоны в состоянии тревоги
unsigned char konfig;			// конфигурация зоны 0-обычная 1-входная-проходная	2-круглосуточная,3-зона бездействия
}

extern  struct info_datchik buf_dat[];		// 4датчика 0- вход постановки снятия

unsigned char map_dozvon[16];					// карта дозвонов


void voice_menu(char);
void clear_zone_alarm(char);
char check_zone_alarm(char r_kont);
void alarm_dozvon(char regim);
void voice_alarm(char,char,char);
char snd_alarm_voice(char n_abon,char regim);
char menu_trevoga(char n_abon);
void init_OK_zone(void);
void init_datchik(void);
void opov_postanovka_snjatie (char);
void sound_opoveshenie(unsigned char infodat);
void voice_slugeb(unsigned char cod_opovesheniya,unsigned char type_oth);
void opoveshenie_slugebnoe(unsigned char cod_opovesheniya,unsigned char type_oth, unsigned char type_opov);
void set_vihod(char n_vihod,char sost_ch);
void menu_otcheti(char n_abon);
//---------------------------

void siren_on(char regim)
{
init_mask_siren();
if (!mask_out_siren) {fl_count_siren=0;return;}	// если не специального назначения то возврат
if(!regim){fl_count_siren=0;set_out_siren(0);return;}
	{
		cnt_siren=EE_READ(time_SIREN)*60;set_out_siren(1); // поехали
		fl_count_siren=1;
	}
}
//----------------------------
void link_out_SET (unsigned char set_w)
{
unsigned char i,end_zone,link_temp,set_out[12];

end_zone=last_zon;
for (i=0;i<12;i++) set_out[i]=0;   // очистить флаги выходов

for (i=1;i<=end_zone;i++)		   //выставление флагов
	{
	link_temp=EE_READ(link_Zone_Vih+i);
	if((link_temp==0)||(link_temp>=12)) continue;  //защита от ошибок
	if(buf_dat[i].alarm) {set_out[link_temp]=set_w;} // поставить тревожные выходы в соответствии с пришедшим параметром ( тревога или восстановление)
	}

for (i=1;i<12;i++)					//выставить выходы если позволяет конфигурация
	{
	if(EE_READ(konf_VIHOD+i-1)==1) set_vihod(i,set_out[i]);
	}
		
}

//---------------------------------------
unsigned char check_in_ring(void)			//проверка входящего номера
{
char work,i,j,n_abon;

 n_abon=check_numb();
 if(n_abon==0xff) {return 1;}			// ни один номер не совпал- звонить
 delay_ms(500);
 put_command("ATA\r");
 read_lexem(1,30);		// прочитать 1 лексему 30 сек
 if (uncode_com("CONNECT")) {terminal_PDP();return 0;}	//звонок данными
 if (!uncode_com("OK")){putst("+++");delay_s(1);putst("\rATH\r");return 0;}//eсли голос то на голос		
 delay_s(1);
 putst("AT+CHFA=0\r");
 delay_ms(300);
 change_in(voice_in);
if(EE_READ(Menu_pass))				//если включен запрос кода доступа
{
	for(i=0;i<4;i++)
 	{
 	voice_speech(kod,PWM1);
 	voice_speech(dostupa,PWM1);
 	work=wait_dtmf(5);
 	if(work==rehetka)
	{
	for(j=0;j<3;j++)
		{
		 work=wait_dtmf(5);
		 if((!work)||(work!=EE_READ(kod_parol+j)))
			 { 
				delay_s(1);
				voice_speech(oshibka,PWM1);
				delay_s_dso(1);
				break;
			 }
		}	
		if (j==3) 	{
					delay_ms(500);
					 voice_speech(kod,PWM1);
					 voice_speech(prinjat,PWM1);
					voice_menu(n_abon);
					break;
					}	// код опознан  - на меню
		}
	if(detekt_razg()!=2) {return 0;}
 	 }
}
	else
		{delay_ms(500);voice_menu(n_abon);}  //если выключен запрос кода доступа
							 // если код неопознан выход
putst("ATH\r");
return 0;					
}
//---------------------------------
char check_vihod(char n_vihod)
{

	
	switch(n_vihod)
 		{
		case 1:
			return out1; 
		case 2:
			return out2;  
		default:
			break;
		}
  	
	return 0;
}
//-------------------------------------------
void init_mask_siren(void)
{
unsigned int mask,i;
mask=0;
for (i=0;i<16;i++)
{
mask>>=1;
if(EE_READ(konf_VIHOD+i)==2) {mask|=0x8000;}
}
mask_out_siren=mask;
}
//--------------------
void set_out_siren_int(unsigned char sost_ch)  // выставить выход сирены на запрограммированные выходы
{
if (mask_out_siren&0x0001)		//если выход 1-ваход состояния
	{if(sost_ch) out1=1;
		else     out1=0;}
if (mask_out_siren&0x0002)		//если выход 2-ваход состояния
	{if(sost_ch) out2=1;
		else     out2=0;}
}
//--------------------
void set_out_siren(unsigned char sost_ch)  // выставить выход сирены на запрограммированные выходы
{
if (EE_READ(konf_VIHOD)==2)		//если выход 1-ваход состояния
	{if(sost_ch) out1=1;
		else     out1=0;}
if (EE_READ(konf_VIHOD+1)==2)		//если выход 2-ваход состояния
	{if(sost_ch) out2=1;
		else     out2=0;}
}
//--------------------
void set_out_ohran(unsigned char sost_ch)  // выставить выход индикации охраны на выходы запрограммированные
{
if (EE_READ(konf_VIHOD)==3)		//если выход 1-ваход состояния
	{if(sost_ch) out1=1;
		else     out1=0;}
if (EE_READ(konf_VIHOD)==4)		//если выход 1-ваход состояния
	{if(sost_ch) out1=0;
		else     out1=1;}
if (EE_READ(konf_VIHOD+1)==3)		//если выход 2-ваход состояния
	{if(sost_ch) out2=1;
		else     out2=0;}
if (EE_READ(konf_VIHOD+1)==4)		//если выход 2-ваход состояния
	{if(sost_ch) out2=0;
		else     out2=1;}
}
//--------------------------------------
void set_vihod(char n_vihod,char sost_ch)
{
static unsigned char sost;
		if (sost_ch) sost=1;
			else sost=0;
		switch(n_vihod)
 		{
			case 1:
				out1=sost; 
				break;
			case 2:
				out2=sost; 
				break;
			default:
				break;
  		}
	
}
//--------------------------------
char set_duble(void)
{
char item;
	while(1)
	{
		delay_s_dso(4);if(dso_vg) break;
		if(detekt_razg()!=2) {return 0;}			// если разговор оборвался  то выход
	}
		item=(set_dtmf()-'0')*10;
	while(1)
	{
		delay_s_dso(4);if(dso_vg) break;
		if(detekt_razg()!=2) {return 0;}			// если разговор оборвался  то выход
	}
		item+=set_dtmf()-'0';
return item;

}
//-------------------
void voice_out(unsigned char n_out)
{
unsigned char work;
work=EE_READ(vihod_VOICE+n_out);
			switch(work)
			{
				case 0:
					voice_speech(cif_0+n_out,PWM1);
					break;
				case 1:
					voice_speech(sireni,PWM1);
					break;
				case 2:
					voice_speech(osveshenie,PWM1);
					break;
				case 3:
					voice_speech(elektrozamok,PWM1);
					break;
				case 4:
					voice_speech(nasos,PWM1);
					break;
				case 5:
					voice_speech(otoplenie,PWM1);
					break;
				case 6:
					voice_speech(piropatron,PWM1);
					break;
				default:
					break;
			}
}
//-----------------
void voice_sost_system(void)
{
unsigned char i;
	voice_speech(sistema,PWM1);
	if (EE_READ(system_status))
				{
				voice_speech(pod_ohranoj,PWM1);
				for (i=1;i<=last_zon;i++)
 					{
					if(buf_dat[i].hold)
						{delay_ms_dso(300);voice_speech(datchik,PWM1);voice_speech(cif_0+i,PWM1);voice_speech(vikl,PWM1);}
					}
				}
		else voice_speech(snjata_s_ohrani,PWM1) ;
}

//----------------------

void menu_datchik(void)
{
char item_menu,duble_click;
	duble_click=0;
 while(1)
 {
	delay_s_dso(1);
	while(1)
	{
		voice_speech(nagatj,PWM1);
		voice_speech(cif_1,PWM1);
		voice_speech(cif_2,PWM1);
		voice_speech(datchik,PWM1);
		delay_ms_dso(300);if(dso_vg) break;
		voice_speech(nagatj,PWM1);voice_speech(cif_0,PWM1);
		voice_speech(vozvrat,PWM1);
		delay_s_dso(4);if(dso_vg) break;
		if(detekt_razg()!=2) return;			// если разговор оборвался  то выход
	}
	item_menu=set_dtmf();
	if (item_menu==zirochka) {item_menu=set_duble();}		//если нажата * то продетектить двузначный выход
	 else {item_menu-='0';}
	if(item_menu==0) return;	// выход если возврат
	if(item_menu>2) {delay_s(1);voice_speech(oshibka,PWM1);continue;}
	if(item_menu==duble_click)
		{
		if(buf_dat[item_menu].konfig&0x80) {buf_dat[item_menu].konfig&=0x7F;EE_WRITE((item_menu+konf_ZONE),buf_dat[item_menu].konfig);init_datchik();}	//инверсия входа
			else {buf_dat[item_menu].konfig|=0x80;EE_WRITE((item_menu+konf_ZONE),buf_dat[item_menu].konfig);}
		
		}	
	voice_speech(datchik,PWM1);
	voice_speech(cif_0+item_menu,PWM1);
	if (buf_dat[item_menu].konfig&0x80) voice_speech(vikl,PWM1);
		else {
			voice_speech(vkl,PWM1);
			delay_ms_dso(300);
			if(buf_dat[item_menu].triger&0xF0) {voice_speech(trevoga,PWM1);}
				else {voice_speech(ispraven,PWM1);}
			}
	duble_click=item_menu;
  }
}

//-----------------
void menu_sistema(char n_abon)
{
char item_menu,duble_click,end_zone;
	duble_click=0;
 while(1)
 {
	delay_s_dso(1);
	while(1)
	{
		delay_s_dso(1);if(dso_vg){ break;}
		voice_speech(nagatj,PWM1);voice_speech(cif_1,PWM1);voice_speech(datchik,PWM1);
		delay_ms_dso(300);if(dso_vg){ break;}
		voice_speech(nagatj,PWM1);voice_speech(cif_2,PWM1);	voice_speech(otchet,PWM1);
		delay_ms_dso(500);if(dso_vg) break;
		voice_speech(nagatj,PWM1);voice_speech(cif_9,PWM1);voice_speech(knopka,PWM1);voice_speech(dostupa,PWM1);
		delay_ms_dso(300);if(dso_vg){ break;}
		voice_speech(nagatj,PWM1);voice_speech(cif_0,PWM1);voice_speech(vozvrat,PWM1);
		delay_s_dso(4);if(dso_vg) {break;}
		if(detekt_razg()!=2) {return;}			// если разговор оборвался  то выход
	}
	item_menu=set_dtmf();
	if(item_menu=='0') {return;}	// выход если возврат
	switch (item_menu-'0')
	{
	case 1:	
			menu_datchik();
		break;
	case 2:
		menu_otcheti(n_abon);
		break;
	case 9:	
		voice_speech(knopka,PWM1);
		voice_speech(dostupa,PWM1);
		voice_speech(vkl,PWM1);
		dist_knop=1;				// кнопка дистанционного доступа
		break;
	default:
		delay_s(1);
		voice_speech(oshibka,PWM1);
		break;
	}
	duble_click=item_menu; 
  }
}
//-----------------------------------------------------

void menu_otcheti(char n_abon)
{
unsigned char item_menu,duble_click;
	duble_click=0;
 while(1)
 {
	delay_s_dso(1);
	while(1)
	{
		voice_speech(nagatj,PWM1);voice_speech(cif_1,PWM1);
		voice_speech(sms,PWM1);voice_speech(nastrojki,PWM1);
		delay_ms_dso(500);if(dso_vg) break;
		voice_speech(nagatj,PWM1);voice_speech(cif_2,PWM1);
		voice_speech(sms,PWM1);voice_speech(balans,PWM1);
		delay_ms_dso(500);if(dso_vg) break;
		voice_speech(nagatj,PWM1);voice_speech(cif_3,PWM1);
		voice_speech(sms,PWM1);voice_speech(abonenti,PWM1);
		delay_ms_dso(500);if(dso_vg) break;;
		voice_speech(nagatj,PWM1);voice_speech(cif_4,PWM1);
		voice_speech(sms,PWM1);voice_speech(temperaturi,PWM1);
		delay_ms_dso(500);if(dso_vg) break;;
		voice_speech(nagatj,PWM1);voice_speech(cif_0,PWM1);
		voice_speech(vozvrat,PWM1);
		delay_s_dso(4);if(dso_vg) break;
		if(detekt_razg()!=2) return;			// если разговор оборвался  то выход
	}
	item_menu=set_dtmf();
	if(item_menu=='0') return;	// выход если возврат
	switch (item_menu-'0')
	{
	case 1:	
		voice_speech(SMS_otoslana,PWM1);
		send_sms_nastroiki(n_abon);
		break;
	case 2:
		voice_speech(SMS_otoslana,PWM1);
		send_sms_balans(n_abon);
		break;
	case 3:
		voice_speech(SMS_otoslana,PWM1);
		send_sms_abonenti(n_abon);
		break;
	case 4:
		voice_speech(SMS_otoslana,PWM1);
		snd_SMS_otchet_temperatura(n_abon);
		break;

	default:
		delay_s(1);
		voice_speech(oshibka,PWM1);
		break;
	}
		if(detekt_razg()!=2) return;	// если разговор оборвался  то выход
  }
}

//-------------------------------------------------
void voice_menu(char n_abon)			// голосовое меню
{
unsigned char item_menu,duble_click,work,end_zone,i;
	delay_s_dso(1);
duble_click=0;
step_voice=0;
 while(1)
 {
	while(1)
	{
	if((!step_voice)&&EE_READ(Voice_otchet))								// если нужно то пропустить оповещение
		{
		voice_speech(sistema,PWM1);
		if (EE_READ(system_status))
				{voice_speech(pod_ohranoj,PWM1);
				if(sost_trevog){delay_ms_dso(500);voice_speech(trevoga,PWM1);}
				}					
			else {voice_speech(snjata_s_ohrani,PWM1) ;}
		}
  step_voice=0;
 
		voice_speech(nagatj,PWM1);
		voice_speech(cif_7,PWM1);
		voice_speech(pod_ohranoj,PWM1);
		delay_ms_dso(300);if(dso_vg){ break;}
		voice_speech(nagatj,PWM1);
		voice_speech(cif_8,PWM1);
		voice_speech(snjata_s_ohrani,PWM1);
		delay_ms_dso(300);if(dso_vg){ break;}
		voice_speech(nagatj,PWM1);voice_speech(cif_1,PWM1);
		voice_speech(vihod,PWM1);voice_out(1);
		delay_ms_dso(300);if(dso_vg) {break;}
		voice_speech(nagatj,PWM1);voice_speech(cif_2,PWM1);
		voice_speech(vihod,PWM1);voice_out(2);
		delay_ms_dso(300);if(dso_vg) {break;}
		voice_speech(nagatj,PWM1);voice_speech(cif_3,PWM1);voice_speech(audiovhod,PWM1);
		delay_ms_dso(300);if(dso_vg) break;
		voice_speech(nagatj,PWM1);voice_speech(cif_4,PWM1);
		voice_speech(sistema,PWM1);
		delay_ms_dso(500);if(dso_vg) break;
		if(detekt_razg()!=2) return;				// если разговор оборвался  то выход
	}

	item_menu=set_dtmf();
	switch (item_menu-'0')
	{
	case 7:								//постановка на охрану
					init_datchik();						// сбросить датчики
					set_out_ohran(1);		//если выход 1-выход состояния то снять с охраны
					delay_s(1);
					if(check_zone_alarm(0)) 
							{led_inform=6;delay_s(2);	//++ если зоны не собраны то оповестить и выйти
							end_zone=last_zon;
							for (i=1;i<=end_zone;i++)
								{if(buf_dat[i].alarm){	voice_speech(trevoga,PWM1);voice_speech(datchik,PWM1);
														voice_speech(cif_0+i,PWM1);
														delay_ms_dso(300);
													 }
								}
							 led_inform=0;
							}
					else {
							
						  time_cnt_bezdejstvie=cnt_bezdejstvie=EE_READ(time_STAND)*60;
						  cnt_siren=EE_READ(time_SIREN)*60;siren_on(0);
						 	set_out_ohran(0);				//если выход 3-ваход состояния то оповестить 
							if (EE_READ(Conf_post_sn)==4) 			//если включен контроль входа постановки снятия то проконтродировать его изменение  5 секунд			
								{
									for (i=0;i<50;i++)				//пауза 5 секунд
									{
										delay_ms(100);
										// выяснить положение кнопки постановки снятия норма- под охраной
										if(buf_dat[0].sost!=(buf_dat[0].triger&0x0F))						//если включена индикация то 
											{
									 			EE_WRITE(system_status,1);led_inform=3;
												sw_opov_post_snyat=2;  //выставить флаг для оповещения
												sound_opoveshenie(1);
												break;
											}
									}
								}
								else
									 {	EE_WRITE(system_status,1);led_inform=3;
										sw_opov_post_snyat=2;  //выставить флаг для оповещения
										sound_opoveshenie(1);
									  }
						 }
			voice_sost_system();
			step_voice=1;
			sost_trevog=0;				// сбросить состояние тревоги
		break;
	case 8:	
		EE_WRITE(system_status,0);led_inform=0;siren_on(0);	//инверсия постановки- снятия
		set_out_ohran(1);		//если выход 3-ваход состояния то оповестить 

		sost_trevog=0;				// сбросить состояние тревоги
		sw_opov_post_snyat=1;		//выставить флаг для оповещения
		sound_opoveshenie(0);
		CNT_time_auto_set=60*(unsigned int)EE_READ(time_auto_set);
		voice_sost_system();
		step_voice=1;
		break;
	case 1:	
	case 2:	
		if(item_menu==duble_click)
			{
			if(check_vihod(item_menu-'0')) {work=0;}	//инверсия выхода
				else {work=1;}
			set_vihod((item_menu-'0'),work);
			}	
		voice_speech(vihod,PWM1);
		voice_out(item_menu-'0');
		if (!check_vihod(item_menu-'0')) voice_speech(vikl,PWM1);
			else
				{ voice_speech(vkl,PWM1);
					if (!EE_READ(konf_VIHOD+item_menu-'0'-1))			// если сконфигурирован как 2 сек то через 2 сек отключить
						{	delay_s(2);set_vihod((item_menu-'0'),0);
							voice_speech(vihod,PWM1);
							voice_out(item_menu-'0');
							voice_speech(vikl,PWM1);
						}
				}
		break;
	case 3:
				change_in(voice_in);
				delay_s_dso(1);
				voice_speech(audiovhod,PWM1);
				voice_speech(vkl,PWM1);
 				change_in(mik_in);
mikrofone_step:
			while(1)
				{
					delay_s_dso(4);if(dso_vg) break;
					if(detekt_razg()!=2) {change_in(voice_in);return;}			// если разговор оборвался  то выход
				}
					item_menu=set_dtmf();
				if((item_menu=='0')||(item_menu=='3'))  	// выход если возврат
					{
						change_in(voice_in);
						voice_speech(audiovhod,PWM1);
						voice_speech(vikl,PWM1);
					}
					else goto mikrofone_step;
			change_in(voice_in);
		break;
	case 4:
		menu_sistema(n_abon);
		break;
	default:
		delay_s(1);
		voice_speech(oshibka,PWM1);
		break;
	}
	duble_click=item_menu;
		if(detekt_razg()!=2) return;	// если разговор оборвался  то выход
 }

}

//-------------------------
void init_datchik(void)
{
char i,work,end_zone;
opros_dat_enable=0;
end_zone=last_zon;
for (i=0;i<=end_zone;i++)
{
	work=EE_READ(regim_napr+i);			// прочитать режим напряжения! норма
	buf_dat[i].triger=work;
	buf_dat[i].tek_sost=work;			// текущее состояние датчика
	buf_dat[i].sost=work;				// состояние датчика с задержкой
	const_dat_wait=9*(unsigned int)EE_READ(Save_const_dat_wait);
	buf_dat[i].wait=const_dat_wait;		// константа задержки на датчик
	buf_dat[i].hold=0;					//датчик разблокироватьзаблокирован
	buf_dat[i].konfig=EE_READ(konf_ZONE+i);	// выставить конфигурации датчиков
}
opros_dat_enable=1;
delay_ms(const_dat_wait*2);
}

//-----------------
void print_zone(void)
{char i;
	putst("AT+zonehold:");
for (i=1;i<=last_zon;i++)
	{
	putchhex(buf_dat[i].hold);
	putch(':');
	putchhex(buf_dat[i].sost);
	putch(':');
	putchhex(buf_dat[i].triger);
	putch(':');
	putchhex(buf_dat[i].konfig);
	putch('-');

	}
	putst("\r\n");
}
//-------------------

void check_datchik(void)
{
char infodat,regim,end_time;

post_snjat=EE_READ(Conf_post_sn);
if((post_snjat==1)||(post_snjat==3))					// режим 1 и 3 
	{
	if(buf_dat[0].triger&0xF0) 						//если есть сработка кнопки постановки снятия
		{
			if(buf_dat[0].sost==(buf_dat[0].triger&0x0F))		//пока не восстановится кнопка ждать
				{
				 buf_dat[0].triger&=0x0F;						// сбросить тригер в 0
				 glob_knopka_system=EE_READ(knopka_system);
				 if(glob_knopka_system) {infodat=0;}		//проинвертировать состояние системы
					else {infodat=1;}
				}
		}	
	}
	else   // режим 0 и 2
	{
	if(buf_dat[0].sost==(buf_dat[0].triger&0x0F)) {infodat=0;}	// выяснить положение кнопки постановки снятия норма- под охраной
		else{infodat=1;}
	}	

	glob_knopka_system=EE_READ(knopka_system);
auto_set:
if(glob_knopka_system!=infodat)				// еслии поменялось положение кнопки постановка- снятие
	{   
		EE_WRITE(knopka_system,infodat);
		glob_knopka_system=infodat;
		if(EE_READ(system_status)!=infodat)  //и оно не совпадает с положением системы то поменять положение системы
			{

				if(infodat)
						{
						 init_datchik();						// сбросить датчики
						delay_s(1);
						if(check_zone_alarm(0)) {led_inform=6;delay_s(2);return;}	//++ если зоны не собраны то оповестить и выйти
						cnt_secund=0;
						end_time=EE_READ(time_VIHOD);			//если взяли под охрану задержка на выход
						led_inform=1;							// пошла задержка
							do{
								if(check_zone_alarm(2)) 		// проверить нет ли непроходных зон в тревоге
											{led_inform=6;delay_s(2);return;}//если есть хоть одна сработанная непроходная- не 
							//	init_OK_zone();	// взять под охрану восстановившиеся	
								delay_ms(100);
								}while (cnt_secund<end_time);
						}
				time_cnt_bezdejstvie=cnt_bezdejstvie=EE_READ(time_STAND)*60;
				cnt_siren=EE_READ(time_SIREN)*60;
				siren_on(0);						//выключить сирену
				sost_trevog=0;				// сбросить состояние тревоги
				clear_zone_alarm(0);		// сбросить тревожные метки
				link_out_SET(1);			// сбросить выходы на  зонах
				EE_WRITE(system_status,infodat);
				init_datchik();
				sw_opov_post_snyat=infodat+1;		//выставить флаг для оповещения
				sound_opoveshenie(infodat);

			}
	}
if (!EE_READ(system_status)) 
				{				// если не под охраной то контролировать круглосуточные
				led_inform=0;
				regim=1;
				set_out_ohran(1);		//если выход 3-ваход состояния то оповестить 
				if(EE_READ(time_auto_set))		// если включена автопостановка на охрану
						{
						if(check_zone_alarm(0))	{CNT_time_auto_set=60*(unsigned int)EE_READ(time_auto_set);init_datchik();}	// если есть зоны в тревоге то обновить счетчик 
							else {if(!CNT_time_auto_set){infodat=1;glob_knopka_system-0;CNT_time_auto_set=60*(unsigned int)EE_READ(time_auto_set); goto  auto_set;}}
						}
				}	
		else 
			{
			if(sost_trevog) {led_inform=2;}	// если были тревоги то мигать
					else {led_inform=3;}		// если подж охраной то все
			regim=0;
			set_out_ohran(0);		//если выход 3-ваход состояния то оповестить 
			}
								//
clear_zone_alarm(0);						// сбросить тревожные метки

if(!check_zone_alarm(regim)) 		// проверить нет ли зон в тревоге
				{
				if(check_zone_alarm(4))			//  если есть восстановившиеся то выставить метки для оповещения по восстановившемся
					{	
					link_out_SET(0);			// сбросить выходы на восстановившихся зонах
					if(EE_READ(Opov_Vostan))		
						{ 
						led_inform=6;
						alarm_dozvon(4);	// оповестить о восстановившихся
						}
					}
				init_OK_zone();	// взять под охрану восстановившиеся
				return;		
				}		

if((buf_dat[1].alarm)&&(!regim))		// если мы под охраной  и сработан 1й датчик то отработать задержку
			{
			cnt_secund=0;
			end_time=EE_READ(time_VHOD);			//если 1 я зона то задержка на вход 
				led_inform=1;							// пошла задержка
				clear_zone_alarm(2);			//сбросить входные- проходные
					do{
						if(check_zone_alarm(2)) 		// проверить нет ли непроходных зон в тревоге
									{break;}//если есть хоть одна сработанная непроходная- общая тревога
						init_OK_zone();	// взять под охрану восстановившиеся
						
					if((post_snjat==1)||(post_snjat==3))					// режим 1 и 3 
						{
						if(buf_dat[0].triger&0xF0) 						//если есть сработка кнопки постановки снятия
							{
							if(buf_dat[0].sost==(buf_dat[0].triger&0x0F))		//пока не восстановится кнопка ждать
								{
								 buf_dat[0].triger&=0x0F;						// сбросить тригер в 0
				 					glob_knopka_system=EE_READ(knopka_system);
				 					if(glob_knopka_system) {infodat=0;}		//проинвертировать состояние системы
										else {infodat=1;}
								}
							}	
						}
					else   // режим 0 и 2
						{
						if(buf_dat[0].sost==(buf_dat[0].triger&0x0F)) {infodat=0;}	// выяснить положение кнопки постановки снятия норма- под охраной
														else{infodat=1;}
						}

						if(EE_READ(knopka_system)!=infodat)				// еслии поменялось положение кнопки постановка- снятие
							{   
								if(infodat) 	 //и пытаются поставить на охрану то запомнить новое положение
										{EE_WRITE(knopka_system,infodat);}
									else {return;}	//если сняли с охраны то стоп
							}
						delay_ms(100);
						}while (cnt_secund<end_time);
				check_zone_alarm(regim);				// вернуть тревоги на входную и проходные зоны зону
			}

link_out_SET(1);
led_inform=6;
siren_on(1);				// включить сирену
alarm_dozvon(regim);						//если есть хоть одна сработанная неоповещенная- общая тревога

}
//-----------------
void init_OK_zone(void)
{char i,end_zone;
//fl_sv_trig=0;
	end_zone=last_zon;
for (i=1;i<=end_zone;i++)
	{
			// все зоны
	if(buf_dat[i].konfig==3) // если зона бездействия то восстановлениее считается тревога
		{if((buf_dat[i].hold)&&((buf_dat[i].triger&0x0F)!=buf_dat[i].sost)){buf_dat[i].hold=0;buf_dat[i].triger&=0x0F;cnt_bezdejstvie=time_cnt_bezdejstvie; } } 
		else{if((buf_dat[i].hold)&&((buf_dat[i].triger&0x0F)==buf_dat[i].sost)) {buf_dat[i].hold=0;buf_dat[i].triger&=0x0F;/*fl_sv_trig=1;*/}}	// нашли восстановленный - пометить
	}
}
//------------------
void clear_zone_alarm(char regim)
{char i,end_zone;
	end_zone=last_zon;
for (i=1;i<=end_zone;i++)
{
 switch(regim)
	{
	case 0:						//	 0 - все 
		buf_dat[i].alarm=0;
	break;
	case 2:						//	 2 - очистить все входные- проходные
		if(buf_dat[i].konfig==1){buf_dat[i].alarm=0;}
	break;
	case 3:						//	 3 - очистить все бездействия
		if(buf_dat[i].konfig==3){buf_dat[i].alarm=0;}
	break;
	default:
	break;
	}
}
}
//-------------------
char check_zone_alarm(char regim)		// проверить и выставить зоны в состоянии тревоги возврат 1 - тревога 0 - норма
										// параметр-т 0 - все не заблокированные 
										// 			  1	- контроль круглосуточных зон
{										//			  2 - мгновенные+ круглосуточные
char i,ret_chr,end_zone;							//			  3 - контроль зон бездействия
ret_chr=0;								//			  4- контроль восстановившихся зон
										//			  5- контроль зон c авварией
end_zone=last_zon;
for (i=1;i<=end_zone;i++)
{	
if (buf_dat[i].konfig&0x80)	continue;	// выделить бит блокировки датчика, если заблокирован, то пропустить опрос датчика	
if (buf_dat[i].alarm) {ret_chr=1; continue;}			//если метка есть - то пропустить	
 switch(regim)
	{
	case 0:						//	 0 - все не заблокированные		
		if(buf_dat[i].konfig==3){if((buf_dat[i].triger&0xF0)&&(!buf_dat[i].hold)&&cnt_bezdejstvie)	// 		3 - зоны бездействия
										{cnt_bezdejstvie=time_cnt_bezdejstvie;buf_dat[i].triger&=0x0F;}		// была сработка- дальше
									else{buf_dat[i].alarm=1;ret_chr=1;}		// не было- тревога 
								}
				else if((buf_dat[i].triger&0xF0)&&(!buf_dat[i].hold)){buf_dat[i].alarm=1;ret_chr=1;}	// нашли тревогу- пометить
	break;
	case 1:					//	 1	- контроль круглосуточных зон			
	if((buf_dat[i].triger&0xF0)&&(!buf_dat[i].hold)&&(buf_dat[i].konfig==2)){buf_dat[i].alarm=1;ret_chr=1;}	// нашли тревогу- пометить
	break;
	case 2:						// 		2 - мгновенные+ круглосуточные
	if((buf_dat[i].triger&0xF0)&&(!buf_dat[i].hold)&&((buf_dat[i].konfig==2)||(buf_dat[i].konfig==0))){buf_dat[i].alarm=1;ret_chr=1;}	// нашли тревогу- пометить
	break;
	case 3:						// 		3 - зоны бездействия
	if(buf_dat[i].konfig==3){if((buf_dat[i].triger&0xF0)&&(!buf_dat[i].hold)&&cnt_bezdejstvie)	// 		3 - зоны бездействия
										{cnt_bezdejstvie=time_cnt_bezdejstvie;buf_dat[i].triger&=0x0F;}		// была сработка- дальше
									else{buf_dat[i].alarm=1;ret_chr=1;}		// не было- тревога 
								}
	break;
	case 4:						// 		4 - восстановившиися зоны
	if((buf_dat[i].hold)&&((buf_dat[i].triger&0x0F)==buf_dat[i].sost)) {buf_dat[i].alarm=1;ret_chr=1;}	// нашли восстановленный - пометить
	break;
	default:
	break;
	}
}
return ret_chr;
}
//---------------
void end_alarm_dozvon(char regim)			// дозвон закончен
{
char i,fl_save_rsh,end_zone;
fl_save_rsh=0;
end_zone=last_zon;
for (i=1;i<=end_zone;i++)
	{		
if (!buf_dat[i].alarm) { continue;}			//если метка нет - то пропустить
	buf_dat[i].alarm=0;						//сбросить метку	
 switch(regim)
	{
	case 0:						//	 0 - все не заблокированные		
	buf_dat[i].hold=1;			// нашли тревогу- заблокировать
	break;
	case 1:						//	  1	- контроль круглосуточных зон			
	if(buf_dat[i].konfig==2){buf_dat[i].hold=1;}			// нашли тревогу- заблокировать}
	break;
	case 2:						// 		2 - мгновенные+ круглосуточные
	if((buf_dat[i].konfig==2)||(!buf_dat[i].konfig)){buf_dat[i].hold=1;}			// нашли тревогу- заблокировать}
	break;
	case 3:						// 		3 - зоны бездействия
	if(buf_dat[i].konfig==3){buf_dat[i].hold=1;}			// нашли тревогу- заблокировать}
	break;
	case 4:						// 		4 - восстановившиися зоны
	buf_dat[i].hold=0;			// нашли восстановленный - обнулить(взять под охрану)
	buf_dat[i].triger&=0x0F;
	fl_save_rsh=1;
	break;
	default:
	break;
	}
	}	
}

//------------------------------
void alarm_dozvon(char regim)			//обработка тревоги 0- общая тревога 1- тревога круглосуточных зон
{
										//0	По данному номеру SMS не отсылается
										//1	По данному номеру SMS отсылается после дозвона в случае, если система не смогла дозвониться ни по одному из номеров списка
										//2	По данному номеру SMS отсылается после дозвона в любом случае
										//3	По данному номеру SMS отсылается в первую очередь, затем дозвон
										//4	По данному номеру отсылается только SMS

char i,n_abon,count_dozvon,count_SMS,sms_regim;
sost_trevog=1;				// установить  состояние тревоги
for (i=0;i<16;i++){map_dozvon[i]=0;}	// очистить карту дозвона
count_dozvon=EE_READ(lim_DOZVON);		// колличество циклов дозвона
count_SMS=EE_READ(lim_SMS);				// прочитать колличество СМС на номер
fl_uspeh=0;								// бит успешного отчета
	while(count_dozvon--)
	{
	for(n_abon=0;n_abon<10;n_abon++)
	 {
		if (map_dozvon[n_abon]) continue;		// если номер исключен из дозвона - следующий
		sms_regim=read_regim(n_abon);
		if((Bufer_SPI[2]==0xFF)||(Bufer_SPI[2]==0x00)) {map_dozvon[n_abon]=1;continue;}		//если номера нет! пропустить 
		if(sms_regim=='3') snd_alarm_sms(n_abon,regim);	// если режим сначала SMS то отправить смс
		if(sms_regim!='4') if(snd_alarm_voice(n_abon,regim)){end_alarm_dozvon(regim);return;} // дозвонится голосом если дозвон прервали! выход
   	 }
	}
 while (count_SMS--)
	{
		for(n_abon=0;n_abon<10;n_abon++)
	 {
		sms_regim=read_regim(n_abon);
		if((Bufer_SPI[2]==0xFF)||(Bufer_SPI[2]==0x00)) {continue;}		//если номера нет! пропустить 
		switch(sms_regim)
		{
		case '0':		//	По данному номеру SMS не отсылается
				break;
		case '1':		// если нет успеха
				if(!fl_uspeh) snd_alarm_sms(n_abon,regim);
				break;
		case '2':		//		По данному номеру SMS отсылается после дозвона в любом случае
				snd_alarm_sms(n_abon,regim);
				break;
		case '3':
				break;	// если режим сначала SMS то следующий
		case '4':		//По данному номеру отсылается только SMS
			 snd_alarm_sms(n_abon,regim);
			break;
		default:
			break;
		}
   	 }

	}
	end_alarm_dozvon(regim);			// дозвон закончен
}

//----------------
char snd_alarm_voice(char n_abon,char regim)				//тревожный дозвон результат 1- прервал администратор, 0 все нормально
{
 char lim_time,i,work,end_zone;
	putst("ATH\r\n");
	delay_s(1);
  	call_abon(n_abon);
	delay_s(3);
	if(!wait_razg(30)) {return 0;}
	//дозвонились
 	delay_ms(300);
	 putst("AT+CHFA=0\r");
 	delay_ms(300);
 	change_in(voice_in);
	fl_uspeh=1;	
	lim_time=120;			//звонок максимум 2 минуты
	cnt_secund=0;
	while(cnt_secund<lim_time)
	{
	check_zone_alarm(regim);				//выставить сработанные зоны
											// отчитаться
end_zone=last_zon;
	for (i=1;i<=end_zone;i++)
		{		
		if(!buf_dat[i].alarm)continue;		// если зона не в тревоге дальше
		if(regim!=4){voice_alarm(i,0,0);}		// озвучить тревогу на зоне в соответствии с конфигом
			else {voice_alarm(i,0,1);}		// озвучить восстановление в зоне в соответствии с конфигом	
		if(dso_vg)	// если нажата кнопка 0 то в меню
			{
			if((cod_dtmf()==0x0A)||(cod_dtmf()==0b00001011)||(cod_dtmf()==0b00001100))
			 {
			work=menu_trevoga(n_abon);
			if(!work) {cnt_secund=0;continue;}			//кнопка не опознана или правильный возврат по 0
			if(work==1) 		// исключили из списка
				{
				map_dozvon[n_abon]=1;
				putst("ATH\r");
				return 0;
				}
			if(work==2) 		// прервали разговор
				{
				putst("ATH\r");
				return 1;
				}
			 }
			}
	delay_ms_dso(300);
		}
	delay_s_dso(2);
	if(detekt_razg()!=2) break;
	}
	putst("ATH\r");						// время вышло
	return 0;
}
//------------------------------------------- 
void voice_alarm(char num_zone,char map_alarm,char type_alarm)   // map_alarm переменная какую таблицу использовать ( откуда вызов тревога (0) или Contact ID(1))
{
char konf;
if(!map_alarm)
	{if(num_zone<16){konf=EE_READ(konf_VOICE+num_zone);}else {konf=0;}}
	else {if(num_zone<32){konf=EE_READ(konf_VOICE_Cid+num_zone);}else {konf=0;}}		

if (konf<40) 
		{
		if(!type_alarm) voice_speech(trevoga,PWM1);
		if(type_alarm==2) voice_speech(avarija,PWM1);
		voice_speech(datchik,PWM1);
		}
		else 
		{
		if(type_alarm==1)	{voice_speech(datchik,PWM1); voice_speech(ispraven,PWM1);}
		}		
 switch(konf)
 {
	case 0:
			if(num_zone<20){voice_speech(cif_0+num_zone,PWM1);break;}
			if(num_zone<30){voice_speech(cif_20,PWM1);voice_speech(num_zone-20+cif_0,PWM1);break;}
			if(num_zone<40){voice_speech(cif_30,PWM1);voice_speech(num_zone-30+cif_0,PWM1);break;}
			if(num_zone<50){voice_speech(cif_40,PWM1);voice_speech(num_zone-40+cif_0,PWM1);break;}
			if(num_zone<60){voice_speech(cif_50,PWM1);voice_speech(num_zone-50+cif_0,PWM1);break;}
			if(num_zone<70){voice_speech(cif_60,PWM1);voice_speech(num_zone-60+cif_0,PWM1);break;}
			if(num_zone<80){voice_speech(cif_70,PWM1);voice_speech(num_zone-70+cif_0,PWM1);break;}
			if(num_zone<90){voice_speech(cif_80,PWM1);voice_speech(num_zone-80+cif_0,PWM1);break;}
			if(num_zone<100){voice_speech(cif_90,PWM1);voice_speech(num_zone-90+cif_0,PWM1);break;}
			if(num_zone<120){voice_speech(cif_100,PWM1);voice_speech(num_zone-100+cif_0,PWM1);break;}
			if(num_zone<130){voice_speech(cif_100,PWM1);voice_speech(cif_20,PWM1);voice_speech(num_zone-120+cif_0,PWM1);break;}
			voice_speech(oshibka,PWM1);			
		break;
	case 1:
			voice_speech(dvigenija,PWM1);		//движения
		break;
	case 2:
			voice_speech(proniknoveniya,PWM1);		//проникновения
		break;
	case 3:
			voice_speech(napadenie,PWM1);		//нападение
		break;
	case 4:
			voice_speech(vodi,PWM1);			//воды
		break;
	case 5:
			voice_speech(pogara,PWM1);			//пожара
		break;
	case 6:
			voice_speech(dima,PWM1);			//дыма
		break;
	case 7:
			voice_speech(gaza,PWM1);			//газа
		break;
	case 8:
			voice_speech(sireni,PWM1);			// сирены
		break;
	case 9:
			voice_speech(klaviaturi,PWM1);		//клавиатуры
		break;
	case 10:
			voice_speech(dveri,PWM1);			//двери
		break;
	case 11:
			voice_speech(okna,PWM1);			//окна
		break;
	case 12:
			voice_speech(narug_perimetr,PWM1);	//наружный периметр
		break;
	case 13:
			voice_speech(fasad,PWM1);			//фасад
		break;
	case 14:
			voice_speech(til,PWM1);			 //тыл
		break;
	case 15:
			voice_speech(etaj,PWM1);			//Этаж1
			voice_speech(cif_1,PWM1);
		break;
	case 16:
			voice_speech(etaj,PWM1);			 //Этаж2
			voice_speech(cif_2,PWM1);
		break;
	case 17:
			voice_speech(zal,PWM1);			//зал
		break;
	case 18:
			voice_speech(spalnya,PWM1);			//спальня
		break;
	case 19:
			voice_speech(prihogaya,PWM1);	//прихожая
		break;
	case 20:
			voice_speech(kuhnya,PWM1);			//кухня
		break;
	case 21:
			voice_speech(sklad,PWM1);			 //тыл
		break;
	case 40:
			voice_speech(Srab_ohran_sig,PWM1);			//Сработала охранная сигнализация
		break;
	case 41:
			voice_speech(Srab_pojag_sig,PWM1);			//Сработала пожарная сигнализация
		break;
	case 51:
	case 52:
	case 53:
	case 54:
	case 55:
	case 56:
	case 57:
	case 58:
	case 59:
	case 60:
	case 61:
	case 62:
	case 63:
	case 64:
	case 65:
	case 66:
			voice_play((konf-50),PWM1);		// записанные сообщения
		break;
	default:
		break;
 }
if (konf<40) 
		{
		if(type_alarm==1) voice_speech(ispraven,PWM1);
		}
}
//--------------------------------------------------
char menu_trevoga(char n_abon)			//меню в состоянии тревоги результат 0 нормальный выход 
										//1-абонент исключен из списка 2- прервано администратором
{
char item_menu,end_zone,i,duble_click,work;
char frst_vhod=0;			//метка первого входа
	delay_s_dso(1);
	if (read_kateg(n_abon)>'2')						// если обыкновенные пользователи то искать только звездочку
		{
			if(set_dtmf()==zirochka) 	{return 1;}
				else {	delay_s(1);
						voice_speech(oshibka,PWM1);
						return 0;
					 }
		}
duble_click=0;
step_voice=0;
 while(1)
 {
	while(1)
	{
	if((!step_voice)&&EE_READ(Voice_otchet))								// если нужно то пропустить оповещение
		{
		voice_speech(sistema,PWM1);
		if (EE_READ(system_status))
				{voice_speech(pod_ohranoj,PWM1);
				if(sost_trevog){delay_ms_dso(500);voice_speech(trevoga,PWM1);}
				}					
			else {voice_speech(snjata_s_ohrani,PWM1) ;}
		}
  step_voice=0;
		voice_speech(nagatj,PWM1);voice_speech(cif_0,PWM1);voice_speech(vozvrat,PWM1);
		delay_ms_dso(500);if(dso_vg) break;
		voice_speech(nagatj,PWM1);voice_speech(cif_7,PWM1);voice_speech(pod_ohranoj,PWM1);
		delay_ms_dso(300);if(dso_vg){ break;}
		voice_speech(nagatj,PWM1);voice_speech(cif_8,PWM1);voice_speech(snjata_s_ohrani,PWM1);
		delay_ms_dso(300);if(dso_vg){ break;}
		voice_speech(nagatj,PWM1);voice_speech(cif_1,PWM1);voice_speech(vihod,PWM1);voice_out(1);
		delay_ms_dso(300);if(dso_vg) {break;}
		voice_speech(nagatj,PWM1);voice_speech(cif_2,PWM1);voice_speech(vihod,PWM1);voice_out(2);
		delay_ms_dso(300);if(dso_vg) {break;}
		voice_speech(nagatj,PWM1);voice_speech(cif_3,PWM1);voice_speech(audiovhod,PWM1);
		delay_ms_dso(300);if(dso_vg) break;
		voice_speech(nagatj,PWM1);voice_speech(cif_4,PWM1);voice_speech(sistema,PWM1);
		delay_ms_dso(500);if(dso_vg) break;
		if(detekt_razg()!=2) {goto end_menu;}				// если разговор оборвался  то выход
	}
	item_menu=set_dtmf();
	switch (item_menu-'0')
	 {
	case 0:	
		if(!frst_vhod) break;				// если первый взход то промолчать
		goto end_menu;				// нет- возврат
	//	break;
	case 7:								//постановка на охрану
					init_datchik();						// сбросить датчики
					set_out_ohran(1);		//если выход 1-выход состояния то снять с охраны
					delay_s(1);
					if(check_zone_alarm(0)) 
							{led_inform=6;delay_s(2);	//++ если зоны не собраны то оповестить и выйти
							end_zone=last_zon;
							for (i=1;i<=end_zone;i++)
								{if(buf_dat[i].alarm){	voice_speech(trevoga,PWM1);voice_speech(datchik,PWM1);
														voice_speech(cif_0+i,PWM1);
														delay_ms_dso(300);
													 }
								}
							 led_inform=0;
							}
					else {
							
						  time_cnt_bezdejstvie=cnt_bezdejstvie=EE_READ(time_STAND)*60;
						  cnt_siren=EE_READ(time_SIREN)*60;siren_on(0);
						 	set_out_ohran(0);				//если выход 3-ваход состояния то оповестить 
							if (EE_READ(Conf_post_sn)==4) 			//если включен контроль входа постановки снятия то проконтродировать его изменение  5 секунд			
								{
									for (i=0;i<50;i++)				//пауза 5 секунд
									{
										delay_ms(100);
										// выяснить положение кнопки постановки снятия норма- под охраной
										if(buf_dat[0].sost!=(buf_dat[0].triger&0x0F))						//если включена индикация то 
											{
									 			EE_WRITE(system_status,1);led_inform=3;
												sw_opov_post_snyat=2;  //выставить флаг для оповещения
												sound_opoveshenie(1);
												break;
											}
									}
								}
								else
									 {	EE_WRITE(system_status,1);led_inform=3;
										sw_opov_post_snyat=2;  //выставить флаг для оповещения
										sound_opoveshenie(1);
									  }
						 }
			voice_sost_system();
			step_voice=1;
			sost_trevog=0;				// сбросить состояние тревоги
		break;
	case 8:	
		EE_WRITE(system_status,0);led_inform=0;siren_on(0);	//инверсия постановки- снятия
		set_out_ohran(1);		//если выход 3-ваход состояния то оповестить 

		sost_trevog=0;				// сбросить состояние тревоги
		sw_opov_post_snyat=1;		//выставить флаг для оповещения
		sound_opoveshenie(0);
		CNT_time_auto_set=60*(unsigned int)EE_READ(time_auto_set);
		voice_sost_system();
		step_voice=1;
		break;
	case 1:	
	case 2:	
		if(item_menu==duble_click)
			{
			if(check_vihod(item_menu-'0')) {work=0;}	//инверсия выхода
				else {work=1;}
			set_vihod((item_menu-'0'),work);
			}	
		voice_speech(vihod,PWM1);
		voice_out(item_menu-'0');
		if (!check_vihod(item_menu-'0')) voice_speech(vikl,PWM1);
			else
				{ voice_speech(vkl,PWM1);
					if (!EE_READ(konf_VIHOD+item_menu-'0'-1))			// если сконфигурирован как 2 сек то через 2 сек отключить
						{	delay_s(2);set_vihod((item_menu-'0'),0);
							voice_speech(vihod,PWM1);
							voice_out(item_menu-'0');
							voice_speech(vikl,PWM1);
						}
				}
		break;
	case 3:
				change_in(voice_in);
				delay_s_dso(1);
				voice_speech(audiovhod,PWM1);
				voice_speech(vkl,PWM1);
 				change_in(mik_in);
mikrofone_step1:
			while(1)
				{
					delay_s_dso(4);if(dso_vg) break;
					if(detekt_razg()!=2) {change_in(voice_in);return 0;}			// если разговор оборвался  то выход
				}
					item_menu=set_dtmf();
				if((item_menu=='0')||(item_menu=='3'))  	// выход если возврат
					{
						change_in(voice_in);
						voice_speech(audiovhod,PWM1);
						voice_speech(vikl,PWM1);
					}
					else goto mikrofone_step1;
			change_in(voice_in);
		break;
	case 4:
		menu_sistema(n_abon);
		break;
	case zirochka-'0':
		return 1;
	//	break;

	case rehetka-'0':
		return 2;
	//	break;

	default:
		delay_s(1);
		voice_speech(oshibka,PWM1);
		break;
	 }
	frst_vhod=1;
	duble_click=item_menu;
		if(detekt_razg()!=2) {goto end_menu;}	// если разговор оборвался  то выход
 }
end_menu:
	if (EE_READ(system_status)){return 0;}
		else return 2;		//если сняли с охраны то стоп прозвон
}

//----------------------------------------------
void sound_opoveshenie(unsigned char infodat)
{
char post_snjat;
post_snjat=EE_READ(Conf_post_sn);
if((post_snjat==2)||(post_snjat==3))				// режим 2 и 3 и специального назначения то 
	{
		set_out_siren(1);											//пропикать
		delay_ms(400);
		set_out_siren(0);
		delay_ms(500);
		if(!infodat)
			{
			set_out_siren(1);
			delay_ms(400);
			set_out_siren(0);
			delay_ms(500);
			}
	}
}
//-------------------------------------------------------------
void opov_postanovka_snjatie (unsigned char type_oth)			// оповещение о постановке- снятии
{
unsigned char type_opov,lim_time;

type_opov=EE_READ(otchet_postan);
if(!type_opov)	return;			// если не отчитыватьс то сразу вернуться

opoveshenie_slugebnoe(post_sn_kod,type_oth,type_opov);  // отчитаться

}
//----------------------------------------------
void send_TEST(void)						//отправка тестового сообщения
{
char type_opov,lim_time,i;
type_opov=EE_READ(regim_TEST);
if(!type_opov)	return;			// если не отчитывать то сразу вернуться
opoveshenie_slugebnoe(test_kod,type_opov,type_opov);  // отчитаться

}
//--------------------
void send_opov_napr(void)
{
char type_opov,lim_time,tmp1;
type_opov=EE_READ(regim_BORT_napr);
if(!type_opov)	return;			// если не отчитыватьс то сразу вернуться

opoveshenie_slugebnoe( napr_kod,fl_bort_napr,type_opov);  // отчитаться


}
//--------------------
void send_opov_temperatura(unsigned char conf_dallas,unsigned char type_opov,unsigned char CH_DL)		// тип сообщения 0-не оповещается о порогах, 1- SMS, 2 голосовое сообщение 
{
unsigned char temp,lim_time,item_menu;
static signed char temp_low,temp_high,temp_gist;
//res_eep=0;							// подключить датчик DS
delay_ms(100);
temperatura_dallas[CH_DL-1]=read_temp_dallas(CH_DL); 
if(temperatura_dallas[CH_DL-1]==0x7F) {delay_s(1);temperatura_dallas[CH_DL-1]=read_temp_dallas(CH_DL);}  //контроль при ошибке 2 раза 
//res_eep=1;

temp_low=EE_READ(TEMP_LOW+CH_DL-1);
temp_high=EE_READ(TEMP_HIGH+CH_DL-1);
temp_gist=EE_READ(TEMP_GIST+CH_DL-1);
	//анализ предыдущего состояния

		switch(temperatura_save[CH_DL-1])
			{
				case 0:							//если была нижняя граница
						temp=0;
						if(temperatura_dallas[CH_DL-1]>=(temp_low+temp_gist)) {temp=1;}
						if(temperatura_dallas[CH_DL-1]>temp_high) {temp=2;}
						if(temperatura_dallas[CH_DL-1]==0x7F) {temp=0x7F;}
						break;
				case 1:
						temp=1;
						if(temperatura_dallas[CH_DL-1]<temp_low) {temp=0;}
						if(temperatura_dallas[CH_DL-1]>temp_high) {temp=2;}
						if(temperatura_dallas[CH_DL-1]==0x7F) {temp=0x7F;}
						break;
				case 2:
						temp=2;
						if(temperatura_dallas[CH_DL-1]<(temp_high-temp_gist)) {temp=1;}
						if(temperatura_dallas[CH_DL-1]<temp_low) {temp=0;}
						if(temperatura_dallas[CH_DL-1]==0x7F) {temp=0x7F;} 
						break;
				default:
						temp=0xFF;
						if(temperatura_dallas[CH_DL-1]<temp_low) {temp=0;}
						if(temperatura_dallas[CH_DL-1]>=temp_low) {temp=1;}
						if(temperatura_dallas[CH_DL-1]>temp_high) {temp=2;}
						if(temperatura_dallas[CH_DL-1]==0x7F) {temp=0x7F;}
						break;
			}


if(temp==temperatura_save[CH_DL-1]) return;				//если температура не изменилась - возврат
 												//если изменились параметры- оповестить
temperatura_save[CH_DL-1]=temp;

opoveshenie_slugebnoe((temperatura_kod1+CH_DL-1),temperatura_save[CH_DL-1],type_opov);  // отчитаться


}

//----------------

void opoveshenie_slugebnoe(unsigned char cod_opovesheniya,unsigned char type_oth, unsigned char type_opov)    // оповещение служебными сообщениями код сообщения и тип сообщения и способ оповещения(1- SMS,2-голос)
{

char i,n_abon,count_dozvon,kateg,lim_time,item_menu,fl_stop;
for (i=0;i<10;i++){map_dozvon[i]=0;}	// очистить карту дозвона
if(type_opov==2){count_dozvon=EE_READ(lim_DOZVON);	}	// колличество циклов дозвона
	else {count_dozvon=1;}
led_inform=6;
	while(count_dozvon--)
	{
	for(n_abon=0;n_abon<10;n_abon++)
	 {
		if (map_dozvon[n_abon]) continue;		// если номер исключен из дозвона - следующий
		kateg=read_kateg(n_abon);
		if((Bufer_SPI[2]==0xFF)||(Bufer_SPI[2]==0x00)) {map_dozvon[n_abon]=1;continue;}		//если номера нет! пропустить 
		if(kateg=='0')
			{
				if((type_opov==1)||(type_opov==3)) {send_sms_slugeb(cod_opovesheniya,type_oth,n_abon);continue;}
				if(type_opov==2)
						{
							putst("ATH\r\n");
							delay_s(1);
  							call_abon(n_abon);
							delay_s(3);
							if(!wait_razg(30)) {continue;}
							//дозвонились
 							delay_ms(300);
	 						putst("AT+CHFA=0\r");
 							delay_ms(300);
 							change_in(voice_in);
							fl_uspeh=1;	
							lim_time=120;			//звонок максимум 2 минуты
							cnt_secund=0;
							while(cnt_secund<lim_time)
								{
											// отчитаться
									voice_slugeb(cod_opovesheniya,type_oth);
								if(dso_vg)	// если нажата кнопка 0 то в меню
									{
										item_menu=set_dtmf();
										if ((item_menu==zirochka)||(item_menu==rehetka))
										{
										putst("ATH\r");
										delay_s(2);
										return;
										}
									}
								delay_s_dso(2);
								if(detekt_razg()!=2) break;
								}
						}
			}
		}
	}	
}
//----

void voice_slugeb(unsigned char cod_opovesheniya,unsigned char type_oth)
{
unsigned char CH_DL;
switch (cod_opovesheniya)
	{
	case post_sn_kod:
				voice_speech(sistema,PWM1);			//
				if(type_oth)	{voice_speech(pod_ohranoj,PWM1);}
					else {voice_speech(snjata_s_ohrani,PWM1);}
			
		break;
	case test_kod:
				voice_speech(sistema,PWM1);			//
				voice_speech(test,PWM1);
		break;
	case napr_kod:
				if(type_oth){voice_speech(setevoe_n,PWM1);voice_speech(ispraven,PWM1);}
					else {voice_speech(avarija,PWM1);voice_speech(setevoe_n,PWM1);}
		break;
	case temperatura_kod1:
	case temperatura_kod2:
		CH_DL=cod_opovesheniya-temperatura_kod1+1;
		switch(type_oth)
			{
				case 0:
						voice_speech(vnimanie,PWM1);voice_speech(ponigenie,PWM1);voice_speech(temperaturi,PWM1);voice_speech(datchik,PWM1);voice_speech(cif_0+CH_DL,PWM1);
					break;
				case 1: 
						voice_speech(datchik,PWM1);voice_speech(temperaturi,PWM1);voice_speech(cif_0+CH_DL,PWM1);voice_speech(ispraven,PWM1);
					break;
				case 2:
						voice_speech(vnimanie,PWM1);voice_speech(povishenie,PWM1);voice_speech(temperaturi,PWM1);voice_speech(datchik,PWM1);voice_speech(cif_0+CH_DL,PWM1);
					break;
				default:
						voice_speech(avarija,PWM1);voice_speech(datchik,PWM1);voice_speech(temperaturi,PWM1);voice_speech(cif_0+CH_DL,PWM1);
					break;
			}
				
		break;
	default:
		break;
	}
}
//----------------------------------------------
void set_zone_int(char n_zone)
{
switch(n_zone)
 {
	case zona_ohran:
			ADCON0=0b00110001;
			break;
	case zona_1:
			ADCON0=0b00101001;
			break;
	case zona_2:
			ADCON0=0b00100001;   //AN8
			break;
	case in_voice_rec:
			ADCON0=0b00011101;
			break;
	case in_trub:
			ADCON0=0b00101101;
			break;
	default:
		break;
  }
}
