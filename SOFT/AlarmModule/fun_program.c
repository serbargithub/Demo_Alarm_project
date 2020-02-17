//������� ���������������� ����������
#include <pic18.h> 
#include "name_port.h"
#include "my_delay.h"
#include "usart.h" 
#include "usart_buf.h" 
#include "function.h"
#include "fun_abon.h"
#include "speech.h"
#include "name_konst.h"
#include "name_adres.h"
#include "my_spi_SST26.h"
#include "eeprom.h"
#include "signal.h"
#include "SMS_rutine.h"
#include "terminal.h"
#include "bluetooth.h"
#include "lexem_rutine.h"
#include "voice_map.h"

extern const char max_buf;
extern const char st_adr_abon;
extern unsigned char fl_ring_yes,led_inform,SW_uart,fl_bt_DATA_yes,fl_bt_CONNECT_yes,fl_bt_DISCONN_yes;
extern  unsigned char number_str[];
extern  unsigned char buffer_clip[];
extern  unsigned char work_string[];
unsigned char load_parametr (void);
void rutine_parametr(char num_command);
void program_regim(void);
extern void sound_error(void);
extern void sound_ok(void);
void detect_command(void);
unsigned char check_base_command (unsigned int num_command);
//----------------
void reset_nastrojki(void)
{
const char eep_reset1[]={ 
		0,
//10-19 ��������� ������������� ��������				
				'*','1','1','1','#',0,0,0,0,0,0,0,0,0,0, //0x01	USSD ������
				'5','5','5',0,		//0x10 ��� �������
				1,					//0x14 ��������� ������� ������ �� �����
				1,			 		//0x15 ��������� ���������� � ��������� ������������ �� �����
				0,					//0x16 ����� ����� ����
				0,					//0x17 ����������� ����������� 8�7�8
 				'1','2','3','4',0,			//0x18 PIN Bluetooth
				0,0,0,
				0,
//20-39 ��������� �������
				0,					//0x21 ��������� ������������ ���� ������� SMS
				0,					//0x22 ������ ��������� ���������� ��� �������������� ���
				1,					//0x23 ����� �������� �������� ���������
				0,					//0x24 ������ �������� �������� ��������� � �����
				8,					//0x25 ����� ������ �������� ���������
				0,					//0x26 ������ ������ � ���������� ������ 0 �� ������������ 1 SMS 2 voice
				1,					//0x27 ����� �������� ��������� � ������� ����������
				0,0,0,0,0,0,0,0,
				2,					//0x30 ����������� �������� �� �����
				1,					//0x31 ����������� ��� �� ����� 
				0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,
//40-59 ��������� ���
				10,					//0x40 ����� �� ����
				10,					//0x41 ����� �� �����
				10,					//0x42 ����� �����������
				10,					//0x43 ����������� ��������� ������� ������� �������������� /10
				0,0,0,0,
				1,3,3,1,1,1,1,1,1,1,1,1,1,1,1,1, //0x48 ������ ���������� �� ����� �� 47 (16 ���)
				0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x58	������������  ��� �� 57  0-������� 1-�������-���������	2-��������������,3-���� �����������
										 // ������� ��� � 1 ���� �������������. � 0 ��������������
				0,					//0x68 ������������ ���������� ������ 0 -���������� ������ ��������� 1 -���������� ������ ������� 2,3 - +��������     ��������� 4- ����� ������� ��� ������ � ����������
				0,					//0x69 ����� � ������� ��� �������������� �� ������ (���� 0 �� ��� ��������������)
				0,0,0,0,0,0,
//60-69 ��������� �������
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x70 ��������  ������� � ��������� ���������� �� 67 (8 �������) 
				1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x80 ������������  ������� �� 6F (16 �������)  
										 						   // 0- ����� ���������� �� 1 ��� 1- �������� 2- ������������ ����������
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x90	�������� ��� � �������   0-�������� ��� ��� ����� ������ ������������� ��� ������� � ����. �������� ������ � ����������� ��������  
				5,					//0xA0 ����� ������ ������
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const char eep_reset2[]={
//70-79 ��������� ��������� ���������
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xB0	������������  ��� 16 ���
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xC0	������������  VOICE 16 ���
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xD0	������������  ��� 32 ��� ��� Contact ID
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xF0 ������������  VOICE 32 ��� ��� Contact ID
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//80-89 ��������� �������������� �������.
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x110 ��������� ������� �������� ����������� (16 ��)0- �������� 1- ������� 2- ����� ������ 
										                           // 3- ����� ���������� 4 - ������ ����������
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x120 ��������� ������� �������� ����������� (16��) 0- �������� 1- SMS ����� 2- ��������� ����� 
				5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, //0x130 ������ ��������  �������� ����������� (16��)
				40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40, //0x140 ������� �������� �������� ����������� (16��)
				2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, //0x150 ���������� �����������				 (16��)
//100-119 ��������� Contact ID. 
				'5','4','3','2','1',0,0,0,0,0,0,0,0,0,0,0, //0x160  ��������� ���������� ������ ����� �������� contact_id
				0,					//0x170	��������� ����������-������ ��������� CONTACT ID 
				0,					//0x171	������ ��������� ������ ����������� ��������� � ���� ���������������� 
				0,0,0,0,0,0};		//End
				

const unsigned int mas_size1=sizeof(eep_reset1);
const unsigned int mas_size2=sizeof(eep_reset2);
unsigned int i_int;

		for (i_int=0;i_int<mas_size1;i_int++)	{ EE_WRITE(i_int,eep_reset1[i_int]);CLRWDT();}	//	�������� ��������� ���������
		for (i_int=0;i_int<mas_size2;i_int++)	{ EE_WRITE(i_int+mas_size1,eep_reset2[i_int]);CLRWDT();}	//	�������� ��������� ���������
		EE_WRITE(save_bort_napr,1);
		EE_WRITE(system_status,0);		// ����� � ������
		EE_WRITE(knopka_system,0);		// ������ ����� � ������
	// �������� ��������� ������������
		delay_ms(255);
		sound_ok();
		sound_ok();
}
//------------------------
void update_module(void)
{
	putst("AT\r");
	
	delay_ms(255);
	putst("AT+CHFA=0\r");
	delay_ms(500);
	putst("AT+CLIR=0\r");
	delay_ms(500);
	putst("AT+CMIC=0,4\r");
	delay_ms(500);
	putst("AT+CLVL=90\r");
	delay_ms(500);
    serial_setup(sp115200,PRI_US);
	delay_ms(50);
 	putst("AT+IPR=115200\r");
	delay_ms(500);
	putst("AT+SIDET=0,0\r");
	delay_ms(500);
	putst("AT+SIDET=2,0\r");
	delay_ms(500);
	putst("AT+CSSN=0\r");
	delay_ms(500);
	putst("AT+COLP=0\r");
	delay_ms(500);
	read_imei();
	delay_ms(500);
	put_command("AT+BTPOWER=1\r");			// �������� Bluetooth
	delay_s(3);
 	putst("AT+BTUNPAIR=0\r");
	delay_ms(500);
	Frame_Buf_Read(IMEI_Save,0);			// ��������� ����
 	putst("AT+BTHOST=");putst(Name_Bluetooth);putst_buf(Bufer_SPI+11);(putst("\r");
	delay_ms(500);
	put_command("AT+BTPOWER=0\r");			// ��������� Bluetooth
	delay_s(2);
	putst("AT&W\r");
	delay_ms(255);
		sound_ok();
		sound_ok();
}
//------------------------
void programm_abon(char vhod)
{
unsigned int i_int;
if (vhod) 							//���� ����� �� ������
  {
	fl_ring_yes=1;
	delay_s(1);
	if(vhod==1){if (sys_key) return;}				//��������� ������ ��� ���
 	put_command("ATH\r");						
 	led_inform=7;							// ������ �����
	for(i_int=0;i_int<60;i_int++)	{delay_ms(250);if (sys_key) break;}
	if(i_int==60)			//���� ���������� ������ ������ 15 ������ ������ �����
			{
			 led_inform=0;
			delay_s(1);
			led_inform=3;
			reset_nastrojki();
			update_module();
			led_inform=0;
			return;
			}
	fl_ring_yes=0;
	fl_bt_DATA_yes=0;
	fl_bt_CONNECT_yes=0;
	fl_bt_DISCONN_yes=0;
	clear_buf_RX(Bluetooth_CH);				//�������� ����� ������ Bluetooth
	put_command("AT+BTPOWER=1\r");			// �������� Bluetooth
	read_lexem(1,3);
	EE_READ_String(pin_bluetooth,work_string);// ��������� PIN �������
	putst("AT+BTPAIRCFG=1,");putst_buf(work_string);putst("\r");	// �������� Bluetooth
	read_lexem(2,1);

	for (i_int=0;i_int<1200;i_int++)						//������� 120 ��� ������
  	{
		if(fl_ring_yes)  break;
		if(fl_bt_CONNECT_yes) {Bluetooth_Connecting();putch(0x1A);break;}
		if(fl_bt_DISCONN_yes) {putch(0x1A);break;}
	 	delay_ms(100);
		if (!sys_key) {led_inform=0;delay_s(1);put_command("AT+BTPOWER=0\r");delay_s(2);return;} //��������� ������ ���� ������ �������
  	}
	delay_s(1);
	put_command("AT+BTPOWER=0\r");	// ��������� Bluetooth
	delay_s(1);
	if(!fl_ring_yes) return; 		//�� ��������� ������
	put_command("ATA\r");			// ����� ������
 	read_lexem(1,30);		// ��������� 1 ������� 30 ���
 	if (uncode_com("CONNECT")) {terminal_PDP();return;}	//������ �������
 	if (!uncode_com("OK")){putst("+++");delay_s(1);putst("\rATH\r");return;}//e��� ����� �� �� �����	
  }
	delay_s(1);
	change_in(voice_in);		
	delay_ms(250);
	voice_speech(regim_program,PWM1);		//����� ����������������

		delay_s(1);
	 while(1)
   		{ 
		if(detekt_razg()!=2) return;		// e��� �������� ��������� �� �����
		delay_s_dso(5);
		if (dso_vg) {if(set_dtmf()==rehetka)detect_command();}
		voice_speech(naberite_komandu,PWM1);					// �������� ��������
   		}
}
//-----------------------------
unsigned int calk_DTMF_parametr (unsigned char n_cif)
{
unsigned int wrk_int,mnog,i;
	if(!n_cif) return 0;
	wrk_int=0;
	mnog=1;						// ��������� �������
	for (i=n_cif;i!=0;i--)
	{
	wrk_int+=((unsigned int)(number_str[i-1]-'0'))*mnog;
	mnog*=10;	
	}
	return wrk_int;
}
//---------------
void Voice_param_prinjat (void)
{	delay_ms(500);voice_speech(param_prinjat,PWM1);}					// �������� ������

void set_Single_Parametr(unsigned char max,unsigned int eep_adr,unsigned int param) //������ ��������� � ������ � ��������� � �������
{	if  (param>max){sound_error(); return;}		 // ��������� ��������
	EE_WRITE(eep_adr,(unsigned char)param);		// ��������� ��������
	Voice_param_prinjat();			// �������� ������
}
void set_Two_Parametr(unsigned char max1,unsigned char max2,unsigned int eep_adr,unsigned int param1,unsigned int param2) //������ ��������� � ������ � ��������� � �������
{	if  (param1>max1){sound_error(); return;}		 // ��������� ��������
	if  (param2>max2){sound_error(); return;}		 // ��������� ��������
	EE_WRITE(eep_adr+param1,(unsigned char)param2);		// ��������� ��������
	Voice_param_prinjat();			// �������� ������
}
//--------
void detect_command(void)
{
unsigned char n_cif_param,i,work,num_abon,ukaz,type_command;
unsigned int num_command,num_parametr1,num_parametr2;
	n_cif_param=load_parametr();
	if ((!n_cif_param)||(n_cif_param>4))	{sound_error(); return;}	//������� ��������, ���� � �������- ����� 
	num_command=calk_DTMF_parametr(n_cif_param);		//��������� ����� ��������
	type_command=check_base_command(num_command); //��������� �� ���� ��� �������� (1 ��� 2 ���������)
	if (!type_command)	{sound_error(); return;} //���� ���������� � ���� �� ������� ����� � �������
	if (type_command>20){sound_error(); return;} //���� ���������� � ���� ������ SMS ����� � �������
	if (type_command>10){type_command-=10;}      //���� ���������� � ���� ������ DTMF �� ��������� ��������
	n_cif_param=load_parametr();
	if (!n_cif_param)	{sound_error();return;}	//������� �������� ��������� ����������� ���� ��������� ���� 0 ����,������
	if (n_cif_param<=3) {num_parametr1=calk_DTMF_parametr(n_cif_param);}	//���� �������� �� 3 � ���� �� ��������� ����� ��������� 1
	if(type_command==2)								// ���� � ������� 2 ��������� �� ��������� ������ � ��������� ������
		{
		n_cif_param=load_parametr();
		if (!n_cif_param)	{sound_error(); return;}	//������� �������� 2, ���� � �������- ����� 
		if (n_cif_param<=3) {num_parametr2=calk_DTMF_parametr(n_cif_param);}	//���� �������� �� 3 � ���� �� ��������� ����� ��������� 2
		}

if(num_command==3775)			//������� ������ �� ��������� ���������
			{
			if (num_parametr1==0) {reset_nastrojki();voice_speech(nast_po_umolch,PWM1); while(1){};}	//  ���� ��� ������ �� �������� ��� ���������
			if (num_parametr1==1) {update_module();	voice_speech(nast_po_umolch,PWM1);  while(1){};}   	//  ���� ��� 37751 �� �� �������� ��� ��������� ������
			return;
			}	

switch (num_command)						//������ ��������
	{
	case 0: break;
//1-9 ��������� ������� ���������
	case 1:										// ������ ��������
		if  ((!num_parametr1)||(num_parametr1>15)){sound_error(); return;}				
		Bufer_SPI[0]='0';				// ��������� ��� ���������
		Bufer_SPI[1]='0';
 		for (i=0;i<14;i++)
		{
		work=buffer_clip[i];
		if(work=='"'){ break;}
		Bufer_SPI[i+2]=work;
		}
		Bufer_SPI[i+2]=0x00;

		num_abon=num_parametr1-1;
		write_num(num_abon);
		delay_ms(500);voice_speech(nomer_sohranen,PWM1);					// ����� ��������
		break;
 	case 2:										//�������� ��������
		if  ((!num_parametr1)||(num_parametr1>15)){sound_error(); return;}
		num_abon=num_parametr1-1;
		clear_num(num_abon);
		Voice_param_prinjat();				// �������� ������
		break;
	case 3:										//���������������� ��������� ��������
		if  ((!num_parametr1)||(num_parametr1>15)){sound_error(); return;}
		if (num_parametr2>9){sound_error(); return;}
		num_abon=num_parametr1-1;
		read_kateg(num_abon);					// ��������� ���
		Bufer_SPI[0]=num_parametr2+'0';
		write_num(num_abon);				// �������� ����� ��������
		Voice_param_prinjat();				// �������� ������
		break;
 	case 4:										//���������������� ������ ������ ��������
		if  ((!num_parametr1)||(num_parametr1>15)){sound_error(); return;}
		if (num_parametr2>9){sound_error(); return;}
		num_abon=num_parametr1-1;
		read_regim(num_abon);					// ��������� ���
		Bufer_SPI[1]=num_parametr2+'0';
		write_num(num_abon);				// �������� ����� ��������
		Voice_param_prinjat();					// �������� ������
		break;
	case 9:
										//�������� ���� ���������
		if  (num_parametr1!=1){sound_error(); return;}
			for (i=0;i<15;i++) 
				{clear_num(i);CLRWDT();}
		Voice_param_prinjat();					// �������� ������
			break;
//10-19 ��������� ������������� ��������
	case 10: 							//USSD ������
			if (n_cif_param>15) {sound_error(); return;}	//���� ������ 15 � ����
			n_cif_param++;
			number_str[n_cif_param]=0;
			ukaz=copy_buf(Bufer_SPI,number_str,0,MAX_BUF_RX_COM);//����������� �� 0
			Frame_Buf_Write(USSD_Balans,ukaz+1);
			Voice_param_prinjat();
			break;
	case 11:							//���������������� ���� �������
			if (n_cif_param!=3) {sound_error(); return;}	//�� 3 �����
			while (n_cif_param)
			{
				n_cif_param--;
				EE_WRITE(kod_parol+n_cif_param,number_str[n_cif_param]);	//�������� �������� �����
			}
			EE_WRITE(zero_place,0);
			Voice_param_prinjat();	
			break;
	case 12:							//	������ ���� ������� ��� ����� � �������.
			set_Single_Parametr(1,Menu_pass,num_parametr1);	break;
	case 13:							//	���������� � ��������� �������.
			set_Single_Parametr(1,Voice_otchet,num_parametr1);	break;
	case 14:							//	����� ����� ���������� ���� � ���������.
			set_Single_Parametr(1,menu_language,num_parametr1);	break;
	case 15:							//���������������� PIN ���� BLUETOOTH
			if (n_cif_param!=4) {sound_error(); return;}	//�� 4 �����
			while (n_cif_param)
			{
				n_cif_param--;
				EE_WRITE(pin_bluetooth+n_cif_param,number_str[n_cif_param]);	//�������� �������� �����
			}

//20-39 ��������� �������
	case 20:								//��������� ��������� ���� � �������
			if (n_cif_param!=10){sound_error(); return;}
			putst("AT+CCLK=\"");
			putch(number_str[0]);
			putch(number_str[1]);
			putch('/');
			putch(number_str[2]);
			putch(number_str[3]);
			putch('/');
			putch(number_str[4]);
			putch(number_str[5]);
			putch(',');
			putch(number_str[6]);
			putch(number_str[7]);
			putch(':');
			putch(number_str[8]);
			putch(number_str[9]);
			putst(":00+00\"\r");
			delay_ms(250);
			delay_ms(250);
			putst("AT&W\r");
			Voice_param_prinjat();	
			break;
	case 21:							//������������ ���� � SMS ���������.
			set_Single_Parametr(1,on_Date_Time,num_parametr1);	break;
	case 22:							//���������� � �������������� ���.
			set_Single_Parametr(1,Opov_Vostan,num_parametr1);	break;
	case 23:							//����� �������� �������� ���������
			set_Single_Parametr(3,regim_TEST,num_parametr1);	break;
	case 24:							//������ �������� �������� ���������.
			set_Single_Parametr(255,period_TEST,num_parametr1);	break;
	case 25:							//����� ������ ������� ��������� ���������. 
			set_Single_Parametr(23,start_TEST,num_parametr1);	break;
	case 26:							//��������� � ���������� � ������ � ������ 
			set_Single_Parametr(2,otchet_postan,num_parametr1);	break;
	case 27:							//��������� � �������� �������� �������.
			set_Single_Parametr(2,regim_BORT_napr,num_parametr1);	break;

	case 30:							//���������� ������� �� �����
			set_Single_Parametr(255,lim_DOZVON,num_parametr1);	break;
	case 31:							//���������� SMS �� �����
			set_Single_Parametr(255,lim_SMS,num_parametr1);	break;
//40-59 ��������� ���
	case 40:							//�������� �� ����.
			set_Single_Parametr(255,time_VHOD,num_parametr1);	break;
	case 41:							//�������� �� �����.
			set_Single_Parametr(255,time_VIHOD,num_parametr1);	break;
	case 42:							//�������� ����������� ��� ���� �����������.
			set_Single_Parametr(255,time_STAND,num_parametr1);	break;
	case 43:							//����� ������� ���.
			set_Single_Parametr(255,Save_const_dat_wait,num_parametr1);	break;
	case 44:							//����� ������ ���������� �� ����. (15 ���)
			set_Two_Parametr(15,3,regim_napr,num_parametr1,num_parametr2);	break;
	case 45:							//������������ (����� ����) ���  (15 ���)
			set_Two_Parametr(15,3,konf_ZONE,num_parametr1,num_parametr2);	break;
	case 46:							//���������� ���  ���������� �������� ���� ������������ � 1 (15 ���)
			if  (num_parametr2>1){sound_error(); return;}
			work=EE_READ(konf_ZONE+num_parametr1);
			if(!num_parametr2) work&=0x7F;
			if(num_parametr2)  work|=0x80;
			set_Two_Parametr(15,131,konf_ZONE,num_parametr1,work);	break;

	case 50:							//����� ������ ����� ���������� ������
			set_Single_Parametr(4,Conf_post_sn,num_parametr1);	break;
	case 51:							//�������������� �� ������.
			set_Single_Parametr(255,time_auto_set,num_parametr1);	break;
//60-69 ��������� �������
	case 60:							//������������ ����� ������ � ���������� (����� 1 ��� ������ 0) (16 �������)
			set_Two_Parametr(15,6,vihod_VOICE,num_parametr1-1,num_parametr2);	break;
	case 61:							//������������ ����� ������ � ���������� (����� 1 ��� ������ 0) (16 �������)
			set_Two_Parametr(15,2,konf_VIHOD,num_parametr1-1,num_parametr2);	break;
	case 62:							//�������� ��� � �������. (16 ��� 16 �������)
			set_Two_Parametr(16,16,link_Zone_Vih,num_parametr1,num_parametr2);	break;
	case 63:							//����� ������ ������.
			set_Single_Parametr(255,time_SIREN,num_parametr1);	break;
//70-79 ��������� ��������� ���������
	case 70:							//SMS ������ ��� ��� ������ . (15 ���)
			set_Two_Parametr(15,255,konf_SMS,num_parametr1,num_parametr2);	break;
	case 71:							//��������� ������ ��� ��� ������  (15 ���)
			set_Two_Parametr(15,255,konf_VOICE,num_parametr1,num_parametr2);	break;
	case 72:							//SMS ������ ��� ������� Contact ID ��� (31 ���)
			set_Two_Parametr(31,255,konf_SMS_Cid,num_parametr1,num_parametr2);	break;
	case 73:							//��������� ������ ��� ������� Contact ID ��� (31 ���)
			set_Two_Parametr(31,255,konf_VOICE_Cid,num_parametr1,num_parametr2);break;

	case 75:							//������ ����� ��������� ��������� ��� ������� ���
			if ((!num_parametr1)||(num_parametr1>16)){sound_error(); return;}
			voice_rec(num_parametr1);break;	
	case 76:							//������ ����� ��������� ��������� ��� ������� ���
			if ((!num_parametr1)||(num_parametr1>16)){sound_error(); return;}
			voice_play(num_parametr1,PWM1);break;
//80-89 ��������� �������������� �������.
	case 80:							//������������  ������������� ��������.(16 ��������)
			set_Two_Parametr(15,3,Conf_TEMP_DAT,num_parametr1-1,num_parametr2);	break;
	case 81:							//��������� ������� ���������� ������� �����������.(16 ��������)
			set_Two_Parametr(15,2,Inf_TEMP_DAT,num_parametr1-1,num_parametr2);	break;
	case 82:							//������ ����� �������� �����������.(16 ��������)
			set_Two_Parametr(15,255,TEMP_LOW,num_parametr1-1,num_parametr2);	break;
	case 83:							//������� ����� �������� �����������. (16 ��������)
			set_Two_Parametr(15,255,TEMP_HIGH,num_parametr1-1,num_parametr2);break;
	case 84:							//���������� �������� �����������.(16 ��������)
			set_Two_Parametr(15,255,TEMP_GIST,num_parametr1-1,num_parametr2);break;
//100-119 ��������� Contact ID.
	case 100: 							//����� �������� ��� ������ Contact ID
			if (n_cif_param>15) {sound_error(); return;}	//���� ������ 15 � ����
			number_str[n_cif_param]=0; // �������� ����� ������ �� ����� �������
			ukaz=copy_buf(Bufer_SPI,number_str,0,MAX_BUF_RX_COM);//����������� �� 0
			Frame_Buf_Write(Number_Contact_ID,ukaz+1);
			Voice_param_prinjat();
	case 101:							//��������� ����������-������ ��������� CONTACT ID 
			set_Single_Parametr(1,post_CONACT_ID,num_parametr1);	break;
	case 102:							//��������� ����������� �������� CONTACT ID 
			set_Single_Parametr(1,CONTACT_ID_print,num_parametr1);	break;


	default:
			sound_error();
			break;
	}
		
//sound_ok();
	delay_s(1);
}

//------
unsigned char check_base_command (unsigned int num_command)
{
  switch (num_command)						//������ ��������
	{
	case 0: return 0;
//1-9 ��������� ������� ���������
	case 1:	return 11;	// ������ DTMF
	case 2:	return 11;	// ������ DTMF	
	case 3:	return 2;	
	case 4:	return 2;
	case 5:	return 22;	// ������ SMS
	case 9:	return 11;	// ������ DTMF
//10-19 ��������� ������������� ��������
	case 10:	return 1;	
	case 11:	return 1;	
	case 12:	return 1;	
	case 13:	return 1;
	case 14:	return 1;
//20-39 ��������� �������
	case 20:	return 1;	
	case 21:	return 1;	
	case 22:	return 1;	
	case 23:	return 1;
	case 24:	return 1;
	case 25:	return 1;	
	case 26:	return 1;	
	case 27:	return 1;	
	case 30:	return 1;
	case 31:	return 1;
//40-59 ��������� ���
	case 40:	return 1;	
	case 41:	return 1;	
	case 42:	return 1;	
	case 43:	return 1;
	case 44:	return 2;
	case 45:	return 2;	
	case 46:	return 2;	
	case 50:	return 1;	
	case 51:	return 1;
//60-69 ��������� �������
	case 60:	return 2;	
	case 61:	return 2;	
	case 62:	return 2;	
	case 63:	return 1;
//70-79 ��������� ��������� ���������
	case 70:	return 2;	
	case 71:	return 2;	
	case 72:	return 2;	
	case 73:	return 2;
	case 74:	return 22;   //������ SMS
	case 75:	return 11;   // ������ DTMF	
	case 76:	return 11;	// ������ DTMF
//80-89 ��������� �������������� �������.
	case 80:	return 2;	
	case 81:	return 2;	
	case 82:	return 2;	
	case 83:	return 2;
	case 84:	return 2;
//100-119 ��������� Contact ID.
	case 100:	return 1;	
	case 101:	return 1;	
	case 102:	return 1;
// �������� ������ �� ��������� ���������
	case 3775:  return 11;   // ������ DTMF	
	}
return 0;
}
//------------------------------------------------
unsigned char load_parametr (void)
{ 
	 unsigned char n_cif,wr_prom;
	wr_prom=0;
	n_cif=0;
	while (wr_prom!=rehetka)
	{
		wr_prom=number_str[n_cif]=wait_dtmf(30);								// ��� ���� ����
		if (!(wr_prom&&(wr_prom^0xFF))) {return 0;}	//���� ����� ������ � �������, ��� ������� �����, ��� ������ * ����� c �������
		if((++n_cif)>=MAX_NUMB) {return  0;}								//���� ���� ������ 32  ������
	}
	n_cif--;
 return n_cif;							//������� ��������� �� ��������� �����(�������)	
}


