#include <pic18.h>
#include "ctype.h"
#include "name_port.h"
#include "my_delay.h"
#include "function.h"
#include "signal.h" 
#include "usart.h" 
#include "usart_buf.h" 
#include "name_konst.h"
#include "name_adres.h"
#include "eeprom.h"
#include "my_spi_SST26.h"
#include "fun_abon.h"
#include "terminal.h"
#include "lexem_rutine.h"
#include "SMS_rutine.h"

extern unsigned char SW_uart;		//������������� �������� COM �����
extern unsigned char buffer_clip[],Bufer_SPI[],fl_ring_yes;
extern  unsigned char led_inform,sost_trevog,dist_knop;
extern  unsigned int cnt_bezdejstvie,time_cnt_bezdejstvie,cnt_siren,CNT_time_auto_set;
unsigned char work_buf[MAX_BUF_RX_COM],work_buf1[MAX_BUF_RX_COM];  // ������������� ������ �������� ������
unsigned char buf_sms_command[MAX_BUF_RX_COM];  // ����� ������ SMS ������

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

void run_smscommand(unsigned char n_abon);
//--------------------------------------------------------------
unsigned long calk_parametr_SMS(char ukaz)					//���������� ��������� �� ������ ( �������� �������� ���� ������ )
{
unsigned long mnog,work_long;
ukaz--;			//���������
work_long=0;
mnog=1;						// ��������� �������
while ((buf_rx_command[ukaz]!='=')&&(buf_rx_command[ukaz]!=';')&&(buf_rx_command[ukaz]!=0x20))
	{
	work_long+=(buf_rx_command[ukaz]-'0')*mnog;	
	mnog*=10;
	if (!(ukaz--)) return 0;		// ������ �� ������
	}
return work_long;
}
//----------------------
unsigned int calk_first_parametr_SMS(void)		//���������� ������� ��������� �� ������( �������� ��������� �� =
{
unsigned char ukaz;
unsigned int mnog,work_int;
ukaz=find_N_char(buf_rx_command,'=',1,MAX_BUF_RX_COM);	// ����� =
ukaz--;			//���������
work_int=0;
mnog=1;						// ��������� �������
while ((buf_rx_command[ukaz]>='0')&&(buf_rx_command[ukaz]<='9'))
	{
	work_int+=(buf_rx_command[ukaz]-'0')*mnog;	
	mnog*=10;
	if (!(ukaz--)) return 0;		// ������ �� ������
	}
return work_int;
}
//-----------------
unsigned char calk_signed_parametr_SMS(void)		//���������� ��������� ���������
{
unsigned char work;
unsigned char ukaz;
unsigned int mnog;
ukaz=find_N_char(buf_rx_command,0,1,MAX_BUF_RX_COM);	// ����� 00
ukaz--;			//���������
work=0;
mnog=1;						// ��������� �������
while ((buf_rx_command[ukaz]>='0')&&(buf_rx_command[ukaz]<='9'))
	{
	work+=(buf_rx_command[ukaz]-'0')*mnog;	
	mnog*=10;
	if (!(ukaz--)) return 0;		// ������ �� ������
	}
if (buf_rx_command[ukaz]=='-'){return (0-work);}
return work;
}
//----------------------------------
void save_parametr_SMS_EE(unsigned long Adres)
{
unsigned char ukaz_tek,ukaz,i;
		ukaz_tek=find_N_char(buf_rx_command,'=',1,MAX_BUF_RX_COM);	// ����� =
		ukaz_tek++;
		ukaz=copy_buf(work_buf,&buf_rx_command[ukaz_tek],0x00,MAX_BUF_RX_COM);//����������� �� ;
		work_buf[ukaz]=0;		//������� �����
		for (i=0;i<=ukaz;i++){EE_WRITE(Adres+i,work_buf[i]);}
}


//---
void save_parametr_SMS(unsigned long Adres)
{
char ukaz_tek,ukaz;
		ukaz_tek=find_N_char(buf_rx_command,'=',1,MAX_BUF_RX_COM);	// ����� =
		ukaz_tek++;
		ukaz=copy_buf(Bufer_SPI,&buf_rx_command[ukaz_tek],0x00,MAX_BUF_RX_COM);//����������� �� 00
		Bufer_SPI[ukaz]=0;		//������� �����
		Frame_Buf_Write(Adres,ukaz+1);
}

