#include <pic18.h>  
#include "my_delay.h"
#include "usart.h"

extern unsigned char SW_uart;		//�������������� �������� COM �����
//-------------------------------------------------------------------
//��������� ������ ����� �� EEPROM
char EE_READ (unsigned int addr)
{
char c;
while (WR){};
c=EEPROM_READ(addr);
return c;
}
//-------------------------------------------------------------------
//��������� ������ char � EEPROM
void EE_WRITE (unsigned int addr,char var)
{
while (WR){};
EEPROM_WRITE(addr,var);
}

//-------------------------------------------------------------------
//��������� ������ int �� EEPROM
int EE_READ_Int (unsigned int addr)
{
unsigned int res;
while (WR){};
*(char*)&res=EE_READ(addr);
while (WR){};
*(((char*)&res)+1)=EE_READ(addr+1);

return res;
}
//-------------------------------------------------------------------
//��������� ������ int � EEPROM
void EE_WRITE_Int (unsigned int addr,int var)
{
while (WR){};
EE_WRITE(addr  ,*((char*)&var  ));
while (WR){};
EE_WRITE(addr+1,*(((char*)&var)+1));
}
//-------------------------------------------------------------------

//��������� ������ long � EEPROM
void EE_WRITE_Long (unsigned int addr,long var)
{
char j;

for (j=0;j<4;j++){ while (WR){};EE_WRITE(addr+j,*(((char*)&var)+j));}
}
//-------------------------------------------------------------------
//��������� ������ long �� EEPROM
long EE_READ_Long (unsigned int addr)
{
long res;
char j;

for (j=0;j<4;j++)
   {while (WR){};*(((char*)&res)+j)=EE_READ(addr+j);}
return res;
}
//--------------------------------------------------------------------------
//��������� ������ string � EEPROM
void EE_WRITE_String (unsigned int addr,char *str)
{
unsigned char j=0;
	while((*str)!=0)
	{
	EE_WRITE(addr,*str);
	addr++;str++;j++;
	if(j>=63) break;	//������ �� ������, ������������ ������ ������ 64 �����
	}
	EE_WRITE(addr,0);	
}

//-------------------------------------------------------------------
//��������� ������ string �� EEPROM
void  EE_READ_String (unsigned int addr,char *str)
{
unsigned char j=0;
	while(1)
	{	
		if(!(*(str+j)=EE_READ(addr))) break; // ���� ��������� 0 �� ����� ������
		addr++;j++;
		if(j>=63){*(str+j)=0; break;}	//������ �� ������, ������������ ������ ������ 64 �����
	}
}

unsigned char write_eep_frame(unsigned int set_adr_eep,unsigned int cnt_byte_eep) //������  ������
{

unsigned char work,fl_read_error;
clear_buf_RX(SW_uart);

      while(cnt_byte_eep)
	{
      work=getch_n(1,SW_uart,&fl_read_error);   //������ ����  
	  if(fl_read_error) {putst("\rError Time\r"); return 0;}
      cnt_byte_eep--;
	  CLRWDT();
	while (WR){};
      EEPROM_WRITE(set_adr_eep,work);
	  delay_ms(10);
      putch(work);							// ACK
      set_adr_eep++;
     }
return 1;
}

//------------------
unsigned char read_eep_frame(unsigned int set_adr_eep,unsigned int cnt_byte_eep)
{
unsigned char work,fl_read_error;
clear_buf_RX(SW_uart);

while(cnt_byte_eep)
   {
while (WR){};
   work=EEPROM_READ(set_adr_eep);
	if(getch_n(1,SW_uart,&fl_read_error)!='+') {return 0;}				// ���� �� ������ ����� error
	putch(work);					//���
	set_adr_eep++;
	cnt_byte_eep--;
	CLRWDT();
   }
return 1;
}

