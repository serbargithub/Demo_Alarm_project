#include <pic18.h>
#include "name_port.h"
#include "my_delay.h"
#include "name_konst.h"
#include "usart.h" 

extern unsigned char cnt_clk,random;
extern unsigned char flag_param;
extern unsigned int cnt_secund;

#define MAX_BUF_MS_I2C	128
#define ADR_Rash 	0xE0

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



unsigned char buf_ms_I2C[MAX_BUF_MS_I2C];



void I2C_my_stop(void);
unsigned char read_answer(char ukaz,char adr_plat);
//+++++++++++++++++++++++++++++++++++
void initI2C_my(void)
{
tris_sda|=mask_scl_in;
tris_sda|=mask_sda_in;
}
//------------------
void I2C_my_start(void)
{
tris_sda&=mask_scl_out;
tris_sda&=mask_sda_out;
ms_sda_out=1;
ms_scl_out=1;
delay_mks(40);
ms_sda_out=0;
delay_mks(40);
ms_scl_out=0;
delay_mks(40);
}
//-------------------------
void I2C_my_stop(void)
{
tris_sda&=mask_scl_out;
tris_sda&=mask_sda_out;
ms_sda_out=0;
delay_mks(40);
ms_scl_out=1;
delay_mks(40);
ms_sda_out=1;
delay_mks(40);
}

//-------------------------
char I2C_my_putch(char byte)			//1- ��� ������ 0- ���
{
char cnt_bit,ret_ack;
tris_sda&=mask_scl_out;
tris_sda&=mask_sda_out;
for(cnt_bit=0;cnt_bit<8;cnt_bit++)
	{
	if(byte&0x80) {ms_sda_out=1;}		//���������� ��������� ���
		else  {ms_sda_out=0;}
	byte<<=1;
	delay_mks(20);
	ms_scl_out=1;						//�����
	delay_mks(20);
	ms_scl_out=0;
	}
tris_sda|=mask_sda_in;
 delay_mks(20);
 ms_scl_out=1;
ret_ack=0;
for(cnt_bit=0;cnt_bit<200;cnt_bit++)	// �������� ���
		{
		 delay_mks(160);
		if(!ms_sda_in){ret_ack=1;break;}
		}
 ms_scl_out=0;
 return ret_ack;
}

//-------------------------
char I2C_my_getch(char set_ACK)	//���� �������� 0 �� �� ��������� ACK 1-���������			
{
char cnt_bit,byte;
ms_scl_out=1;
tris_sda|=mask_sda_in;
tris_sda&=mask_scl_out;
for(cnt_bit=0;cnt_bit<8;cnt_bit++)
	{
	byte<<=1;
	byte&=0xFE;			//�������� ������� ���
	delay_mks(20);
	ms_scl_out=1;						//�����
	delay_mks(20);
	if(ms_sda_in){byte|=0x01;}			//���� ������� 1
	ms_scl_out=0;						// ��������� �����
	}
	if(set_ACK)
			{
			tris_sda&=mask_sda_out;
			ms_sda_out=0;				//������ ACK		
			}
	delay_mks(20);
	ms_scl_out=1;						//�����
	delay_mks(20);
	ms_scl_out=0;						// ��������� �����
	delay_mks(20);
	ms_sda_out=1;				//��������� ACK
return byte;

}			
//--------------------------------------
unsigned char transl_I2C(char ukaz,char adr_plat)
{
char i;
I2C_my_stop();
I2C_my_start();
if (!I2C_my_putch(adr_plat)) {I2C_my_stop();return 0;}
ukaz=buf_ms_I2C[0];
I2C_my_putch(ukaz+1);
for (i=1;i<=ukaz;i++)
	{
	I2C_my_putch(buf_ms_I2C[i]);// �������� ��������
	if(!buf_ms_I2C[i])break;			//���� 0 �� �����
	}
I2C_my_stop();
return 1;
}
//---------------------
unsigned char wait_scl_ms(int wait)
{
	while (wait)
	{
	cnt_clk=32;			//1ms
	while (cnt_clk){CLRWDT();}
	if (ms_scl_in) return 1;
	wait--;
	}
return 0;
}
//-------------------------------------
unsigned char read_answer(char ukaz,char adr_plat)			// ������ ������ �� ������ 
{
char i,ack;
I2C_my_stop();
I2C_my_start();
I2C_my_putch(adr_plat+1);
tris_sda|=mask_scl_in;
tris_sda|=mask_sda_in;
delay_ms(1);
if(!wait_scl_ms(3000)){I2C_my_stop();return 0;}// 3 cek ������ �� ������
buf_ms_I2C[0]=ukaz=I2C_my_getch(1);
if (ukaz>MAX_BUF_MS_I2C){I2C_my_stop();return 0;}//������ �� ������
for (i=1;i<=ukaz;i++)
	{
	tris_sda|=mask_scl_in;
	tris_sda|=mask_sda_in;
	if(!wait_scl_ms(500)){I2C_my_stop();return 0;}//������ �� ������
	ack=1;
	if(i==ukaz)ack=0;	//��������� ���� ��� ack
	buf_ms_I2C[i]=I2C_my_getch(ack);// ������� ����
	}
I2C_my_stop();
return 1;		//�����
}
//---------------------

