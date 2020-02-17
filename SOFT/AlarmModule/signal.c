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


unsigned char fl_uspeh,step_voice;									// ����  ��������� ������
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
unsigned char hold;				// ���� �����������
unsigned char triger;			//� ������� ������� ��������- ���������� ��������� ������� �������
unsigned char alarm;				// ���� � ��������� �������
unsigned char konfig;			// ������������ ���� 0-������� 1-�������-���������	2-��������������,3-���� �����������
}

extern  struct info_datchik buf_dat[];		// 4������� 0- ���� ���������� ������

unsigned char map_dozvon[16];					// ����� ��������


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
if (!mask_out_siren) {fl_count_siren=0;return;}	// ���� �� ������������ ���������� �� �������
if(!regim){fl_count_siren=0;set_out_siren(0);return;}
	{
		cnt_siren=EE_READ(time_SIREN)*60;set_out_siren(1); // �������
		fl_count_siren=1;
	}
}
//----------------------------
void link_out_SET (unsigned char set_w)
{
unsigned char i,end_zone,link_temp,set_out[12];

end_zone=last_zon;
for (i=0;i<12;i++) set_out[i]=0;   // �������� ����� �������

for (i=1;i<=end_zone;i++)		   //����������� ������
	{
	link_temp=EE_READ(link_Zone_Vih+i);
	if((link_temp==0)||(link_temp>=12)) continue;  //������ �� ������
	if(buf_dat[i].alarm) {set_out[link_temp]=set_w;} // ��������� ��������� ������ � ������������ � ��������� ���������� ( ������� ��� ��������������)
	}

for (i=1;i<12;i++)					//��������� ������ ���� ��������� ������������
	{
	if(EE_READ(konf_VIHOD+i-1)==1) set_vihod(i,set_out[i]);
	}
		
}

//---------------------------------------
unsigned char check_in_ring(void)			//�������� ��������� ������
{
char work,i,j,n_abon;

 n_abon=check_numb();
 if(n_abon==0xff) {return 1;}			// �� ���� ����� �� ������- �������
 delay_ms(500);
 put_command("ATA\r");
 read_lexem(1,30);		// ��������� 1 ������� 30 ���
 if (uncode_com("CONNECT")) {terminal_PDP();return 0;}	//������ �������
 if (!uncode_com("OK")){putst("+++");delay_s(1);putst("\rATH\r");return 0;}//e��� ����� �� �� �����		
 delay_s(1);
 putst("AT+CHFA=0\r");
 delay_ms(300);
 change_in(voice_in);
if(EE_READ(Menu_pass))				//���� ������� ������ ���� �������
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
					}	// ��� �������  - �� ����
		}
	if(detekt_razg()!=2) {return 0;}
 	 }
}
	else
		{delay_ms(500);voice_menu(n_abon);}  //���� �������� ������ ���� �������
							 // ���� ��� ��������� �����
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
void set_out_siren_int(unsigned char sost_ch)  // ��������� ����� ������ �� ������������������� ������
{
if (mask_out_siren&0x0001)		//���� ����� 1-����� ���������
	{if(sost_ch) out1=1;
		else     out1=0;}
if (mask_out_siren&0x0002)		//���� ����� 2-����� ���������
	{if(sost_ch) out2=1;
		else     out2=0;}
}
//--------------------
void set_out_siren(unsigned char sost_ch)  // ��������� ����� ������ �� ������������������� ������
{
if (EE_READ(konf_VIHOD)==2)		//���� ����� 1-����� ���������
	{if(sost_ch) out1=1;
		else     out1=0;}
if (EE_READ(konf_VIHOD+1)==2)		//���� ����� 2-����� ���������
	{if(sost_ch) out2=1;
		else     out2=0;}
}
//--------------------
void set_out_ohran(unsigned char sost_ch)  // ��������� ����� ��������� ������ �� ������ �������������������
{
if (EE_READ(konf_VIHOD)==3)		//���� ����� 1-����� ���������
	{if(sost_ch) out1=1;
		else     out1=0;}
if (EE_READ(konf_VIHOD)==4)		//���� ����� 1-����� ���������
	{if(sost_ch) out1=0;
		else     out1=1;}
if (EE_READ(konf_VIHOD+1)==3)		//���� ����� 2-����� ���������
	{if(sost_ch) out2=1;
		else     out2=0;}
if (EE_READ(konf_VIHOD+1)==4)		//���� ����� 2-����� ���������
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
		if(detekt_razg()!=2) {return 0;}			// ���� �������� ���������  �� �����
	}
		item=(set_dtmf()-'0')*10;
	while(1)
	{
		delay_s_dso(4);if(dso_vg) break;
		if(detekt_razg()!=2) {return 0;}			// ���� �������� ���������  �� �����
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
		if(detekt_razg()!=2) return;			// ���� �������� ���������  �� �����
	}
	item_menu=set_dtmf();
	if (item_menu==zirochka) {item_menu=set_duble();}		//���� ������ * �� ������������ ���������� �����
	 else {item_menu-='0';}
	if(item_menu==0) return;	// ����� ���� �������
	if(item_menu>2) {delay_s(1);voice_speech(oshibka,PWM1);continue;}
	if(item_menu==duble_click)
		{
		if(buf_dat[item_menu].konfig&0x80) {buf_dat[item_menu].konfig&=0x7F;EE_WRITE((item_menu+konf_ZONE),buf_dat[item_menu].konfig);init_datchik();}	//�������� �����
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
		if(detekt_razg()!=2) {return;}			// ���� �������� ���������  �� �����
	}
	item_menu=set_dtmf();
	if(item_menu=='0') {return;}	// ����� ���� �������
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
		dist_knop=1;				// ������ �������������� �������
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
		if(detekt_razg()!=2) return;			// ���� �������� ���������  �� �����
	}
	item_menu=set_dtmf();
	if(item_menu=='0') return;	// ����� ���� �������
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
		if(detekt_razg()!=2) return;	// ���� �������� ���������  �� �����
  }
}

