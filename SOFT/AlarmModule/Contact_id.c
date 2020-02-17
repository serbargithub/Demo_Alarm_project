#include <pic18.h> 
#include "name_port.h"
#include "my_delay.h"
#include "usart.h" 
#include "fun_abon.h"
#include "name_konst.h"
#include "name_adres.h"
#include "eeprom.h"
#include "function.h"
#include "usart_buf.h" 
#include "my_spi_SST26.h"
#include "SMS_rutine.h"
#include "signal.h"
#include "speech.h"
#include "terminal.h"
#include "lexem_rutine.h"
#include "voice_map.h"


unsigned char buffer_work[128];

extern unsigned char fl_beep,fl_uspeh,led_inform,sost_trevog,trub_on;
extern unsigned char number_str[],map_dozvon[];					// ����� ��������
extern  unsigned int point_buf,cnt_secund,cnt_siren,cnt_bezdejstvie,time_cnt_bezdejstvie;


struct RX_trevog
{
   unsigned int  code;
   unsigned char type;
   unsigned char group;
   unsigned char zone;
};
struct RX_trevog BUF_recive[16];
//const unsigned char St_AdrID=0xB0;			//������ ���� ������ 


void print_command(char n);
unsigned char ld_cif_id(void);
unsigned char build_SMS(char i);
unsigned char chek_num_central(void);
char snd_contact_id_voice(char n_abon,char n_pos);
void alarm_Contact_ID(char n_pos);
char voice_contact_id(char i);
void  snd_contact_id_sms(char n_abon,char n_pos);
void bild_SMS_zone(char num_zone,char regim);
void check_post_snyat(unsigned char);	
//===================================================================

//---------------------
unsigned char ck_start_id(void)					//������� ��������� ��������
{
	unsigned char i;
	delay_s(1);
  for (i=0;i<2;i++)
  {

	beep_on(5,6);						// 1400 Hz
	delay_ms(100);
	fl_beep=0;
	delay_ms(100);
	beep_on(3,4);							// 2300 Hz
	delay_ms(100);
	fl_beep=0;
	delay_s_dso(1);
	if (dso_vg) return 0;	
  }
	return 1;
}

//------------------------------

void end_signal(void)					// ������ ��������� ������
{
	beep_on(6,5);						// 1400 Hz
	delay_s(1);
	fl_beep=0;	
}
//-------------------------------
unsigned char ck_sprava(void)						// �������� ��������� �������
{
	delay_s_dso(1);
	if (dso_vg) {return 0;}
	return 1;
}
//-------------------------------

unsigned char load_com_id(void)
{
	unsigned char i,work,dop;
	for(i=0;i<16;i++)
	{
	work=ld_cif_id();
	if (work==0xFF)  {return 1;}		// �� ������ �����- ������
	else
		{
		number_str[i]=work;	
		}
	}
	work=0;
	for(i=0;i<15;i++)				//������� ����������� �����
	{
	work+=number_str[i];
	}
	dop=(work/15+1)*15-work;			//���������� ����������� �����
	if (dop!=number_str[15]) {return 1;}	//���������- ������
	return 0;
}

//---------------------------------

unsigned char ld_cif_id(void)			// ����� �����+ ������������� � ������� ��
{
unsigned char numb,i;
//long g;
numb=0;
//g=5000;			//
i=100;
while(1)						// 100 ms
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
if (!(i--))	{return 0xFF;}
delay_ms(1);
} 				
if	(!numb) {return 0xFF;}			//����� �������� � �������
	//��������� ����
return numb;
}

//---------------------------------

