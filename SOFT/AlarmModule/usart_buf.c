//�������  ��� ������ � USART � ������� ������( ������������� � ����� ������)

#include <pic18.h>
#include "usart.h" 
#include "name_konst.h"
#include "usart_buf.h"

extern unsigned char buf_rx_command[];

#define MAX_STR_Buf MAX_BIG_BUF_STR   //������ ������ ��� ������������ ������� ��������� SMS � �.�.
unsigned char STR_Buf[MAX_STR_Buf];
unsigned int point_buf;					//������� ������� � ������

//-----
void putst_buf(char *str)		//������� ������ �� ������
{
	check_err();
	while((*str)!=0)
	{
    	if (*str==0xFF) {break;}
		putch(*str);
		str++;
	}
}

//----------------------------
void wr_strbuf(register const char *str, char point_n)				//������ ������ � ����� ���� 0 ������� ��� ����������
{
if (!point_n) point_buf=0;
	while((*str)!=0)
	{
		if (!(point_buf<MAX_STR_Buf)){STR_Buf[point_buf-1]=0;return;}   //���� ����� ������ �� ��������� ������� ����� � �����
		STR_Buf[point_buf]=*str;
		point_buf++;
		str++;
	}
		STR_Buf[point_buf]=0;							//�������� ������� ����� ������
}
//----------------------------------------------------

void wr_strbuf_buf( char *str, char point_n)			//������ ������ � ����� �� ������ ���� 0 ������� ��� ����������
{
if (!point_n) point_buf=0;
	while((*str)!=0)
	{
		if (!(point_buf<MAX_STR_Buf)){STR_Buf[point_buf-1]=0;return;}   //���� ����� ������ �� ��������� ������� ����� � �����
		STR_Buf[point_buf]=*str;
		point_buf++;
		str++;
	}
		STR_Buf[point_buf]=0;							//�������� ������� ����� ������
}
//----------------------------------------------------
void wr_ptbuf(unsigned char c, char point_n)						//������ ����� � ����� ���� 0 ������� ��� ����������
{
if (!point_n) point_buf=0;
		if (!(point_buf<MAX_STR_Buf)){STR_Buf[point_buf-1]=0;return;}   //���� ����� ������ �� ��������� ������� ����� � �����
		STR_Buf[point_buf]=c;
		point_buf++;
		STR_Buf[point_buf]=0;							//�������� ������� ����� ������
}
//-----------
void wr_ptbuf_dec(unsigned char c, char point_n)
{
	unsigned char temp;
	if (!point_n) point_buf=0;
	temp=c;
	//hundreds
	if ((c/100)>0) wr_ptbuf((c/100)+'0',1);
	c-=(c/100)*100;

	//tens
	if (((temp/10)>0) || ((temp/100)>0)) wr_ptbuf((c/10)+'0',1);
	c-=(c/10)*10;

	//ones
	wr_ptbuf((c/1)+'0',1);
}

//-----------
void wr_ptbuf_long(unsigned long c, char point_n)
{
	static unsigned char st_razr;
	unsigned char temp;
	unsigned long  delit;
	if (!c) {wr_ptbuf('0',1);return;}
	st_razr=0;
	for (delit=1000000000L;delit>=1;delit/=10)
		{	
			if(((c/delit)>0)||st_razr) 
					{
					temp=(c/delit)+'0';
					st_razr=1;
					wr_ptbuf(temp,1);
					}
			c-=(c/delit)*delit;
			CLRWDT();
		}
}
//-------------------------------------------------
unsigned char find_N_char(unsigned char *massive,unsigned char simvol,unsigned char n_cnt,unsigned int MAX_STR) //����� N ������� � �������
{
unsigned int ukaz;
ukaz=0;
	if (*massive==simvol){ --n_cnt;}
	while(n_cnt)
	{
	ukaz++;	
	massive++;
	if(simvol)	{if((ukaz>=MAX_STR)||(!(*massive))) {return 0xFF;}}
		else 	{if(ukaz>=MAX_STR) {return 0xFF;}}
		
	if (*massive==simvol){ --n_cnt;continue;}
	}
return ukaz;
}
//--------------------------------------------
void set_ukaz_zap(char*ukz,char zap)		// ��������� ��������� �� ������� �� �����
{
//char i;
*ukz=0;

do
	{
	++*ukz;
	if(*ukz>=MAX_BUF_RX_COM){*ukz=0;break;}
	if(buf_rx_command[*ukz]==0x2C) {zap--;}		//�������
	}while (zap);
}
//----------------------------------------
unsigned char compare_buf(char *bufer1,char *bufer2,char end_symb,char max_buf2)	// �������� ����� 2 � ������� 1 �� 0 ��� end_symb
{
char i;
for (i=0;i<max_buf2;i++)
	{
		if ((!*bufer2)&&(!*bufer1)) return 1;   // ����� ���� - ����� 
		if  (*bufer2==end_symb)		return 1;	// ������� �����
		if (*bufer2!=*bufer1) return 0;
		bufer1++;
		bufer2++;	
	}
return 0;
}

//---------------------------
unsigned char compare_str_buf(register const char *str,unsigned char *bufer)		//�������� ������( ��������) c ���������� ������ �� ����� ������
{
	while((*str)!=0)
	{
    if ((*str)!=*bufer) return 0;	// �� ������� ��������
		bufer++;
		str++;
	}
return 1;		// ������ ����������
}
//---------------------------
char copy_buf(char *bufer1,char *bufer2,char end_symb,char max_buf2)	// ����������� ����� 2 � ����� 1 �� 0 ��� end_symb ������ ���������� 0
			//��������� ��������� �� ��������� ����
{
char i;
for (i=0;i<max_buf2;i++)
	{
		*bufer1=*bufer2;
		if (!*bufer2) {return i;}   // ����� ���� - ����� 
		if  (*bufer2==end_symb)	{return i;}	// ������� �����
		bufer1++;
		bufer2++;	
	}
return 0;
}

//---------------------------------
unsigned long calk_parametr_buf(unsigned char  *bufer,unsigned char end_symb)		//���������� ��������� �� ������ �������� ����������� �� ����������� ��������� � ���� �� ����� �� end_symb
{
unsigned char *bufer_st,i;
unsigned long mnog,work_long;
bufer_st=bufer-1;		// ��������� ��������
for(i=0;i<11;i++)	// ������������ �������� ����������� ����
	{
	if (*bufer==end_symb) break;			// ����� end_symb
	if (*bufer==0x00) break;			// ����� end_symb
	bufer++;
	}
	if (i==11) {return 0;}		// ������ �� ������
	bufer--;  		//���������
work_long=0;
mnog=1;						// ��������� �������
while (bufer!=bufer_st)
	{
	work_long+=(*bufer-'0')*mnog;	
	mnog*=10;
	bufer--;
	}
return work_long;
}