//-------------------------------------------------
void voice_menu(char n_abon)			// ��������� ����
{
unsigned char item_menu,duble_click,work,end_zone,i;
	delay_s_dso(1);
duble_click=0;
step_voice=0;
 while(1)
 {
	while(1)
	{
	if((!step_voice)&&EE_READ(Voice_otchet))								// ���� ����� �� ���������� ����������
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
		if(detekt_razg()!=2) return;				// ���� �������� ���������  �� �����
	}

	item_menu=set_dtmf();
	switch (item_menu-'0')
	{
	case 7:								//���������� �� ������
					init_datchik();						// �������� �������
					set_out_ohran(1);		//���� ����� 1-����� ��������� �� ����� � ������
					delay_s(1);
					if(check_zone_alarm(0)) 
							{led_inform=6;delay_s(2);	//++ ���� ���� �� ������� �� ���������� � �����
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
						 	set_out_ohran(0);				//���� ����� 3-����� ��������� �� ���������� 
							if (EE_READ(Conf_post_sn)==4) 			//���� ������� �������� ����� ���������� ������ �� ����������������� ��� ���������  5 ������			
								{
									for (i=0;i<50;i++)				//����� 5 ������
									{
										delay_ms(100);
										// �������� ��������� ������ ���������� ������ �����- ��� �������
										if(buf_dat[0].sost!=(buf_dat[0].triger&0x0F))						//���� �������� ��������� �� 
											{
									 			EE_WRITE(system_status,1);led_inform=3;
												sw_opov_post_snyat=2;  //��������� ���� ��� ����������
												sound_opoveshenie(1);
												break;
											}
									}
								}
								else
									 {	EE_WRITE(system_status,1);led_inform=3;
										sw_opov_post_snyat=2;  //��������� ���� ��� ����������
										sound_opoveshenie(1);
									  }
						 }
			voice_sost_system();
			step_voice=1;
			sost_trevog=0;				// �������� ��������� �������
		break;
	case 8:	
		EE_WRITE(system_status,0);led_inform=0;siren_on(0);	//�������� ����������- ������
		set_out_ohran(1);		//���� ����� 3-����� ��������� �� ���������� 

		sost_trevog=0;				// �������� ��������� �������
		sw_opov_post_snyat=1;		//��������� ���� ��� ����������
		sound_opoveshenie(0);
		CNT_time_auto_set=60*(unsigned int)EE_READ(time_auto_set);
		voice_sost_system();
		step_voice=1;
		break;
	case 1:	
	case 2:	
		if(item_menu==duble_click)
			{
			if(check_vihod(item_menu-'0')) {work=0;}	//�������� ������
				else {work=1;}
			set_vihod((item_menu-'0'),work);
			}	
		voice_speech(vihod,PWM1);
		voice_out(item_menu-'0');
		if (!check_vihod(item_menu-'0')) voice_speech(vikl,PWM1);
			else
				{ voice_speech(vkl,PWM1);
					if (!EE_READ(konf_VIHOD+item_menu-'0'-1))			// ���� ��������������� ��� 2 ��� �� ����� 2 ��� ���������
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
					if(detekt_razg()!=2) {change_in(voice_in);return;}			// ���� �������� ���������  �� �����
				}
					item_menu=set_dtmf();
				if((item_menu=='0')||(item_menu=='3'))  	// ����� ���� �������
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
		if(detekt_razg()!=2) return;	// ���� �������� ���������  �� �����
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
	work=EE_READ(regim_napr+i);			// ��������� ����� ����������! �����
	buf_dat[i].triger=work;
	buf_dat[i].tek_sost=work;			// ������� ��������� �������
	buf_dat[i].sost=work;				// ��������� ������� � ���������
	const_dat_wait=9*(unsigned int)EE_READ(Save_const_dat_wait);
	buf_dat[i].wait=const_dat_wait;		// ��������� �������� �� ������
	buf_dat[i].hold=0;					//������ ��������������������������
	buf_dat[i].konfig=EE_READ(konf_ZONE+i);	// ��������� ������������ ��������
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
if((post_snjat==1)||(post_snjat==3))					// ����� 1 � 3 
	{
	if(buf_dat[0].triger&0xF0) 						//���� ���� �������� ������ ���������� ������
		{
			if(buf_dat[0].sost==(buf_dat[0].triger&0x0F))		//���� �� ������������� ������ �����
				{
				 buf_dat[0].triger&=0x0F;						// �������� ������ � 0
				 glob_knopka_system=EE_READ(knopka_system);
				 if(glob_knopka_system) {infodat=0;}		//���������������� ��������� �������
					else {infodat=1;}
				}
		}	
	}
	else   // ����� 0 � 2
	{
	if(buf_dat[0].sost==(buf_dat[0].triger&0x0F)) {infodat=0;}	// �������� ��������� ������ ���������� ������ �����- ��� �������
		else{infodat=1;}
	}	

	glob_knopka_system=EE_READ(knopka_system);
auto_set:
if(glob_knopka_system!=infodat)				// ����� ���������� ��������� ������ ����������- ������
	{   
		EE_WRITE(knopka_system,infodat);
		glob_knopka_system=infodat;
		if(EE_READ(system_status)!=infodat)  //� ��� �� ��������� � ���������� ������� �� �������� ��������� �������
			{

				if(infodat)
						{
						 init_datchik();						// �������� �������
						delay_s(1);
						if(check_zone_alarm(0)) {led_inform=6;delay_s(2);return;}	//++ ���� ���� �� ������� �� ���������� � �����
						cnt_secund=0;
						end_time=EE_READ(time_VIHOD);			//���� ����� ��� ������ �������� �� �����
						led_inform=1;							// ����� ��������
							do{
								if(check_zone_alarm(2)) 		// ��������� ��� �� ����������� ��� � �������
											{led_inform=6;delay_s(2);return;}//���� ���� ���� ���� ����������� �����������- �� 
							//	init_OK_zone();	// ����� ��� ������ ����������������	
								delay_ms(100);
								}while (cnt_secund<end_time);
						}
				time_cnt_bezdejstvie=cnt_bezdejstvie=EE_READ(time_STAND)*60;
				cnt_siren=EE_READ(time_SIREN)*60;
				siren_on(0);						//��������� ������
				sost_trevog=0;				// �������� ��������� �������
				clear_zone_alarm(0);		// �������� ��������� �����
				link_out_SET(1);			// �������� ������ ��  �����
				EE_WRITE(system_status,infodat);
				init_datchik();
				sw_opov_post_snyat=infodat+1;		//��������� ���� ��� ����������
				sound_opoveshenie(infodat);

			}
	}
if (!EE_READ(system_status)) 
				{				// ���� �� ��� ������� �� �������������� ��������������
				led_inform=0;
				regim=1;
				set_out_ohran(1);		//���� ����� 3-����� ��������� �� ���������� 
				if(EE_READ(time_auto_set))		// ���� �������� �������������� �� ������
						{
						if(check_zone_alarm(0))	{CNT_time_auto_set=60*(unsigned int)EE_READ(time_auto_set);init_datchik();}	// ���� ���� ���� � ������� �� �������� ������� 
							else {if(!CNT_time_auto_set){infodat=1;glob_knopka_system-0;CNT_time_auto_set=60*(unsigned int)EE_READ(time_auto_set); goto  auto_set;}}
						}
				}	
		else 
			{
			if(sost_trevog) {led_inform=2;}	// ���� ���� ������� �� ������
					else {led_inform=3;}		// ���� ���� ������� �� ���
			regim=0;
			set_out_ohran(0);		//���� ����� 3-����� ��������� �� ���������� 
			}
								//
clear_zone_alarm(0);						// �������� ��������� �����

if(!check_zone_alarm(regim)) 		// ��������� ��� �� ��� � �������
				{
				if(check_zone_alarm(4))			//  ���� ���� ���������������� �� ��������� ����� ��� ���������� �� ����������������
					{	
					link_out_SET(0);			// �������� ������ �� ���������������� �����
					if(EE_READ(Opov_Vostan))		
						{ 
						led_inform=6;
						alarm_dozvon(4);	// ���������� � ����������������
						}
					}
				init_OK_zone();	// ����� ��� ������ ����������������
				return;		
				}		

if((buf_dat[1].alarm)&&(!regim))		// ���� �� ��� �������  � �������� 1� ������ �� ���������� ��������
			{
			cnt_secund=0;
			end_time=EE_READ(time_VHOD);			//���� 1 � ���� �� �������� �� ���� 
				led_inform=1;							// ����� ��������
				clear_zone_alarm(2);			//�������� �������- ���������
					do{
						if(check_zone_alarm(2)) 		// ��������� ��� �� ����������� ��� � �������
									{break;}//���� ���� ���� ���� ����������� �����������- ����� �������
						init_OK_zone();	// ����� ��� ������ ����������������
						
					if((post_snjat==1)||(post_snjat==3))					// ����� 1 � 3 
						{
						if(buf_dat[0].triger&0xF0) 						//���� ���� �������� ������ ���������� ������
							{
							if(buf_dat[0].sost==(buf_dat[0].triger&0x0F))		//���� �� ������������� ������ �����
								{
								 buf_dat[0].triger&=0x0F;						// �������� ������ � 0
				 					glob_knopka_system=EE_READ(knopka_system);
				 					if(glob_knopka_system) {infodat=0;}		//���������������� ��������� �������
										else {infodat=1;}
								}
							}	
						}
					else   // ����� 0 � 2
						{
						if(buf_dat[0].sost==(buf_dat[0].triger&0x0F)) {infodat=0;}	// �������� ��������� ������ ���������� ������ �����- ��� �������
														else{infodat=1;}
						}

						if(EE_READ(knopka_system)!=infodat)				// ����� ���������� ��������� ������ ����������- ������
							{   
								if(infodat) 	 //� �������� ��������� �� ������ �� ��������� ����� ���������
										{EE_WRITE(knopka_system,infodat);}
									else {return;}	//���� ����� � ������ �� ����
							}
						delay_ms(100);
						}while (cnt_secund<end_time);
				check_zone_alarm(regim);				// ������� ������� �� ������� � ��������� ���� ����
			}

link_out_SET(1);
led_inform=6;
siren_on(1);				// �������� ������
alarm_dozvon(regim);						//���� ���� ���� ���� ����������� �������������- ����� �������

}
//-----------------
void init_OK_zone(void)
{char i,end_zone;
//fl_sv_trig=0;
	end_zone=last_zon;
for (i=1;i<=end_zone;i++)
	{
			// ��� ����
	if(buf_dat[i].konfig==3) // ���� ���� ����������� �� ��������������� ��������� �������
		{if((buf_dat[i].hold)&&((buf_dat[i].triger&0x0F)!=buf_dat[i].sost)){buf_dat[i].hold=0;buf_dat[i].triger&=0x0F;cnt_bezdejstvie=time_cnt_bezdejstvie; } } 
		else{if((buf_dat[i].hold)&&((buf_dat[i].triger&0x0F)==buf_dat[i].sost)) {buf_dat[i].hold=0;buf_dat[i].triger&=0x0F;/*fl_sv_trig=1;*/}}	// ����� ��������������� - ��������
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
	case 0:						//	 0 - ��� 
		buf_dat[i].alarm=0;
	break;
	case 2:						//	 2 - �������� ��� �������- ���������
		if(buf_dat[i].konfig==1){buf_dat[i].alarm=0;}
	break;
	case 3:						//	 3 - �������� ��� �����������
		if(buf_dat[i].konfig==3){buf_dat[i].alarm=0;}
	break;
	default:
	break;
	}
}
}
//-------------------
char check_zone_alarm(char regim)		// ��������� � ��������� ���� � ��������� ������� ������� 1 - ������� 0 - �����
										// ��������-� 0 - ��� �� ��������������� 
										// 			  1	- �������� �������������� ���
{										//			  2 - ����������+ ��������������
char i,ret_chr,end_zone;							//			  3 - �������� ��� �����������
ret_chr=0;								//			  4- �������� ���������������� ���
										//			  5- �������� ��� c ��������
end_zone=last_zon;
for (i=1;i<=end_zone;i++)
{	
if (buf_dat[i].konfig&0x80)	continue;	// �������� ��� ���������� �������, ���� ������������, �� ���������� ����� �������	
if (buf_dat[i].alarm) {ret_chr=1; continue;}			//���� ����� ���� - �� ����������	
 switch(regim)
	{
	case 0:						//	 0 - ��� �� ���������������		
		if(buf_dat[i].konfig==3){if((buf_dat[i].triger&0xF0)&&(!buf_dat[i].hold)&&cnt_bezdejstvie)	// 		3 - ���� �����������
										{cnt_bezdejstvie=time_cnt_bezdejstvie;buf_dat[i].triger&=0x0F;}		// ���� ��������- ������
									else{buf_dat[i].alarm=1;ret_chr=1;}		// �� ����- ������� 
								}
				else if((buf_dat[i].triger&0xF0)&&(!buf_dat[i].hold)){buf_dat[i].alarm=1;ret_chr=1;}	// ����� �������- ��������
	break;
	case 1:					//	 1	- �������� �������������� ���			
	if((buf_dat[i].triger&0xF0)&&(!buf_dat[i].hold)&&(buf_dat[i].konfig==2)){buf_dat[i].alarm=1;ret_chr=1;}	// ����� �������- ��������
	break;
	case 2:						// 		2 - ����������+ ��������������
	if((buf_dat[i].triger&0xF0)&&(!buf_dat[i].hold)&&((buf_dat[i].konfig==2)||(buf_dat[i].konfig==0))){buf_dat[i].alarm=1;ret_chr=1;}	// ����� �������- ��������
	break;
	case 3:						// 		3 - ���� �����������
	if(buf_dat[i].konfig==3){if((buf_dat[i].triger&0xF0)&&(!buf_dat[i].hold)&&cnt_bezdejstvie)	// 		3 - ���� �����������
										{cnt_bezdejstvie=time_cnt_bezdejstvie;buf_dat[i].triger&=0x0F;}		// ���� ��������- ������
									else{buf_dat[i].alarm=1;ret_chr=1;}		// �� ����- ������� 
								}
	break;
	case 4:						// 		4 - ���������������� ����
	if((buf_dat[i].hold)&&((buf_dat[i].triger&0x0F)==buf_dat[i].sost)) {buf_dat[i].alarm=1;ret_chr=1;}	// ����� ��������������� - ��������
	break;
	default:
	break;
	}
}
return ret_chr;
}
//---------------
void end_alarm_dozvon(char regim)			// ������ ��������
{
char i,fl_save_rsh,end_zone;
fl_save_rsh=0;
end_zone=last_zon;
for (i=1;i<=end_zone;i++)
	{		
if (!buf_dat[i].alarm) { continue;}			//���� ����� ��� - �� ����������
	buf_dat[i].alarm=0;						//�������� �����	
 switch(regim)
	{
	case 0:						//	 0 - ��� �� ���������������		
	buf_dat[i].hold=1;			// ����� �������- �������������
	break;
	case 1:						//	  1	- �������� �������������� ���			
	if(buf_dat[i].konfig==2){buf_dat[i].hold=1;}			// ����� �������- �������������}
	break;
	case 2:						// 		2 - ����������+ ��������������
	if((buf_dat[i].konfig==2)||(!buf_dat[i].konfig)){buf_dat[i].hold=1;}			// ����� �������- �������������}
	break;
	case 3:						// 		3 - ���� �����������
	if(buf_dat[i].konfig==3){buf_dat[i].hold=1;}			// ����� �������- �������������}
	break;
	case 4:						// 		4 - ���������������� ����
	buf_dat[i].hold=0;			// ����� ��������������� - ��������(����� ��� ������)
	buf_dat[i].triger&=0x0F;
	fl_save_rsh=1;
	break;
	default:
	break;
	}
	}	
}

//------------------------------
void alarm_dozvon(char regim)			//��������� ������� 0- ����� ������� 1- ������� �������������� ���
{
										//0	�� ������� ������ SMS �� ����������
										//1	�� ������� ������ SMS ���������� ����� ������� � ������, ���� ������� �� ������ ����������� �� �� ������ �� ������� ������
										//2	�� ������� ������ SMS ���������� ����� ������� � ����� ������
										//3	�� ������� ������ SMS ���������� � ������ �������, ����� ������
										//4	�� ������� ������ ���������� ������ SMS

char i,n_abon,count_dozvon,count_SMS,sms_regim;
sost_trevog=1;				// ����������  ��������� �������
for (i=0;i<16;i++){map_dozvon[i]=0;}	// �������� ����� �������
count_dozvon=EE_READ(lim_DOZVON);		// ����������� ������ �������
count_SMS=EE_READ(lim_SMS);				// ��������� ����������� ��� �� �����
fl_uspeh=0;								// ��� ��������� ������
	while(count_dozvon--)
	{
	for(n_abon=0;n_abon<10;n_abon++)
	 {
		if (map_dozvon[n_abon]) continue;		// ���� ����� �������� �� ������� - ���������
		sms_regim=read_regim(n_abon);
		if((Bufer_SPI[2]==0xFF)||(Bufer_SPI[2]==0x00)) {map_dozvon[n_abon]=1;continue;}		//���� ������ ���! ���������� 
		if(sms_regim=='3') snd_alarm_sms(n_abon,regim);	// ���� ����� ������� SMS �� ��������� ���
		if(sms_regim!='4') if(snd_alarm_voice(n_abon,regim)){end_alarm_dozvon(regim);return;} // ���������� ������� ���� ������ ��������! �����
   	 }
	}
 while (count_SMS--)
	{
		for(n_abon=0;n_abon<10;n_abon++)
	 {
		sms_regim=read_regim(n_abon);
		if((Bufer_SPI[2]==0xFF)||(Bufer_SPI[2]==0x00)) {continue;}		//���� ������ ���! ���������� 
		switch(sms_regim)
		{
		case '0':		//	�� ������� ������ SMS �� ����������
				break;
		case '1':		// ���� ��� ������
				if(!fl_uspeh) snd_alarm_sms(n_abon,regim);
				break;
		case '2':		//		�� ������� ������ SMS ���������� ����� ������� � ����� ������
				snd_alarm_sms(n_abon,regim);
				break;
		case '3':
				break;	// ���� ����� ������� SMS �� ���������
		case '4':		//�� ������� ������ ���������� ������ SMS
			 snd_alarm_sms(n_abon,regim);
			break;
		default:
			break;
		}
   	 }

	}
	end_alarm_dozvon(regim);			// ������ ��������
}

//----------------
char snd_alarm_voice(char n_abon,char regim)				//��������� ������ ��������� 1- ������� �������������, 0 ��� ���������
{
 char lim_time,i,work,end_zone;
	putst("ATH\r\n");
	delay_s(1);
  	call_abon(n_abon);
	delay_s(3);
	if(!wait_razg(30)) {return 0;}
	//�����������
 	delay_ms(300);
	 putst("AT+CHFA=0\r");
 	delay_ms(300);
 	change_in(voice_in);
	fl_uspeh=1;	
	lim_time=120;			//������ �������� 2 ������
	cnt_secund=0;
	while(cnt_secund<lim_time)
	{
	check_zone_alarm(regim);				//��������� ����������� ����
											// ����������
end_zone=last_zon;
	for (i=1;i<=end_zone;i++)
		{		
		if(!buf_dat[i].alarm)continue;		// ���� ���� �� � ������� ������
		if(regim!=4){voice_alarm(i,0,0);}		// �������� ������� �� ���� � ������������ � ��������
			else {voice_alarm(i,0,1);}		// �������� �������������� � ���� � ������������ � ��������	
		if(dso_vg)	// ���� ������ ������ 0 �� � ����
			{
			if((cod_dtmf()==0x0A)||(cod_dtmf()==0b00001011)||(cod_dtmf()==0b00001100))
			 {
			work=menu_trevoga(n_abon);
			if(!work) {cnt_secund=0;continue;}			//������ �� �������� ��� ���������� ������� �� 0
			if(work==1) 		// ��������� �� ������
				{
				map_dozvon[n_abon]=1;
				putst("ATH\r");
				return 0;
				}
			if(work==2) 		// �������� ��������
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
	putst("ATH\r");						// ����� �����
	return 0;
}
//------------------------------------------- 
void voice_alarm(char num_zone,char map_alarm,char type_alarm)   // map_alarm ���������� ����� ������� ������������ ( ������ ����� ������� (0) ��� Contact ID(1))
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
			voice_speech(dvigenija,PWM1);		//��������
		break;
	case 2:
			voice_speech(proniknoveniya,PWM1);		//�������������
		break;
	case 3:
			voice_speech(napadenie,PWM1);		//���������
		break;
	case 4:
			voice_speech(vodi,PWM1);			//����
		break;
	case 5:
			voice_speech(pogara,PWM1);			//������
		break;
	case 6:
			voice_speech(dima,PWM1);			//����
		break;
	case 7:
			voice_speech(gaza,PWM1);			//����
		break;
	case 8:
			voice_speech(sireni,PWM1);			// ������
		break;
	case 9:
			voice_speech(klaviaturi,PWM1);		//����������
		break;
	case 10:
			voice_speech(dveri,PWM1);			//�����
		break;
	case 11:
			voice_speech(okna,PWM1);			//����
		break;
	case 12:
			voice_speech(narug_perimetr,PWM1);	//�������� ��������
		break;
	case 13:
			voice_speech(fasad,PWM1);			//�����
		break;
	case 14:
			voice_speech(til,PWM1);			 //���
		break;
	case 15:
			voice_speech(etaj,PWM1);			//����1
			voice_speech(cif_1,PWM1);
		break;
	case 16:
			voice_speech(etaj,PWM1);			 //����2
			voice_speech(cif_2,PWM1);
		break;
	case 17:
			voice_speech(zal,PWM1);			//���
		break;
	case 18:
			voice_speech(spalnya,PWM1);			//�������
		break;
	case 19:
			voice_speech(prihogaya,PWM1);	//��������
		break;
	case 20:
			voice_speech(kuhnya,PWM1);			//�����
		break;
	case 21:
			voice_speech(sklad,PWM1);			 //���
		break;
	case 40:
			voice_speech(Srab_ohran_sig,PWM1);			//��������� �������� ������������
		break;
	case 41:
			voice_speech(Srab_pojag_sig,PWM1);			//��������� �������� ������������
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
			voice_play((konf-50),PWM1);		// ���������� ���������
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
char menu_trevoga(char n_abon)			//���� � ��������� ������� ��������� 0 ���������� ����� 
										//1-������� �������� �� ������ 2- �������� ���������������
{
char item_menu,end_zone,i,duble_click,work;
char frst_vhod=0;			//����� ������� �����
	delay_s_dso(1);
	if (read_kateg(n_abon)>'2')						// ���� ������������ ������������ �� ������ ������ ���������
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
	if((!step_voice)&&EE_READ(Voice_otchet))								// ���� ����� �� ���������� ����������
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
		if(detekt_razg()!=2) {goto end_menu;}				// ���� �������� ���������  �� �����
	}
	item_menu=set_dtmf();
	switch (item_menu-'0')
	 {
	case 0:	
		if(!frst_vhod) break;				// ���� ������ ����� �� ����������
		goto end_menu;				// ���- �������
	//	break;
	case 7:								//���������� �� ������
					init_datchik();						// �������� �������
					set_out_ohran(1);		//���� ����� 1-����� ��������� �� ����� � ������
					delay_s(1);
					if(check_zone_alarm(0)) 
							{led_inform=6;delay_s(2);	//++ ���� ���� �� ������� �� ���������� � �����
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
						 	set_out_ohran(0);				//���� ����� 3-����� ��������� �� ���������� 
							if (EE_READ(Conf_post_sn)==4) 			//���� ������� �������� ����� ���������� ������ �� ����������������� ��� ���������  5 ������			
								{
									for (i=0;i<50;i++)				//����� 5 ������
									{
										delay_ms(100);
										// �������� ��������� ������ ���������� ������ �����- ��� �������
										if(buf_dat[0].sost!=(buf_dat[0].triger&0x0F))						//���� �������� ��������� �� 
											{
									 			EE_WRITE(system_status,1);led_inform=3;
												sw_opov_post_snyat=2;  //��������� ���� ��� ����������
												sound_opoveshenie(1);
												break;
											}
									}
								}
								else
									 {	EE_WRITE(system_status,1);led_inform=3;
										sw_opov_post_snyat=2;  //��������� ���� ��� ����������
										sound_opoveshenie(1);
									  }
						 }
			voice_sost_system();
			step_voice=1;
			sost_trevog=0;				// �������� ��������� �������
		break;
	case 8:	
		EE_WRITE(system_status,0);led_inform=0;siren_on(0);	//�������� ����������- ������
		set_out_ohran(1);		//���� ����� 3-����� ��������� �� ���������� 

		sost_trevog=0;				// �������� ��������� �������
		sw_opov_post_snyat=1;		//��������� ���� ��� ����������
		sound_opoveshenie(0);
		CNT_time_auto_set=60*(unsigned int)EE_READ(time_auto_set);
		voice_sost_system();
		step_voice=1;
		break;
	case 1:	
	case 2:	
		if(item_menu==duble_click)
			{
			if(check_vihod(item_menu-'0')) {work=0;}	//�������� ������
				else {work=1;}
			set_vihod((item_menu-'0'),work);
			}	
		voice_speech(vihod,PWM1);
		voice_out(item_menu-'0');
		if (!check_vihod(item_menu-'0')) voice_speech(vikl,PWM1);
			else
				{ voice_speech(vkl,PWM1);
					if (!EE_READ(konf_VIHOD+item_menu-'0'-1))			// ���� ��������������� ��� 2 ��� �� ����� 2 ��� ���������
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
					if(detekt_razg()!=2) {change_in(voice_in);return 0;}			// ���� �������� ���������  �� �����
				}
					item_menu=set_dtmf();
				if((item_menu=='0')||(item_menu=='3'))  	// ����� ���� �������
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
		if(detekt_razg()!=2) {goto end_menu;}	// ���� �������� ���������  �� �����
 }
end_menu:
	if (EE_READ(system_status)){return 0;}
		else return 2;		//���� ����� � ������ �� ���� �������
}

//----------------------------------------------
void sound_opoveshenie(unsigned char infodat)
{
char post_snjat;
post_snjat=EE_READ(Conf_post_sn);
if((post_snjat==2)||(post_snjat==3))				// ����� 2 � 3 � ������������ ���������� �� 
	{
		set_out_siren(1);											//���������
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
void opov_postanovka_snjatie (unsigned char type_oth)			// ���������� � ����������- ������
{
unsigned char type_opov,lim_time;

type_opov=EE_READ(otchet_postan);
if(!type_opov)	return;			// ���� �� ����������� �� ����� ���������

opoveshenie_slugebnoe(post_sn_kod,type_oth,type_opov);  // ����������

}
//----------------------------------------------
void send_TEST(void)						//�������� ��������� ���������
{
char type_opov,lim_time,i;
type_opov=EE_READ(regim_TEST);
if(!type_opov)	return;			// ���� �� ���������� �� ����� ���������
opoveshenie_slugebnoe(test_kod,type_opov,type_opov);  // ����������

}
//--------------------
void send_opov_napr(void)
{
char type_opov,lim_time,tmp1;
type_opov=EE_READ(regim_BORT_napr);
if(!type_opov)	return;			// ���� �� ����������� �� ����� ���������

opoveshenie_slugebnoe( napr_kod,fl_bort_napr,type_opov);  // ����������


}
//--------------------
void send_opov_temperatura(unsigned char conf_dallas,unsigned char type_opov,unsigned char CH_DL)		// ��� ��������� 0-�� ����������� � �������, 1- SMS, 2 ��������� ��������� 
{
unsigned char temp,lim_time,item_menu;
static signed char temp_low,temp_high,temp_gist;
//res_eep=0;							// ���������� ������ DS
delay_ms(100);
temperatura_dallas[CH_DL-1]=read_temp_dallas(CH_DL); 
if(temperatura_dallas[CH_DL-1]==0x7F) {delay_s(1);temperatura_dallas[CH_DL-1]=read_temp_dallas(CH_DL);}  //�������� ��� ������ 2 ���� 
//res_eep=1;

temp_low=EE_READ(TEMP_LOW+CH_DL-1);
temp_high=EE_READ(TEMP_HIGH+CH_DL-1);
temp_gist=EE_READ(TEMP_GIST+CH_DL-1);
	//������ ����������� ���������

		switch(temperatura_save[CH_DL-1])
			{
				case 0:							//���� ���� ������ �������
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


if(temp==temperatura_save[CH_DL-1]) return;				//���� ����������� �� ���������� - �������
 												//���� ���������� ���������- ����������
temperatura_save[CH_DL-1]=temp;

opoveshenie_slugebnoe((temperatura_kod1+CH_DL-1),temperatura_save[CH_DL-1],type_opov);  // ����������


}

//----------------

void opoveshenie_slugebnoe(unsigned char cod_opovesheniya,unsigned char type_oth, unsigned char type_opov)    // ���������� ���������� ����������� ��� ��������� � ��� ��������� � ������ ����������(1- SMS,2-�����)
{

char i,n_abon,count_dozvon,kateg,lim_time,item_menu,fl_stop;
for (i=0;i<10;i++){map_dozvon[i]=0;}	// �������� ����� �������
if(type_opov==2){count_dozvon=EE_READ(lim_DOZVON);	}	// ����������� ������ �������
	else {count_dozvon=1;}
led_inform=6;
	while(count_dozvon--)
	{
	for(n_abon=0;n_abon<10;n_abon++)
	 {
		if (map_dozvon[n_abon]) continue;		// ���� ����� �������� �� ������� - ���������
		kateg=read_kateg(n_abon);
		if((Bufer_SPI[2]==0xFF)||(Bufer_SPI[2]==0x00)) {map_dozvon[n_abon]=1;continue;}		//���� ������ ���! ���������� 
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
							//�����������
 							delay_ms(300);
	 						putst("AT+CHFA=0\r");
 							delay_ms(300);
 							change_in(voice_in);
							fl_uspeh=1;	
							lim_time=120;			//������ �������� 2 ������
							cnt_secund=0;
							while(cnt_secund<lim_time)
								{
											// ����������
									voice_slugeb(cod_opovesheniya,type_oth);
								if(dso_vg)	// ���� ������ ������ 0 �� � ����
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