unsigned char recive_id(void)					// ������� ������ ���������  � ������ �� � �����
{												// ��������� ����������� �������� ���������
unsigned char i,j;
//cnt_reciv=0;
	if (ck_start_id())	{return 0;}				// ������� ������, ���� ��� ������ - ������� 0 
for (i=0;i<16;)									//������������ ����������� ����������� ���������
	{
		if (!ck_sprava()) 							// ������ �����?
			{
				if (!load_com_id())					//�������� ������� ���������?
				{
					for (j=6;j<15;j++) {if (number_str[j]==10) number_str[j]=0;} // ��������� ���������
				    BUF_recive[i].code =number_str[7]*100;	// ��� �������
				    BUF_recive[i].code+=number_str[8]*10;	// ��� �������
				    BUF_recive[i].code+=number_str[9];		// ��� �������
					BUF_recive[i].type=number_str[6]; 		// ��� ������� (����-����)
				    BUF_recive[i].group= number_str[10]*10;	// ����� ������
				    BUF_recive[i].group+=number_str[11];	// ����� ������
				    BUF_recive[i].zone= number_str[12]*100;	// ����� ����
				    BUF_recive[i].zone+=number_str[13]*10;	// ����� ����
				    BUF_recive[i].zone+=number_str[14];		// ����� ����

					end_signal();					// ������� ����� 
					i++;
				}
				else continue;
			}
		else break; 
	}
return i;
		
}
//-----------------------------------------------------
unsigned char chek_num_central(void)				//�������� �� ������ �� ��������
{
unsigned char temp,cnt_id; 
	if ((number_str[0]=='5')&&
		(number_str[1]=='4')&&
		(number_str[2]=='3')&&
		(number_str[3]=='2')&&
		(number_str[4]=='1'))
			{
				temp=recive_id();
				cnt_id=EE_READ(CONTACT_ID_print);
				if(cnt_id){putst("\r\nRecive-"); putch(temp+'0'); putch('\r');}
				if (!temp) {return 0;}		// �� ������� �� ������ ���������- ����� �����	
				if(cnt_id){print_command(temp);}
				if(EE_READ(post_CONACT_ID)){check_post_snyat(temp);}	// ��������� �� ���� �� �������� ����������- ������ ���������� - ������
				alarm_Contact_ID(temp);				//���������� �������� ���������
			}
	 else {return 1;}

return 0;
}
//----------------------------------