//----------------------------------
void save_kategoriya_SMS(unsigned long Adres)
{
unsigned char ukaz_tek,ukaz,i;
		ukaz_tek=find_N_char(buf_rx_command,'=',1,MAX_BUF_RX_COM);	// ����� =
		ukaz_tek++;
		EE_WRITE(Adres,buf_rx_command[ukaz_tek]);
		EE_WRITE(Adres+1,buf_rx_command[ukaz_tek+1]);
}
//---------------------------------
unsigned char read_abonent_SMS(unsigned char n_abon)
{
unsigned char ukaz,ukaz1,ukaz_m,work,mnog;
unsigned int work_int,adr_int1,adr_int2;
ukaz=ukaz1=find_N_char(buf_rx_command,';',1,MAX_BUF_RX_COM);	// ����� ;
ukaz_m=find_N_char(buf_rx_command,'-',1,MAX_BUF_RX_COM);	// ����� -
if(ukaz_m>ukaz) ukaz_m=0xFF;				//���� ����� ���� �� ��������� ��������, ������� ������� ���������� ����
ukaz--;			//���������

if(ukaz_m!=0xFF)
	{
	adr_int2=0;
	mnog=1;						// ��������� �������
	while (buf_rx_command[ukaz]!='-')								//��������� �������� �����
		{
		adr_int2+=(buf_rx_command[ukaz]-'0')*mnog;					// ��������� ����� ������ 
		mnog*=10;
		if (!(ukaz--)) break;		// ������ �� ������
		}
	if((adr_int2<1)||(adr_int2>15)) return 3;						// ����� �� ��������- ������
	adr_int2--;	
	ukaz--;			//���������
	}
adr_int1=0;
mnog=1;						// ��������� �������
while (buf_rx_command[ukaz]!='=')
	{
	adr_int1+=(buf_rx_command[ukaz]-'0')*mnog;					// ��������� ����� ������ 
	mnog*=10;
	if (!(ukaz--)) break;		// ������ �� ������
	}
if((adr_int1<1)||(adr_int1>512)) return 3;						// ����� �� ��������- ������
adr_int1--;											// ���������(������ � ������ 0)
if(ukaz_m==0xFF){adr_int2=adr_int1;}				// ���� ��������� ���� ������ �� ��������� �����
if((adr_int2-adr_int1)>5){adr_int2=adr_int1+5;}		// ���������� ����������� �������

return 1;
}
//---------------------------------
unsigned char save_abonent_SMS(void)
{
unsigned char ukaz,ukaz1,i,work,mnog;
unsigned int work_int;
ukaz=ukaz1=find_N_char(buf_rx_command,'=',1,MAX_BUF_RX_COM);	// ����� =
ukaz--;			//���������
work_int=0;
mnog=1;						// ��������� �������
while (buf_rx_command[ukaz]!='N')
	{
	work_int+=(buf_rx_command[ukaz]-'0')*mnog;					// ��������� ����� ������ 
	mnog*=10;
	if (!(ukaz--)) break;		// ������ �� ������
	}
if((work_int<1)||(work_int>15)) return 3;						// ����� �� ��������- ������
work_int--;											// ���������(������ � ������ 0)
ukaz=ukaz1+1;
for (i=0;i<16;i++)												//���������� �����
	{
		work=buf_rx_command[ukaz];
		if((work==';')||(!work)) break;
		Bufer_SPI[i]=work;
		ukaz++;
	}
	Bufer_SPI[i]=0;				// ������� ����� ������
	write_num(work_int);	// ��������� �����
return 2;
}

