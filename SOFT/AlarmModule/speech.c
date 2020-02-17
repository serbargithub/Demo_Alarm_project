#include <pic18.h>
#include "name_port.h"
#include "name_konst.h"
#include "name_adres.h"
#include "signal.h"
#include "my_spi_SST26.h"
#include "my_delay.h"
#include "usart.h" 
#include "voice_map.h"
#include "eeprom.h"

extern unsigned long adr_counter;
extern unsigned char Sync_Voice,fl_ring,opros_dat_enable; 



void voice_speech(unsigned int num,unsigned char CH)			//выбор сообщени€ результат adr_st=начало adr_end конец
{
unsigned long adr_st,adr_end;
char *temp_uk;
//unsigned char code;
unsigned char pwmout_l,pwmout_h,counter_dov;           // temporary variable for PWM
if(EE_READ(menu_language)==1) {num+=UKR_table;}		// если выбран украинский €зык то добавить смещение 
start_FL_read((--num)*8+ADD_adr);			//выставить адрес+смещение
temp_uk=&adr_st;
*temp_uk=count_FL_read();
*(temp_uk+1)=count_FL_read();
*(temp_uk+2)=count_FL_read();	
*(temp_uk+3)=count_FL_read();		//прочитать старт адрес
temp_uk=&adr_end;
*temp_uk=count_FL_read();
*(temp_uk+1)=count_FL_read();
*(temp_uk+2)=count_FL_read();
*(temp_uk+3)=count_FL_read();		// прочитать стоп адрес
stop_FLASH();
adr_st+=ADD_adr;
adr_end+=ADD_adr;

start_FL_read(adr_st);
counter_dov=0;
Sync_Voice=0;
do
{
pwmout_l<<=2;
if (!counter_dov){pwmout_l=count_FL_read();counter_dov=4;}
counter_dov--;
pwmout_h=count_FL_read();
while(!Sync_Voice)CLRWDT();    // Wait for 8kHz to output
//control1=0;
Sync_Voice = 0;

if (CH==PWM1)
	{
	CCPR1L = pwmout_h-0x20; // Write 7 bits
	CCP1CON &= 0xcf;             // to CCPR1L
	if(pwmout_l&0x80)            // Write 2 bits
		{DC1B1 = 1;}       // to CCP1CON
	if(pwmout_l&0x40)            // to get 9-bit
		{DC1B0 = 1;}       // PWM
	}
if (CH==PWM2)
	{
	CCPR2L = pwmout_h-0x20; // Write 7 bits	
	}

CLRWDT();
if (dso_vg) {break;}
}while (adr_counter<adr_end);							//глобальна€ переменна€ adr_counter
stop_FLASH();
delay_ms_dso(100);
}
//---------------------
void voice_rec(char n_voice)
{
unsigned int temp_int;
unsigned char temp1,temp2;
unsigned long rec_end,rec_start,temp_long;
static unsigned char save_enebl;
if ((n_voice<1)||(n_voice>16)){voice_speech(oshibka,PWM1); return;}
voice_speech(govorite,PWM1);
rec_start=REC_ADR+(n_voice-1)*REC_frame;
rec_end=REC_ADR+(n_voice)*REC_frame-1;
Open_SST();
for (temp_long=rec_start;temp_long<rec_end;temp_long+=0x1000)		//стереть предыдущее сообщение 
	{
		Sector_Erase(temp_long);
	}
save_enebl=opros_dat_enable;
opros_dat_enable=0;
ADCON0=0b00011101;		//подключить ј÷ѕ AN7
start_FL_write(rec_start,0,0);

do
{
	while(!Sync_Voice){CLRWDT();}    // Wait for 8kHz to output
	Sync_Voice = 0;
	GODONE=1;
	while(GODONE)CLRWDT(); 
	temp_int=(ADRESH-93)<<1;
	if(ADRESL&0x80) temp_int++;
	temp_int+=128;
	temp1=(unsigned char)(temp_int&0x00ff);
	while(!Sync_Voice){CLRWDT();}    // Wait for 8kHz to output
	Sync_Voice = 0;
	GODONE=1;
	while(GODONE)CLRWDT(); 
	temp_int=ADRESH;
	temp_int=(ADRESH-93)<<1;
	if(ADRESL&0x80) temp_int++;
	temp_int+=128;
	temp2=(unsigned char)(temp_int&0x00ff);
	count_FL_write(temp1,temp2);
	if(dso_vg) {break;}
CLRWDT();
}while(adr_counter<rec_end);			//глобальна€ переменна€ adr_counter
 end_FL_write();
while(dso_vg)CLRWDT();
voice_speech(soob_zapisano,PWM1);
opros_dat_enable=save_enebl;
}
//---------------------------------
void voice_play(char n_voice,char CH)
{
unsigned long play_start,play_end;
unsigned char pwmout;           // temporary variable for PWM
if ((n_voice<1)||(n_voice>16)){voice_speech(oshibka,PWM1); return;}
play_end=REC_ADR+(n_voice)*REC_frame;
play_start=REC_ADR+(n_voice-1)*REC_frame;
start_FL_read(play_start);
do
	{
pwmout=count_FL_read();
while(!Sync_Voice)CLRWDT();    // Wait for 8kHz to output
Sync_Voice = 0;
if (CH==PWM1)
	{
	CCPR1L =pwmout; // Write 7 bits
	CCP1CON &= 0xcf;             // to CCPR1L
	}
  else
	{
	CCPR2L = pwmout; // Write 7 bits
	CCP2CON &= 0xcf;             // to CCPR1L
	}
	if(dso_vg) break;
CLRWDT();
	}while (adr_counter<play_end);							//глобальна€ переменна€ adr_counter
stop_FLASH();
}