void print_command(char n)			//����������� ��������� n-����� �������
{
char i;
for (i=0;i<n;i++)
	{
		putst("\r\n");putch(i+'0'+1);
		putst(": code-"); putch_long(BUF_recive[i].code);
		putst(": type-"); putchdec(BUF_recive[i].type);
		putst(": zone-"); putchdec(BUF_recive[i].zone);
		putst("\r");

	}
}
//---------------------
char base_code (int code,char kategorija,char voice_sms, char trevoga_vostanovlenie)			// ���� ����� �� ��������� 0-voice 1- sms
{
//��������� 0,1,2
if((kategorija>='0')&&(kategorija<='2'))
 {
	switch (code)
		{
		case 100:	return 1;		//Knopka TREVOGA!
		case 115:	return 1;		//Knopka POJAR!
		case 120:	return 1;		//Knopka PANIKA!
		case 121:	return 1;		//VNIMANIE! Kod vveden pod ugrozoj
	if(trevoga_vostanovlenie)
		{
	case 110:	return 1;          //�������� �������
	case 111:	return 1;          //������ ����
	case 112:	return 1;          //����������
	case 113:	return 1;          //������ ����
	case 114:	return 1;          //�������� ��������
	case 117:	return 1;          //�����

	case 150:	return 1;

		case 130:	return 1;		//Trevoga v zone -
		case 131:	return 1;		//Trevoga v zone -
		case 132:	return 1;		//Trevoga v zone -
		case 133:	return 1;		//Trevoga v zone -
		case 134:	return 1;		//Trevoga v zone -
		case 135:	return 1;		//Trevoga v zone -
		case 136:	return 1;		//Trevoga v zone -
		case 137:	return 1;		//Trevoga v zone -
		case 144:	return 1;		//Narushenie tamperna v zone
		case 372:	return 1;		//�������� ��������� v zone
		}
		}
	if((kategorija=='2')&&(!voice_sms)) return 0;
	switch (code)
		{
		case 400:	return 1;		//Sistema snyata s ohrani.spec ustrojstvom
		case 401:	return 1;		//Sistema snyata s ohrani.
		case 402:	return 1;		//Sistema snyata s ohrani.
		case 405:	return 1;		//otmena avto postanovki na ohranu
		case 408:	return 1;		//������� ����������.
		case 409:	return 1;		//���������� ������ ���������
		}
	if((kategorija=='1')&&(!voice_sms)) return 0;
	switch (code)
		{
		case 301:	return 1;		//Propalo setevoe pitanie!
		case 302:	return 1;		//Akkumulator pitanija razragen!
		case 321:	return 1;		//Neispravna sirena.
		case 333:	return 1;		//Neispravna klaviatura.
		case 351:	return 1;		//Telef linija vostanovlena.
		case 373:	return 1;		//����� �������� ����
		case 601:	return 1;		//Test.
		case 602:	return 1;		//Test.
		case 603:	return 1;		//Test.
		}
	if(!voice_sms) return 0;	// ���� ��������� ������ ��  ������ sms
	switch (code)
		{
		case 140:	return 1;		//TREVOGA neskolkih zon!
		case 143:	return 1;		//Neispraven rashiritel zon.
		case 145:	return 1;		//Narushenie tamperna 
		case 300:	return 1;		//������������� ������ ��� ������� ������!
		case 384:	return 1;		//Razrjad baterejki vo vneshnem ustrojstve! 
		case 406:	return 1;		//Ohrana snyata posle trevogi.
		case 421:	return 1;		//Klaviatura zablokirovana.
		case 454:	return 1;		//������ ���������� ����������
		case 470:	return 1;		//Sistema snyata s ohrani chastichno
		case 623:	return 1;		//Vnimanie! Bufer sobitij zapolnen na 75%.
		}
 }

//��������� 3 � ������
if(kategorija>'2')
	{
	switch (code)
		{
		case 100:	return 1;		//Knopka TREVOGA!
		case 115:	return 1;		//Knopka POJAR!
		case 120:	return 1;		//Knopka PANIKA!
		case 121:	return 1;		//VNIMANIE! Kod vveden pod ugrozoj
	if(trevoga_vostanovlenie)
			{
		case 110:	return 1;	   //�������� �������
	case 111:	return 1;          //������ ����
	case 112:	return 1;          //����������
	case 113:	return 1;          //������ ����
	case 114:	return 1;          //�������� ��������
	case 117:	return 1;          //�����
		case 150:	return 1;
		case 130:	return 1;		//Trevoga v zone -
		case 131:	return 1;		//Trevoga v zone -
		case 132:	return 1;		//Trevoga v zone -
		case 133:	return 1;		//Trevoga v zone -
		case 134:	return 1;		//Trevoga v zone -
		case 135:	return 1;		//Trevoga v zone -
		case 136:	return 1;		//Trevoga v zone -
		case 137:	return 1;		//Trevoga v zone -
		case 144:	return 1;		//Narushenie tamperna v zone
			}
		}
	}
return 0;			// �� ������� ������
}
//------------------------------------
unsigned char build_SMS(char i)						//������ SMS. �������� ������� ������� � ���� ��������� ������ ���������+ ��� � ������
{
switch (BUF_recive[i].code)
 {

	case 100:
			if(BUF_recive[i].type==1) {wr_strbuf("Knopka TREVOGA!",0);}
				else {return 0;}
		break;
	case 115:
			if(BUF_recive[i].type==1) {wr_strbuf("Knopka POJAR!",0);}
				else {return 0;}
		break;
	case 120:
			if(BUF_recive[i].type==1) {wr_strbuf("Knopka PANIKA!",0);}
				else {return 0;}
		break;
	case 121:
			wr_strbuf("VNIMANIE! Kod vveden pod ugrozoy",0);
		break;
	case 140:
			wr_strbuf("TREVOGA neskolkih zon!",0);
		break;

//--�������� �������--
	case 110:
	case 111:
	case 112:
	case 114:
	case 117:
			if(BUF_recive[i].type==1) {wr_strbuf("Vozgoranie! ",0);bild_SMS_zone(BUF_recive[i].zone,5);wr_strbuf(" Gruppa: ",1);wr_ptbuf_dec(BUF_recive[i].group,1);}
				else {wr_strbuf(" ",0);bild_SMS_zone(BUF_recive[i].zone,4);wr_strbuf(" Gruppa: ",1);wr_ptbuf_dec(BUF_recive[i].group,1);}
		break;
	case 113:
			if(BUF_recive[i].type==1) {wr_strbuf("Voda utechka. ",0);bild_SMS_zone(BUF_recive[i].zone,5);wr_strbuf(" Gruppa: ",1);wr_ptbuf_dec(BUF_recive[i].group,1);}
				else {wr_strbuf(" ",0);bild_SMS_zone(BUF_recive[i].zone,4);wr_strbuf(" Gruppa: ",1);wr_ptbuf_dec(BUF_recive[i].group,1);}
		break;
//--�������� �������--
	case 150:

	case 130:
	case 131:
	case 132:
	case 133:
	case 134:
	case 135:
	case 136:
	case 137:
			wr_strbuf(" ",0);
			if(BUF_recive[i].type==1) {bild_SMS_zone(BUF_recive[i].zone,3);wr_strbuf(" Gruppa: ",1);wr_ptbuf_dec(BUF_recive[i].group,1);}
				else {bild_SMS_zone(BUF_recive[i].zone,4);wr_strbuf(" Gruppa: ",1);wr_ptbuf_dec(BUF_recive[i].group,1);}

		break;
	case 143:
			if(BUF_recive[i].type==1) {wr_strbuf("Neispraven rashiritel zon!",0);}
				else {wr_strbuf("Rashiritel zon vostanovlen.",0);}
		break;
	case 144:
			if(BUF_recive[i].type==1) {wr_strbuf("Tamper! ",0);bild_SMS_zone(BUF_recive[i].zone,5);wr_strbuf(" Gruppa: ",1);wr_ptbuf_dec(BUF_recive[i].group,1);}
				else {wr_strbuf("Tamper norma. ",0);bild_SMS_zone(BUF_recive[i].zone,4);wr_strbuf(" Gruppa: ",1);wr_ptbuf_dec(BUF_recive[i].group,1);}
		break;
	case 145:
			if(BUF_recive[i].type==1) {wr_strbuf("Narushenie tampera!",0);}
				else {wr_strbuf("Zona tampera",0);wr_strbuf(" vostanovlena.",1);}
		break;
//----- �������������
	case 300:
			if(BUF_recive[i].type==1) {wr_strbuf("Avarija vihoda dopolnitelnogo pitaniya!",0);}
				else {wr_strbuf("Vostanovlenie vihoda dopolnitelnogo pitaniya.",0);}
		break;
	case 301:
			if(BUF_recive[i].type==1) {wr_strbuf("Propalo setevoe pitanie!",0);}
				else {wr_strbuf("Setevoe pitanie vostanovleno.",0);}
		break;
	case 302:
			if(BUF_recive[i].type==1) {wr_strbuf("Akkumulator pitaniya neispraven!",0);}
				else {wr_strbuf("Akkumulator pitaniya ispraven.",0);}
		break;
	case 321:
			if(BUF_recive[i].type==1) {wr_strbuf("Neispravna sirena!",0);}
				else {wr_strbuf("Sirena vostanovlena.",0);}
		break;
	case 333:
			if(BUF_recive[i].type==1) {wr_strbuf("Neispravna klaviatura!",0);}
				else {wr_strbuf("Klaviatura vostanovlena.",0);}
		break;
	case 351:
		//	if(BUF_recive[i].type==1) {wr_strbuf("Neisprava telef. linija\0",0);}
			//	else 
					{wr_strbuf("Telef linija vostanovlena.",0);}
		break;
	case 372:
			if(BUF_recive[i].type==1) {wr_strbuf("Korotkoe zamikanie v zone! ",0);}
				else {wr_strbuf("Zamikanie v zone ustraneno. ",0);}
		break;
	case 373:
			if(BUF_recive[i].type==1) {wr_strbuf("Neispravna pogarnaya zona! ",0);}
				else {wr_strbuf("Pogarnaya zona vostanovlena.",0);}
		break;
	case 384:
			if(BUF_recive[i].type==1) {wr_strbuf("Razrjad batereyki vo vneshnem ustroystve! ",0);}
				else {wr_strbuf("Vostanovlenie batereyki vo vneshnem ustroystve.",0);}
		break;

//----���������� ������
	case 400:
			if(BUF_recive[i].type==1) {wr_strbuf("Specialnoe snyatie s ohrani.",0);}
				else	{wr_strbuf("Specialnaya postanovka na ohranu.",0);}
		break;
	case 401:
	case 402:
			if(BUF_recive[i].type==1) {wr_strbuf("Sistema snyata s ohrani. Kod polzovatelya: ",0);wr_ptbuf_dec(BUF_recive[i].zone,1);wr_strbuf(" Gruppa: ",1);wr_ptbuf_dec(BUF_recive[i].group,1);}
				else {wr_strbuf("Sistema vzyata pod ohranu. Kod polzovatelya: ",0);wr_ptbuf_dec(BUF_recive[i].zone,1);wr_strbuf(" Gruppa: ",1);wr_ptbuf_dec(BUF_recive[i].group,1);}
		break;
	case 405:
			 {wr_strbuf("Otmena avto-postanovki na ohranu. Kod polzovatelya: ",0);wr_ptbuf_dec(BUF_recive[i].zone,1);wr_strbuf(" Gruppa: ",1);wr_ptbuf_dec(BUF_recive[i].group,1);}
		break;
	case 406:
			wr_strbuf("Ohrana snyata posle trevogi.",0);
		break;
	case 408:		//������� ����������.
					{wr_strbuf("Uskorennaya postanovka na ohranu.",0);}
		break;
	case 409:
			if(BUF_recive[i].type==1) {wr_strbuf("Snyatie s ohrani tumblerom.",0);}
				else {wr_strbuf("Postanovka na ohranu tumblerom.",0);}
		break;
	case 421:
			wr_strbuf("Klaviatura zablokirovana.",0);
		break;
	case 454:
			if(BUF_recive[i].type==1) {wr_strbuf("Otsutstvie aktivnosti v zone!",0);}
				else {wr_strbuf("Aktivnostj obnarugena.",0);}
		break;
	case 470:
			wr_strbuf("Chastichnaja postanovka na ohranu.",0);
				break;
//--- �����
	case 601:
	case 602:
	case 603:
			wr_strbuf("Test.",0);
		break;
	case 623:
			wr_strbuf("Vnimanie! Bufer sobitiy zapolnen na 75%.",0);
		break;
	default:
		return 0;
 }
	return point_buf;			// ������� ������ ���������
}
//------------------------------------------------
void bild_SMS_zone(char num_zone,char regim)
{
char konf;
unsigned long adr_sms;
if(num_zone<32){konf=EE_READ(konf_SMS_Cid+num_zone);}
	else {konf=0;}
if (konf<40)
	{ 
	if(regim==3){wr_strbuf("Trevoga zona: ",1);}
	if(regim==4) {wr_strbuf("Vostanovlena zona: ",1);}
	if(regim==5) {wr_strbuf("Avarija zona: ",1);}
	}
	else
	{
	if(regim==4) {wr_strbuf("PODKL: ",1);}
	}

 switch(konf)
 {
	case 0:
			wr_ptbuf_dec(num_zone,1);
		break;
	case 1:
			wr_strbuf("dvigenija",1);			// 
		break;
	case 2:
			wr_strbuf("proniknoveniya",1);			// 
		break;
	case 3:
			wr_strbuf("napadenie",1);			// 
		break;
	case 4:
			wr_strbuf("voda",1);			// 
		break;
	case 5:
			wr_strbuf("pozhara",1);		// 
		break;
	case 6:
			wr_strbuf("dima",1);			// 
		break;
	case 7:
			wr_strbuf("gaza",1);			// 
		break;
	case 8:
			wr_strbuf("sirena",1);			// �����������
		break;
	case 9:
			wr_strbuf("klaviatura",1);			//�����
		break;
	case 10:
			wr_strbuf("dveri",1);			// 
		break;
	case 11:
			wr_strbuf("okna",1);			// 
		break;
	case 12:
			wr_strbuf("narugnij perimetr",1);			//
		break;
	case 13:
			wr_strbuf("fasad",1);			// 
		break;
	case 14:
			wr_strbuf("til",1);			// 
		break;
	case 15:
			wr_strbuf("Etazh 1",1);		// 
		break;
	case 16:
			wr_strbuf("Etazh 2",1);		// 
		break;

	case 40:
			wr_strbuf("Srabotal datchik ohrani",1);		//
		break;
	case 41:
			wr_strbuf("Srabotal datchik pozhara",1);		// 
		break;
	case 51:
			adr_sms=REC_SMS1;
			break;
	case 52:
			adr_sms=REC_SMS2;
			break;
	case 53:
			adr_sms=REC_SMS3;
			break;
	case 54:
			adr_sms=REC_SMS4;
			break;
	case 55:
			adr_sms=REC_SMS5;
			break;
	case 56:
			adr_sms=REC_SMS6;
			break;
	case 57:
			adr_sms=REC_SMS7;
			break;
	case 58:
			adr_sms=REC_SMS8;
			break;
	default:
		break;
 }
	if(konf>50)
			{
			Frame_Buf_Read(adr_sms,0);//��������� ���� ���
			wr_strbuf(Bufer_SPI,1);
			}
}