//-----------------------------------
unsigned char detect_smscommand(unsigned char n_abon )		//���������� SMS ������� 0 �� ���������� 1
									// 						1 - ����������, �� ������� ������
									//						2- ����������, ������� ������
									//						3-���������� �� � ���������� ������
{
unsigned char ukaz_tek,wrk_char,cnt_rc,i,ukaz,end_zone;
unsigned long wrk_long;
unsigned int wrk_int;

for(i=0;i<MAX_BUF_RX_COM;i++) 
	{buf_rx_command[i]=(char)toupper(buf_rx_command[i]);
		if((buf_rx_command[i]=='=')||(buf_rx_command[i]==';')||(buf_rx_command[i]==0))break;	// �������� � �������� ��������

	}

ukaz=find_N_char(buf_rx_command,0,1,MAX_BUF_RX_COM);	// ����� ;
//��������� ��������
 if (uncode_com("BALSTR=")) 				//������ ������� ��������
		{
		if (ukaz>23) return 3;
		save_parametr_SMS_EE(USSD_Balans);
		return 2;
		} 
 if (uncode_com("PASS=")) 				//������ ����� � ����
		{
		if (ukaz>8) return 3;
		save_parametr_SMS_EE(pin_bluetooth);
		return 2;
		} 
 if (uncode_com("BPIN=")) 				//������ ����� � ����
		{
		if (ukaz>9) return 3;
		save_parametr_SMS_EE(kod_parol);
		return 2;
		} 
 if (uncode_com("UM")) 				//���� SMS ������
		{
 		wrk_int=calk_first_parametr_SMS();
		if((!wrk_int)||(wrk_int>15)) return 3;  //�������� ������� ����������
		switch (wrk_int)
		{
		case 1:		wrk_long= REC_SMS1;break;
		case 2:		wrk_long= REC_SMS2;break;
		case 3:		wrk_long= REC_SMS3;break;
		case 4:		wrk_long= REC_SMS4;break;
		case 5:		wrk_long= REC_SMS5;break;
		case 6:		wrk_long= REC_SMS6;break;
		case 7:		wrk_long= REC_SMS7;break;
		case 8:		wrk_long= REC_SMS8;break;
		case 9:		wrk_long= REC_SMS9;break;
		case 10:	wrk_long= REC_SMS10;break;
		case 11:	wrk_long= REC_SMS11;break;
		case 12:	wrk_long= REC_SMS12;break;
		case 13:	wrk_long= REC_SMS13;break;
		case 14:	wrk_long= REC_SMS14;break;
		case 15:	wrk_long= REC_SMS15;break;
		}
		save_parametr_SMS(wrk_long);
		return 2;
		} 
 if (uncode_com("LT")) 				//�������� �����������
		{
 		wrk_int=calk_first_parametr_SMS();
		if((!wrk_int)||(wrk_int>2)) return 3;  //�������� ������� ����������
		wrk_char=calk_signed_parametr_SMS();
		EE_WRITE(wrk_int+TEMP_LOW-1,wrk_char);
		return 2;
		} 
 if (uncode_com("HT")) 				//�������� �����������
		{
 		wrk_int=calk_first_parametr_SMS();
		if((!wrk_int)||(wrk_int>2)) return 3;  //�������� ������� ����������
		wrk_char=calk_signed_parametr_SMS();
		EE_WRITE(wrk_int+TEMP_HIGH-1,wrk_char);
		return 2;
		} 
 if (uncode_com("GT")) 				//�������� �����������
		{
 		wrk_int=calk_first_parametr_SMS();
		if((!wrk_int)||(wrk_int>2)) return 3;  //�������� ������� ����������
		wrk_char=calk_signed_parametr_SMS();
		EE_WRITE(wrk_int+TEMP_GIST-1,wrk_char);
		return 2;
		} 
if (uncode_com("N")) 				//�������� ����� �������� � ������
		{
		return save_abonent_SMS();
		} 
if (uncode_com("RD=")) 				//��������� ������ ���������
		{
		return read_abonent_SMS(n_abon);
		} 
if (uncode_com("ABON")) 				//��������� ������ ���������
		{
		send_sms_abonenti(n_abon);
		return 1;
		} 
if (uncode_com("CONFIG")) 				//��������� ������ ���������
		{
		send_sms_nastroiki(n_abon);
		return 1;
		} 
//-------------------------------------------
if (uncode_com("AT+WR=")) 				//������ ����� � EEPROM �� ������
		{
		multi_write(1);
		return 2;			//	
		}   
if (uncode_com("AT+RD=")) 				//������ ����� � EEPROM �� ������
		{
		multi_read(1);
		return 2;			//	
		}  
//����������� ��������(��������)
if (uncode_com("BALANCE")) 				//�������� ������
		{
		delay_s(2);
		putst("ATH\r");
		delay_s(1);
		send_sms_balans(n_abon);
		return 1;			//	
		} 
if (uncode_com("ARM")) 				//��������� ��� ������
		{
					init_datchik();						// �������� �������
					set_out_ohran(1);		//���� ����� 3-����� ��������� ��  ����� � ������
					delay_s(1);
					if(check_zone_alarm(0)) 
							{led_inform=6;delay_s(2);	//++ ���� ���� �� ������� �� ���������� � �����
							wr_strbuf("Arming Error\r",1);	
							end_zone=last_zon;
							for (i=1;i<=end_zone;i++)
								{if(buf_dat[i].alarm){	wr_strbuf("Zone ",1);wr_ptbuf('0'+i,1);	wr_strbuf(": Alarm\r",1); }}
							 led_inform=0;
							}
					else {
							
						  time_cnt_bezdejstvie=cnt_bezdejstvie=EE_READ(time_STAND)*60;
						  cnt_siren=EE_READ(time_SIREN)*60;siren_on(0);
						 	set_out_ohran(0);		//���� ����� 3-����� ��������� �� ���������� 
							if (EE_READ(Conf_post_sn)==4) 			//���� ������� �������� ����� ���������� ������ �� ����������������� ��� ���������  5 ������			
								{
									for (i=0;i<50;i++)				//����� 5 ������
									{
										delay_ms(100);
										// �������� ��������� ������ ���������� ������ �����- ��� �������
										if(buf_dat[0].sost!=(buf_dat[0].triger&0x0F))						//���� �������� ��������� �� 
											{
									 			EE_WRITE(system_status,1);led_inform=3;
											//	sw_opov_post_snyat=2;  //��������� ���� ��� ����������
												sound_opoveshenie(1);
												break;
											}
									}
								 if(i==50) {wr_strbuf("Arming Error\r Base zone Error\r",1);	}
								}
								else
									 {	EE_WRITE(system_status,1);led_inform=3;
								//		sw_opov_post_snyat=2;  //��������� ���� ��� ����������
										sound_opoveshenie(1);
									  }
						 }
			sost_trevog=0;				// �������� ��������� �������
		wr_strbuf("System ARMED\r",1);
		return 2;			//	
		} 
if (uncode_com("DISARM")) 				//�������� ��������
		{
		EE_WRITE(system_status,0);led_inform=0;siren_on(0);	//�������� ����������- ������
		set_out_ohran(1);		//���� ����� 3-����� ��������� �� ���������� 
		sost_trevog=0;				// �������� ��������� �������
		sound_opoveshenie(0);
		CNT_time_auto_set=60*(unsigned int)EE_READ(time_auto_set);
		wr_strbuf("System DISARMED\r",1);
		return 2;			//	
		} 
if (uncode_com("STATUS")) 				//�������� ��������
		{
		send_sms_status(n_abon);
		return 1;			//	
		} 
if (uncode_com("PROG")) 				//�������� ��������
		{
		dist_knop=1;				// ������ �������������� �������
		wr_strbuf("Programming activated.\r",1);
		return 2;			//	
		} 
if (uncode_com("SOUT1")) 				//�������� ��������
		{
		set_vihod(1,1);
		if (!EE_READ(konf_VIHOD+1-1))			// ���� ��������������� ��� 2 ��� �� ����� 2 ��� ���������
			{	delay_s(2);set_vihod(1,0);}
		wr_strbuf("OUT 1 activated.\r",1);
		return 2;			//	
		} 
if (uncode_com("SOUT2")) 				//�������� ��������
		{
		set_vihod(2,1);
		if (!EE_READ(konf_VIHOD+2-1))			// ���� ��������������� ��� 2 ��� �� ����� 2 ��� ���������
			{	delay_s(2);set_vihod(2,0);}
		wr_strbuf("OUT 2 activated.\r",1);
		return 2;			//	
		} 

end_err:
return 0;
}
//--------------------------
void check_sms_command(void)
{
unsigned char ukaz,n,i,n_abon,temp;
unsigned int work_int;
	delay_ms(500);
	putst("AT+CMGF=1\r");
	delay_ms(500);
for(n=0;n<20;n++)
{
	delay_ms(500);
	put_command("AT+CMGL=\"ALL\"\r");			// ��������� ���� �� SMS
	temp=read_lexem(1,2);		// ��������� 1 �������
	if (uncode_com("OK")) { delay_ms(1000);break;}		//���� SMS ������� ��� - ����
	if (uncode_com("+CMGL:")) 
		{ 
		copy_lexem(work_buf,buf_rx_command);
		temp=load_lexem(2);		// ��������� 1 ������� (����� SMS)
		copy_lexem(work_buf1,buf_rx_command);
		copy_lexem(buf_rx_command,work_buf);
		set_ukaz_zap(&ukaz,1);	//�������� ��������� �� -1� ������� 
		work_int=calk_parametr(ukaz); //��������� ����� ������
		fl_ring_yes=1;					//������������� �������� RING
		set_ukaz_zap(&ukaz,2);	//�������� ��������� �� -2� �������
		ukaz+=2;
		for(i=0;i<15;i++)
		{buffer_clip[i]=buf_rx_command[ukaz++];}	//��������� ����� ������ ���������� ���
		copy_lexem(buf_rx_command,work_buf1);
		n_abon=check_numb();
		while(1)									//��������� ��������� ��������
			{
				if(!getch_us(SW_uart)){delay_ms(500);if(!getch_us(SW_uart))break;}
			}
		if(n_abon!=0xFF)
				{
					run_smscommand(n_abon);
				}
		delay_s(1);putst("AT+CMGD=");putch_long(work_int);putch('\r');delay_s(1);	// ������� ������������ SMS	
	delay_ms(500);
		}	
	else break;	//���� ���������. ��� �������- ����
 }	
fl_ring_yes=0;
}