unsigned char connect_I2C_master (char ukaz,char adr_plat)		// ���������� �� ������ ����
{
initI2C_my();
 if (!transl_I2C(ukaz,adr_plat))	{initI2C_my();return 0;}			//���� ������ �������
		delay_ms(500);	//�������� ��� ��������� ������
 if (!read_answer(ukaz,adr_plat)){initI2C_my();return 0;}	//���� ������ �������

initI2C_my();		// ��� �� ����
return 1;
}


//----------------------
void putst_msI2C_buf(const char *str,char fl_ukaz)		//������� ������ ���� �������� 0 � ������, 1 �������� 
{
	char i;
	if(fl_ukaz) {i=buf_ms_I2C[0];}
		else i=1;
	while((*str)!=0)
	{
		 buf_ms_I2C[i]=*str;
			if ((++i)>=(MAX_BUF_MS_I2C-2)){i--;break;}
		str++;
	}
	buf_ms_I2C[i]=0;
	buf_ms_I2C[0]=i;		// ������ ���� ��������� �� �����
}
//--------------------------------
void putch_msI2C_buf( char byte,char fl_ukaz)		//������� ���� ���� �������� 0 � ������, 1 �������� 
{
	char i;
	if(fl_ukaz) i=buf_ms_I2C[0];
		else i=1;
	buf_ms_I2C[i]=byte;
	if ((++i)>=(MAX_BUF_MS_I2C-2)){i--;}
	buf_ms_I2C[i]=0;
	buf_ms_I2C[0]=i;		// ������ ���� ��������� �� �����
}


//-----------------------------------

unsigned long calck_I2Cpar_ms(char ukaz)		// ��������� �������� �� ���������
{
unsigned long mnog,work_long;
work_long=0;
mnog=1;						// ��������� �������
while ((buf_ms_I2C[ukaz]!='=')&&(buf_ms_I2C[ukaz]!=0x2C))
	{
	work_long+=(buf_ms_I2C[ukaz]-'0')*mnog;	
	mnog*=10;
	if (!(ukaz--)) return 0;		// ������ �� ������
	}
return work_long;
}
//-----------------------------------------

char find_charI2C_ms(char simvol,char n_cnt) //����� N ������� � �������
{
char ukaz;
ukaz=1;
	if (buf_ms_I2C[ukaz]==simvol){ --n_cnt;}
	while(n_cnt)
	{
	ukaz++;	
	if((ukaz>MAX_BUF_MS_I2C)||(!buf_ms_I2C[ukaz])) {return 0xFF;}
	if (buf_ms_I2C[ukaz]==simvol){ --n_cnt;continue;}
	}
return ukaz;
}

//------------------------------------------
char read_dat_rash(void)//������ �� ����������� �������� � �������� ���
{
char i,ukaz;
	putst_msI2C_buf("READ_ZN\r",0);

	if(!connect_I2C_master(MAX_BUF_MS_I2C,ADR_Rash)) {return 1;}	//������������� �� �����������
	for (i=5;i<=12;i++)
	{
	ukaz=find_charI2C_ms(',',i-4);
	if(ukaz==0xFF) return 1;	// ���� ����� �� � ������� 
	buf_dat[i].sost=buf_ms_I2C[ukaz-1]-'0';
	buf_dat[i].triger&=0x0F;
	buf_dat[i].triger|=((buf_ms_I2C[ukaz-2]-'0')<<4);
	}
	return 0;
}
//------------------------------------------
char konf_dat_rash(void) // ������ � ����������� ������� ��� � ������������ ���
{
char i;
	putst_msI2C_buf("KONF_ZN=",0);
	for (i=5;i<=12;i++)
	{
	putch_msI2C_buf((0x0F&buf_dat[i].triger)+'0',1);
	putch_msI2C_buf(buf_dat[i].konfig+'0',1);
	putch_msI2C_buf(',',1);
	}
	if(!connect_I2C_master(MAX_BUF_MS_I2C,ADR_Rash)) {return 1;}	//������������� �� �����������
	return 0;
}
//------------------------------------------
char save_trig_rash(void) // ������ � ����������� �������� ���
{

char i;
	putst_msI2C_buf("SVTRZN=",0);
	for (i=5;i<=12;i++)
	{
	putch_msI2C_buf(((buf_dat[i].triger&0xF0)>>4)+'0',1);
	putch_msI2C_buf(',',1);
	}
	if(!connect_I2C_master(MAX_BUF_MS_I2C,ADR_Rash)) {return 1;}	//������������� �� �����������
	return 0;
}