void check_post_snyat(unsigned char n_pos)
{
unsigned char i,code_rec;
	for (i=0;i<n_pos;i++)					// � ���� �������
		{		
		code_rec=BUF_recive[i].code	;	// ��������� ������ �� ���������� - ������
			switch(code_rec)
				{
				case 400:
				case 401:
				case 402:
				case 405:
				case 408:		//������� ����������
				case 409:
						if(BUF_recive[i].type==1) 
								{		
									EE_WRITE(system_status,0);led_inform=0;siren_on(0);	// ������ � ������
									set_out_ohran(1);		//���� ����� -����� ��������� �� ���������� 
									sost_trevog=0;				// �������� ��������� �������
								}
							else 
								{
									EE_WRITE(system_status,1);led_inform=3;					// ���������� �� ������
						  			time_cnt_bezdejstvie=cnt_bezdejstvie=EE_READ(time_STAND)*60;
						  			cnt_siren=EE_READ(time_SIREN)*60;siren_on(0);
						 			set_out_ohran(0);		//���� ����� -����� ��������� �� ���������� 
								}
					break;
				default:
					break;
				}
		}
}

//-------------------------------
void alarm_Contact_ID(char n_pos)
{
										//0	�� ������� ������ SMS �� ����������
										//1	�� ������� ������ SMS ���������� ����� ������� � ������, ���� ������� �� ������ ����������� �� �� ������ �� ������� ������
										//2	�� ������� ������ SMS ���������� ����� ������� � ����� ������
										//3	�� ������� ������ SMS ���������� � ������ �������, ����� ������
										//4	�� ������� ������ ���������� ������ SMS

char i,n_abon,count_dozvon,count_SMS,sms_regim;

for (i=0;i<10;i++){map_dozvon[i]=0;}	// �������� ����� �������
count_dozvon=EE_READ(lim_DOZVON);		// ����������� ������ �������
count_SMS=EE_READ(lim_SMS);				// ��������� ����������� ��� �� �����
fl_uspeh=0;								// ��� ��������� ������
	while(count_dozvon--)
	{
	for(n_abon=0;n_abon<10;n_abon++)
	 {
		if (map_dozvon[n_abon]) {continue;}		// ���� ����� �������� �� ������� - ���������
		sms_regim=read_regim(n_abon);
		if((Bufer_SPI[2]==0xFF)||(Bufer_SPI[2]==0x00)) {map_dozvon[n_abon]=1;continue;}		//���� ������ ���! ���������� 
		if(sms_regim=='3') {snd_contact_id_sms(n_abon,n_pos);}	// ���� ����� ������� SMS �� ��������� ���
		if(sms_regim!='4') {if(snd_contact_id_voice(n_abon,n_pos)){/*end_contact_id_dozvon();*/return;}} // ���������� ������� ���� ������ ��������! �����
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
				if(!fl_uspeh) snd_contact_id_sms(n_abon,n_pos);
				break;
		case '2':		//		�� ������� ������ SMS ���������� ����� ������� � ����� ������
				snd_contact_id_sms(n_abon,n_pos);
				break;
		case '3':
				break;	// ���� ����� ������� SMS �� ���������
		case '4':		//�� ������� ������ ���������� ������ SMS
			 snd_contact_id_sms(n_abon,n_pos);
			break;
		default:
			break;
		}
   	 }

	}