//--------------
 void roll_left_buf(char *buf,unsigned char n_roll)		// ����� � ���� �� n ����
{
unsigned char counter=0;
 while (1)
 {
	*buf=*(buf+n_roll);
	if (!(*buf)) break;
	buf++;
	if((++counter)>127) {*buf=0;break;}			// ������ �� �����
 }

}	
//-----------------------
void run_smscommand(unsigned char n_abon)
{
unsigned char ukaz,cnt_command_answer,cnt_command,temp,fl_answer;
fl_answer=0;
if (!uncode_com("##")) 	{return;} 		// ����  ������� �������� ���������� SMS- 
										// ���� ��� ������� ���������� SMS- �������
roll_left_buf(buf_rx_command,2);		// ������ ##
ukaz=find_N_char(buf_rx_command,';',1,MAX_BUF_RX_COM);
if(ukaz==0xFF) return;				//���� ������� ��� �������� ����� �� �����
wr_strbuf("\rExecuted commands:\r",0);
cnt_command_answer=cnt_command=0;
copy_lexem(buf_sms_command,buf_rx_command);		//��������� ������ �������
while (1)					// ���� 
	{
	ukaz=find_N_char(buf_sms_command,';',1,MAX_BUF_RX_COM);
	if(ukaz==0xFF) break;				//��� ��� ������ �������- ����
	buf_sms_command[ukaz]=0;
	copy_lexem(buf_rx_command,buf_sms_command);	//��������� ����� � ������� ������
	temp=detect_smscommand(n_abon);

	if(temp){cnt_command++;}		//���� �������� ���������� �� ��������� ���	
	if(temp==2)				// ���� �������� ������������� � ������� ������ � ��������, ��
		{
		wr_strbuf_buf(buf_sms_command,1);	// �������� ���������� ��������	
		wr_strbuf("; -OK\r",1);
		cnt_command_answer++;
		buf_sms_command[ukaz]=';';		//������� ������� 
		}
	if(temp==3)				// ���� �������� ������������� � ������� ������ � ��������, �� ������ ���������� ��
		{
		wr_strbuf_buf(buf_sms_command,1);	// �������� ���������� ��������	
		wr_strbuf("; -WP\r",1);				// �����������
		cnt_command_answer++;
		buf_sms_command[ukaz]=';';		//������� ������� 
		}
	  roll_left_buf(buf_sms_command,ukaz+1);	//������� ������������ ��������		

	}
//�������� ��� �� ���������� ������� 
if(!cnt_command){ wr_strbuf("\rNo valid commands!",1);send_long_SMS (STR_Buf,n_abon);return;}
if(!cnt_command_answer) {return;}// ���� ��� �� ����� ������� ��������� ������ �� �����
send_long_SMS (STR_Buf,n_abon);
}
