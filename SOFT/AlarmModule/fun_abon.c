#include <pic18.h> 
#include "my_spi_SST26.h"
#include "usart.h" 
#include "my_delay.h"

extern const char st_adr_abon;
//extern  char Bufer_SPI[]; 
extern  unsigned char number_str[];
extern unsigned char buffer_clip[16];
//-------------------------
void snd_abon(char num)
{
	long adr;
	char p,rd;
//	num--;
	adr=num*16+st_adr_abon;
	Frame_Buf_Read(adr,16);
	if((Bufer_SPI[2]!='+')&&(Bufer_SPI[2]!='8')&&(Bufer_SPI[2]!='0')){putch('+');}		//���� ����� ��� + �������� +
	for (p=2;p<16;p++) 
	{
	rd=Bufer_SPI[p];
	if (rd>'9') break;
	if (rd=='"') break;
	if (rd==0x00) break;
	putch(rd);
	}
}
//----------------------
void call_abon(char num)			// ������ �� ������ ������ 
{
	putst("atd");
	snd_abon(num);
	putst(";\r");

}
//-----------------------
char read_regim(char num)			// ��������� ����� ��������
{
	int adr;
//	num--;
	adr=num*16+st_adr_abon;
	Frame_Buf_Read(adr,16);
	return Bufer_SPI[1];
}
//-------------------------
char read_kateg(char num)			// ��������� ���������
{
	int adr;
//	num--;
	adr=num*16+st_adr_abon;
	Frame_Buf_Read(adr,16);
	return Bufer_SPI[0];
}
//-------------------------
void clear_num (char num)			// ������� �����
{
	char i;
 int adr;
//num--;
adr=num*16+st_adr_abon;
Bufer_SPI[0]='0';
Bufer_SPI[1]='0';
Bufer_SPI[2]=0x00;
for (i=3;i<16;i++)
	{
		Bufer_SPI[i]=0xFF;
	}
Frame_Buf_Write(adr,16);
}
//--------------------------
void write_num (char num)			// �������� �����
{
 int adr;
//num--;
adr=num*16+st_adr_abon;
Frame_Buf_Write(adr,16);
}
//-------------------------------------------------------
char check_numb(void)				// ��������� ����� ������ 0xff �� �����
{
char n,p,work,n_end,n_ok;
//int adr;
for  (n_end=1;n_end<16;n_end++)
		{if (buffer_clip[n_end]=='"') break;} //����� ����� ������
n_end--;
for (n=0;n<15;n++)
 {
	read_kateg(n);			// ��������� � ����� �����
	if((Bufer_SPI[2]==0xFF)||(Bufer_SPI[2]==0x00)) {continue;}		//���� ������ ���! ���������� 
	for  (p=2;p<16;p++)
		{if ((Bufer_SPI[p]=='"')||(Bufer_SPI[p]==0x00)) break;} //����� ����� ������
	n_ok=0;
	do
	{
	p--;
	work=Bufer_SPI[p];
	if	(n_ok>7) {return n;} // e��� ������� ������� 8 ���� �����
	if (buffer_clip[n_end-n_ok]!=work) break;	// ������������- ���������
	CLRWDT();
	n_ok++;
	}while(p>2);
 }
	return 0xff;				// �� ���� �� ������- �����
}
//----------------------