//	end_contact_id_dozvon();			// ������ ��������
}

//----------------
char snd_contact_id_voice(char n_abon,char n_pos)				//��������� ������ ��������� 1- ������� �������������, 0 ��� ���������
{
 char lim_time,i,work,kategorija,mark_opov_vostan;

kategorija=read_kateg(n_abon);
mark_opov_vostan=EE_READ(Opov_Vostan);
work=0;
for (i=0;i<n_pos;i++)	
	{
		work+=base_code (BUF_recive[i].code,kategorija,0,BUF_recive[i].type||mark_opov_vostan);//���� ����� ���� �� ��������� � ��������������� � ������ �������������� �� �������� ���
	}
if (!work) return 0;		// ���� ��� ����������� ����� �� �������

led_inform=6;
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
											// ����������
	for (i=0;i<n_pos;i++)					// � ���� �������
		{		
		if(base_code (BUF_recive[i].code,kategorija,0,BUF_recive[i].type||mark_opov_vostan))	voice_contact_id(i);		// �������� ������� ���� ��������� ���������
			else continue;
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
char voice_contact_id(char i)
{
unsigned char c,temp;
switch (BUF_recive[i].code)
 {
	case 100:
			if(BUF_recive[i].type==1)
					 {voice_speech(knopka,PWM1); voice_speech(trevoga,PWM1);}  // ������ �������
				else {return 0;}
		break;
	case 115:
			if(BUF_recive[i].type==1)
					 {voice_speech(knopka,PWM1); voice_speech(pogara,PWM1);}  //������ �����
				else {return 0;}
		break;
	case 120:
			if(BUF_recive[i].type==1)
					 {voice_speech(knopka,PWM1); voice_speech(napadenie,PWM1);}  //������ ������(���������)
				else {return 0;}
		break;
	case 121:
 			voice_speech(kod,PWM1);voice_speech(vkl,PWM1); voice_speech(pod_ugrozoj,PWM1);	// ����� � ������ ��� �������
		break;
//--�������� �������--
	case 110:
	case 111:
	case 112:
	case 114:
	case 117:
			if(BUF_recive[i].type==1) {voice_speech(Srab_pojag_sig,PWM1); voice_alarm(BUF_recive[i].zone,1,0);}// ���� � �������
				else {voice_alarm(BUF_recive[i].zone,1,1);}				// ���� ��������������
		    if(BUF_recive[i].group>0){	voice_speech(gruppa,PWM1); voice_speech(BUF_recive[i].group+cif_0,PWM1);}
		break;
	case 113:
			if(BUF_recive[i].type==1) {voice_speech(datchik,PWM1);voice_speech(vodi,PWM1); voice_alarm(BUF_recive[i].zone,1,0);}// ���� � �������
				else {voice_alarm(BUF_recive[i].zone,1,1);}				// ���� ��������������
		    if(BUF_recive[i].group>0){	voice_speech(gruppa,PWM1); voice_speech(BUF_recive[i].group+cif_0,PWM1);}
		break;
//--�������� �������--
	case 150:

	case 130:															// �������� ���� �� �������
	case 131:
	case 132:
	case 133:
	case 134:
	case 135:
	case 136:
	case 137:
			if(BUF_recive[i].type==1) {voice_alarm(BUF_recive[i].zone,1,0);}// ���� � �������
				else {voice_alarm(BUF_recive[i].zone,1,1);}				// ���� ��������������
		    if(BUF_recive[i].group>0){	voice_speech(gruppa,PWM1); voice_speech(BUF_recive[i].group+cif_0,PWM1);}
		break;

	case 144:
			if(BUF_recive[i].type==1) {voice_alarm(BUF_recive[i].zone,1,2);}// ���� � �������(������)
				else {voice_alarm(BUF_recive[i].zone,1,1);}				// ���� ��������������
		break;
//----�������������
	case 301:
			if(BUF_recive[i].type==1) {	voice_speech(avarija,PWM1);}			//setevoe ����������
				else {voice_speech(ispraven,PWM1);}
				voice_speech(datchik,PWM1); voice_speech(setevoe_n,PWM1);
		break;
	case 302:
			if(BUF_recive[i].type==1) {	voice_speech(avarija,PWM1);}			//���������� ����������
				else {voice_speech(ispraven,PWM1);}
				voice_speech(datchik,PWM1); voice_speech(batarejnoe_n,PWM1);
		break;
	case 321:
			if(BUF_recive[i].type==1) {	voice_speech(avarija,PWM1);}			//������
				else {voice_speech(ispraven,PWM1);}
				voice_speech(datchik,PWM1); voice_speech(sireni,PWM1);
		break;
	case 333:
			if(BUF_recive[i].type==1) {	voice_speech(avarija,PWM1);}			//����������
				else {voice_speech(ispraven,PWM1);}
				voice_speech(datchik,PWM1); voice_speech(klaviaturi,PWM1);
		break;
	case 351:
		//	if(BUF_recive[i].type==1) {wr_strbuf("Neisprava telef. linija\0",0);}
			//	else 
				voice_speech(ispraven,PWM1);
				voice_speech(datchik,PWM1); voice_speech(tel_linija,PWM1);		//���������� �����
		break;
	case 372:	//������ � ��������� ���� ��� �������					// �������� ���� �� �������
			if(BUF_recive[i].type==1){voice_speech(datchik,PWM1);voice_speech(avarija,PWM1);}
				else{voice_speech(datchik,PWM1);voice_speech(ispraven,PWM1);}
	case 373:
			if(BUF_recive[i].type==1) {	voice_speech(avarija,PWM1);}			//����������
				else {voice_speech(ispraven,PWM1);}
				voice_speech(datchik,PWM1); voice_speech(pogara,PWM1);
		break;
//---���������� ������-
	case 400:
	case 401:
	case 402:
	case 408:		//������� ����������.
			if(BUF_recive[i].type==1) 
				{	voice_speech(sistema,PWM1);voice_speech(snjata_s_ohrani,PWM1);
				}
				else 
				{	voice_speech(sistema,PWM1);voice_speech(pod_ohranoj,PWM1);
				}
			voice_speech(polzovatel,PWM1);voice_speech(kod,PWM1);
			if(BUF_recive[i].zone<=20){voice_speech(BUF_recive[i].zone+cif_0,PWM1);}
				else 
					{
					temp=c=BUF_recive[i].zone;
					//hundreds
					if ((c/100)>0) voice_speech((c/100)+cif_0,PWM1);
						c-=(c/100)*100;	
					//tens
					if (((temp/10)>0) || ((temp/100)>0)) voice_speech((c/10)+cif_0,PWM1);
					c-=(c/10)*10;
					//ones
						voice_speech((c/1)+cif_0,PWM1);
							}
			break;
	case 405:
				{	voice_speech(sistema,PWM1);voice_speech(snjata_s_ohrani,PWM1);
				 	voice_speech(polzovatel,PWM1);voice_speech(kod,PWM1);
					voice_speech(BUF_recive[i].zone+cif_0,PWM1);
				}
				break;

	case 409:		//���������� ������ ���������.
			if(BUF_recive[i].type==1) 
				{	voice_speech(sistema,PWM1);voice_speech(snjata_s_ohrani,PWM1);}
				else 
				{	voice_speech(sistema,PWM1);voice_speech(pod_ohranoj,PWM1);}
				break;
//--- �����
	case 601:
	case 602:
	case 603:
			voice_speech(sistema,PWM1);voice_speech(test,PWM1);					//�������� ���������
		break;
	default:
		return 0;
 }
	return 1;			// ������� ������ ���������

}
//------------------------------------------------

void  snd_contact_id_sms(char n_abon,char n_pos)				// SMS ��������  ��������- ����������� ���������
{
char temp,j,ss,cnt,pnt,kategorija,work,sms_regim,mark_opov_vostan;
mark_opov_vostan=EE_READ(Opov_Vostan);
kategorija=read_kateg(n_abon);
work=0;
for (ss=0;ss<n_pos;ss++){work+=base_code (BUF_recive[ss].code,kategorija,1,BUF_recive[ss].type||mark_opov_vostan);}
if (!work) return;		// ���� ��� ����������� ����� �� �������

putst("AT+CMGF=1\r");
delay_ms(500);
load_date_time();			// ��������� ����- �����
copy_lexem(&buffer_work[0],&STR_Buf[0]);

		sms_regim=read_regim(n_abon);

	j=0;			//������ ������ SMS
	cnt=0;			//��������� ���������
	while(j<n_pos)							//���� �������� �������������� SMS �����������  �����
			{ work=0;
			for	(ss=j;ss<n_pos;ss++) {work+=base_code (BUF_recive[ss].code,kategorija,1,BUF_recive[ss].type||mark_opov_vostan);}	// ����������� ������ �� �������� ���������
			if (!work) {return;}

			putst("AT+CMGS=\"");
  			snd_abon(n_abon);putch('"');putch(0x0d);	// ������������ �����
			delay_ms(255);
			putst_buf(&buffer_work[0]);				// ���������� ����-�����
			putst("\r");
				pnt=0;
				for	(;j<n_pos;j++)					//��������������� ������
					{
					if (!base_code (BUF_recive[j].code,kategorija,1,BUF_recive[j].type||mark_opov_vostan)) continue;	// ���� ��������� �� ������������� ��������� - ����������
					temp=build_SMS(j);
					if (!temp) {continue;}
					pnt+=temp;
					if (pnt>140)	break;			// ���� ������ SMS ����������� �� ��������� ��� � ������������� ���� �����
					cnt++;
					putchdec(cnt);					//����� ���������
					putch('-');
					putst_buf(&STR_Buf[0]);
					putch('\n');
					}

				putch(0x1A);			//cntrol Z
				check_OK(10);			//10 ������ ����� OK
				delay_s(1);
			}
}
