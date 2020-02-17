#include <pic18.h> 
#include "name_port.h"
#include "my_delay.h"
#include "usart.h" 
#include "usart_buf.h" 
#include "name_konst.h"
#include "name_adres.h"
#include "signal.h" 
#include "eeprom.h"
#include "fun_abon.h"
#include "terminal.h"
#include "my_spi.h"
#include "lexem_rutine.h"

extern signed  char temperatura_dallas[2],temperatura_save[2];
extern const char st_adr_abon;

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
extern unsigned char work_string[];

extern unsigned char glob_knopka_system,SW_uart;
extern unsigned int cnt_secund;
extern unsigned char fl_bort_napr;



void print_SMS_alarm(char num_zone,char regim);
void load_date_time(void);
void bild_sms_balans(void);
void send_long_SMS(unsigned char *str,unsigned char n_abon);
//-----------------------------

char snd_alarm_sms(char n_abon,char regim)				//тревожная отправка SMS результат 1- отправлено, 0 неудача
{
 char i,n,end_zone;


	putst("AT+CMGF=1\r");
	delay_ms(500);
	load_date_time();			// прочитать дату- время

	putst("AT+CMGS=\"");
  	snd_abon(n_abon);putch('"');putch(0x0d);	// сформировать номер
	delay_ms(255);
	putst_buf(&STR_Buf[0]);				// напечатать дату-время
									//cформировать  SMS
	n=1;
	end_zone=last_zon;
	for (i=1;i<=end_zone;i++)
		{		
		if(!buf_dat[i].alarm)continue;					// если зона не в тревоге дальше
		putst("\r");putchdec(n++);putch('-');
		print_SMS_alarm(i,regim);						// напечатать тревогу зоны в соответствии с конфигом
		}
	
		putch(0x1A);			//cntrol Z
		check_OK(10);			//10 секунд ждать OK
		delay_s(1); 
		return 1;
}

//-------------------------------------------

void print_SMS_alarm(char num_zone,char regim)
{
unsigned long adr_sms;
char konf;
if(num_zone<16){konf=EE_READ(konf_SMS+num_zone);}
	else {konf=0;}
if (konf<40)
	{
	if(regim<4){putst("Trevoga datchik: ");}
	if(regim==4) {putst("Vostanovlen datchik: ");}
	if(regim==5) {putst("Avarija datchik: ");}
	}
	else
	{
	if(regim==4) {putst("PODKL: ");}
	}
 switch(konf)
 {
	case 0:
			putchdec(num_zone);
		break;
	case 1:
			putst("dvigenija");			// 
		break;
	case 2:
			putst("proniknoveniya");			// 
		break;
	case 3:
			putst("napadenie");			// 
		break;
	case 4:
			putst("voda");			// 
		break;
	case 5:
			putst("pozhara");		// 
		break;
	case 6:
			putst("dima");			// 
		break;
	case 7:
			putst("gaza");			// 
		break;
	case 8:
			putst("sirena");			// аккумулятор
		break;
	case 9:
			putst("klaviatura");			//удара
		break;
	case 10:
			putst("dveri");			// 
		break;
	case 11:
			putst("okna");			// 
		break;
	case 12:
			putst("narugnij perimetr");			//
		break;
	case 13:
			putst("fasad");			// 
		break;
	case 14:
			putst("til");			// 
		break;
	case 15:
			putst("Etazh 1");		// 
		break;
	case 16:
			putst("Etazh 2");		// 
		break;
	case 17:
			putst("zal");			//зал
		break;
	case 18:
			putst("spalnya");			//спальня
		break;
	case 19:
			putst("prihogaya");	//прихожая
		break;
	case 20:
			putst("kuhnya");			//кухня
		break;
	case 21:
			putst("sklad");			 //тыл
		break;
	case 40:
			putst("Srabotal datchik ohrani");		//
		break;
	case 41:
			putst("Srabotal datchik pozhara");		// 
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
		if(konf>50){read_parametr(adr_sms);}  	//впечатать свою смс	
}

//-------------------------------

void snd_SMS_otchet(char type_oth)		// отправка SMS о постановке снятии
{

	putst("AT+CMGF=1\r");
	delay_ms(500);
	load_date_time();			// прочитать дату- время
	putst("AT+CMGS=\"");
  	snd_abon(0);putch('"');putch(0x0d);	// сформировать номер  главного абонента
	delay_ms(255);
	putst_buf(STR_Buf);				// напечатать дату-время
	if(type_oth){putst("\rSistema na ohrane.");}
			else{putst("\rSistema snjata s ohrani.");}
	putch(0x1A);			//cntrol Z
	check_OK(10);			//10 секунд ждать OK
	delay_s(1); 
}


//-------------------------------
void load_date_time(void)
{
unsigned char fl_read_error,i;
if(!EE_READ(on_Date_Time)){wr_strbuf("\r",0);return;}
wr_strbuf("Date: ",0);
if (put_command("AT+CCLK?\r")) return;
getch_n(10,SW_uart,&fl_read_error);
for(i=0;i<17;i++) {wr_ptbuf(getch_n(1,SW_uart,&fl_read_error),1);}
wr_strbuf("\n",1);
delay_ms(250);
}

//----------------------------------
void load_date_time_PL_CSQ(void)
{
unsigned char fl_read_error,i,work;
if(!EE_READ(on_Date_Time)){wr_strbuf("\r",0);}
	else
	{
	wr_strbuf("Date: ",0);
	if (put_command("AT+CCLK?\r")) return;
	getch_n(10,SW_uart,&fl_read_error);
	for(i=0;i<17;i++) {wr_ptbuf(getch_n(1,SW_uart,&fl_read_error),1);}
	delay_ms(350);
	wr_strbuf("\r\n",1);
	}
if (put_command("AT+CSQ\r")) return;
getch_n(3,SW_uart,&fl_read_error);
for(i=0;i<7;i++) 
		{	work=getch_n(1,SW_uart,&fl_read_error);
			if (work=='\r') break;
			wr_ptbuf(work,1);
		}

if(EE_READ(Conf_TEMP_DAT))
	{
	if(temperatura_dallas[0]==0x7F){wr_strbuf("\rDat1 TMP: Error",1);}
	if (temperatura_dallas[0]>=0){wr_strbuf("\rDat1 TMP:+",1);wr_ptbuf_dec(temperatura_dallas[0],1);}
		else {wr_strbuf("\rDat1 TMP:-",1);wr_ptbuf_dec(0-temperatura_dallas[0],1);}
	}
if(EE_READ(Conf_TEMP_DAT+1))
	{
	if(temperatura_dallas[1]==0x7F){wr_strbuf("\rDat2 TMP: Error",1);}
	if (temperatura_dallas[1]>=0){wr_strbuf("\rDat2 TMP:+",1);wr_ptbuf_dec(temperatura_dallas[1],1);}
		else {wr_strbuf("\rDat2 TMP:-",1);wr_ptbuf_dec(0-temperatura_dallas[1],1);}
	}

delay_ms(250);
}
//------------------------------------

void snd_SMS_test(char regim)		// отправка SMS о постановке снятии
{
		if(glob_knopka_system){wr_strbuf("\rTest. Sistema na ohrane.\r",1);}
				else {wr_strbuf("\rTest. Sistema snjata s ohrani.\r",1);}
	load_date_time();			// прочитать дату- время
	if(regim==3){bild_sms_balans();}	// если sms+ баланс - впечатать баланс
	send_long_SMS(STR_Buf,0);		//отправить SMS абоненту 0

}

//----------------------------------
void snd_SMS_napr(void)
{
	putst("AT+CMGF=1\r");
	delay_ms(500);
	load_date_time();			// прочитать дату- время
	putst("AT+CMGS=\"");
  	snd_abon(0);putch('"');putch(0x0d);	// сформировать номер  главного абонента
	delay_ms(255);
	putst_buf(STR_Buf);				// напечатать дату-время
	if(fl_bort_napr) putst("\rBatarejnoe pitanie norma.");
		else putst("\rNizkoe batarejnoe pitanie.");
	putch(0x1A);			//cntrol Z
	check_OK(10);			//10 секунд ждать OK
	delay_s(1); 
}

//------------------------------------
void send_long_SMS(unsigned char *str,unsigned char n_abon)
{
unsigned char i,end_sms,part,work,ukaz_metka;
	delay_s(2);
	putst("ATH\r");
	delay_s(2);
	putst("AT+CMGF=1\r");
	delay_ms(500);
end_sms=0;
part=1;
while(1)
{
	putst("AT+CMGS=\"");
 	snd_abon(n_abon);putch('"');putch(0x0d);	// сформировать номер  главного абонента
	delay_ms(255);
	if(part>1){putst("PART: ");putchdec(part);putch('\r');}
	part++;
    ukaz_metka=149;
	for(i=0;i<149;i++)			//поиск конца строки
		{
		work=*(str+i);
		if(!work){ukaz_metka=i;end_sms=1; break;}
		if(work=='\n'){ukaz_metka=i;} 
		if(work=='\r'){ukaz_metka=i;} 
		}
	if((ukaz_metka<i)&&(ukaz_metka<80)) ukaz_metka=i;
	for(i=0;i<=ukaz_metka;i++)
		{
		if(*str==0){end_sms=1; break;}
		putch(*str);
		str++;
		}
	delay_s(1); 
	putch(0x1A);						//cntrol Z
	if(!check_OK(20)) break;			//10 секунд ждать OK
	delay_s(1); 
	if(end_sms) break;
}
}
//------------------------------------
void print_on_off(char on_off)
{
if(on_off)	{wr_strbuf("ON",1);}
		else {wr_strbuf("OFF",1);}
}
//---------------------------------
void send_sms_nastroiki(char n_abon)		// отправка SMS о постановке снятии
{
char i,work,end_zone;
	end_zone=last_zon;
	load_date_time_PL_CSQ();			// прочитать дату- время
	wr_ptbuf('\r',1);
	wr_strbuf("OHRANA: ",1);print_on_off(EE_READ(system_status));wr_ptbuf('\r',1);
	wr_strbuf("Balans:",1);for(i=0;i<7;i++){work=EE_READ(USSD_Balans+i);wr_ptbuf(work,1);if(work=='#') break;} wr_ptbuf('\r',1);
	wr_strbuf("Password:",1);print_on_off(EE_READ(Menu_pass));wr_ptbuf('\r',1);
	wr_strbuf("Lang:",1);wr_ptbuf_dec(EE_READ(menu_language),1);wr_ptbuf('\r',1);

	wr_strbuf("Per TST:",1);wr_ptbuf_dec(EE_READ(period_TEST),1);wr_ptbuf('\r',1);
	wr_strbuf("St TST:",1);wr_ptbuf_dec(EE_READ(start_TEST),1);	wr_ptbuf('\r',1);
	wr_strbuf("Rg TST:",1);wr_ptbuf_dec(EE_READ(regim_TEST),1);	wr_ptbuf('\r',1);
	wr_strbuf("Otch Post:",1);wr_ptbuf_dec(EE_READ(otchet_postan),1);wr_ptbuf('\r',1);
	wr_strbuf("Reg Post:",1);wr_ptbuf_dec(EE_READ(Conf_post_sn),1);wr_ptbuf('\r',1);
	wr_strbuf("Rg BAT:",1);wr_ptbuf_dec(EE_READ(regim_BORT_napr),1);wr_ptbuf('\r',1);

	wr_strbuf("Cnt Dozv:",1);wr_ptbuf_dec(EE_READ(lim_DOZVON),1);wr_ptbuf('\r',1);
	wr_strbuf("Cnt SMS:",1);wr_ptbuf_dec(EE_READ(lim_SMS),1);wr_ptbuf('\r',1);

	wr_strbuf("Tm VIH:",1);wr_ptbuf_dec(EE_READ(time_VIHOD),1);wr_ptbuf('\r',1);
	wr_strbuf("Tm VH:",1);wr_ptbuf_dec(EE_READ(time_VHOD),1);wr_ptbuf('\r',1);
	wr_strbuf("Tm ST:",1);wr_ptbuf_dec(EE_READ(time_STAND),1);wr_ptbuf('\r',1);
	wr_strbuf("Tm Srn:",1);wr_ptbuf_dec(EE_READ(time_SIREN),1);wr_ptbuf('\r',1);

	wr_strbuf("Ur ZONE:",1);for(i=0;i<=end_zone;i++){wr_ptbuf_dec(i,1);wr_ptbuf(':',1);wr_ptbuf_dec(EE_READ(regim_napr+i),1);wr_ptbuf(',',1);}wr_ptbuf('\r',1);
	wr_strbuf("Cf ZONE:",1);for(i=0;i<=end_zone;i++)
								{wr_ptbuf_dec(i,1);wr_ptbuf(':',1);work=EE_READ(konf_ZONE+i);
								if(work<0x80){wr_ptbuf_dec(work,1);}
										else {wr_strbuf("BL",1);}			//если зона блокирована
								wr_ptbuf(',',1);
								}
	wr_ptbuf('\r',1);
	wr_strbuf("Time AutoSet:",1);wr_ptbuf_dec(EE_READ(time_auto_set),1);wr_ptbuf('\r',1);

	wr_strbuf("Cf VIH:",1);for(i=1;i<=2;i++){wr_ptbuf_dec(i,1);wr_ptbuf(':',1);wr_ptbuf_dec(EE_READ(konf_VIHOD+i-1),1);wr_ptbuf(',',1);}wr_ptbuf('\r',1);
	wr_strbuf("link ZONE VIH:",1);for(i=1;i<=end_zone;i++){wr_ptbuf_dec(i,1);wr_ptbuf(':',1);wr_ptbuf_dec(EE_READ(link_Zone_Vih+i),1);wr_ptbuf(',',1);}	wr_ptbuf('\r',1);			

	wr_strbuf("Cf SMS:",1);for(i=1;i<=end_zone;i++){wr_ptbuf_dec(i,1);wr_ptbuf(':',1);wr_ptbuf_dec(EE_READ(konf_SMS+i),1);wr_ptbuf(',',1);}wr_ptbuf('\r',1);
	wr_strbuf("Cf VOICE:",1);for(i=1;i<=end_zone;i++){wr_ptbuf_dec(i,1);wr_ptbuf(':',1);wr_ptbuf_dec(EE_READ(konf_VOICE+i),1);wr_ptbuf(',',1);} wr_ptbuf('\r',1);

	wr_strbuf("TempON:",1);wr_ptbuf_dec(EE_READ(Conf_TEMP_DAT),1);wr_ptbuf('\r',1);
if(EE_READ(Conf_TEMP_DAT))
	{
	wr_strbuf("TempLOW:",1);wr_ptbuf_dec(EE_READ(TEMP_LOW),1);
	wr_ptbuf('\r',1);
	wr_strbuf("TempHIGH:",1);wr_ptbuf_dec(EE_READ(TEMP_HIGH),1);
	wr_ptbuf('\r',1);
	wr_strbuf("TempGis:",1);wr_ptbuf_dec(EE_READ(TEMP_GIST),1);
	}
	send_long_SMS(STR_Buf,n_abon);

}
//--------------
void bild_sms_balans(void)		// сборка SMS с балансом
{
unsigned int time_sec,getch_byte;
unsigned char i,temp,n;
	putst("AT+CUSD=1,\"");
	EE_READ_String(USSD_Balans,work_string);
	putst_buf(work_string);putst("\"\r");
	read_lexem(3,1);
			clear_buf_RX(SW_uart);
			time_sec=cnt_secund+10;		// 10 секунды ждать ответ
			if(time_sec<cnt_secund){cnt_secund=0;time_sec=10;}	// защита от ошибок
			for (n=0;n<=11;n++)				// отступить 11 символов
 				 {
					while(!read_buf_RX(SW_uart))
						{CLRWDT();
						if(time_sec<=cnt_secund) return;
						}
				 }
			time_sec=cnt_secund+2;		// 2 секунды ждать ответ
			if(time_sec<cnt_secund){cnt_secund=0;time_sec=2;}	// защита от ошибок
			while(time_sec>cnt_secund)
				{
					CLRWDT();
					getch_byte=read_buf_RX(SW_uart);
					if(getch_byte) {wr_ptbuf((unsigned char)getch_byte,1);}
				}
					

}
//------------------------
void send_sms_status(unsigned char n_abon)
{
unsigned char i,temp,end_zone,work;
	end_zone=last_zon;
	load_date_time_PL_CSQ();			// прочитать дату- время
	wr_ptbuf('\r',1);
	wr_strbuf("OHRANA: ",1);print_on_off(EE_READ(system_status));
	wr_ptbuf('\r',1);
	for(i=1;i<=end_zone;i++)
	{wr_strbuf("Zn ",1);temp=0;if(buf_dat[i].sost!=(buf_dat[i].triger&0x0F))temp=1;
	 wr_ptbuf_dec(i,1);wr_ptbuf(':',1);print_on_off(temp);wr_ptbuf('\r',1);}	
	 wr_strbuf("OUT 1:",1);print_on_off(check_vihod(1));
	 wr_ptbuf('\r',1);
	 wr_strbuf("OUT 2:",2);print_on_off(check_vihod(2));
	 wr_ptbuf('\r',1);

if(EE_READ(Conf_TEMP_DAT))
	{
	if(temperatura_dallas[0]==0x7F){wr_strbuf("\rDat1 TMP: Error",1);}
	if (temperatura_dallas[0]>=0){wr_strbuf("\rDat1 TMP:+",1);wr_ptbuf_dec(temperatura_dallas[0],1);}
		else {wr_strbuf("\rDat1 TMP:-",1);wr_ptbuf_dec(0-temperatura_dallas[0],1);}
     wr_ptbuf('\r',1);
	}
if(EE_READ(Conf_TEMP_DAT+1))
	{
	if(temperatura_dallas[1]==0x7F){wr_strbuf("\rDat2 TMP: Error",1);}
	if (temperatura_dallas[1]>=0){wr_strbuf("\rDat2 TMP:+",1);wr_ptbuf_dec(temperatura_dallas[1],1);}
		else {wr_strbuf("\rDat2 TMP:-",1);wr_ptbuf_dec(0-temperatura_dallas[1],1);}
    wr_ptbuf('\r',1);
	}
	send_long_SMS(STR_Buf,n_abon);
}
//-------------------------
void send_sms_balans(char n_abon)				// отправка смс с балансом
{
	delay_s(2);
	putst("ATH\r");
	delay_s(1);
	load_date_time();			// прочитать дату- время
	bild_sms_balans();
	send_long_SMS(STR_Buf,n_abon);
}
//-------------------------
void send_sms_abonenti(char n_abon)				// отправка смс с абонентами
{
	long adr;
unsigned char work,num,p,rd;
	delay_s(2);
	load_date_time_PL_CSQ();			// прочитать дату- время
	for(num=0;num<15;num++)
	{
	wr_ptbuf('\r',1);wr_ptbuf_dec(num+1,1);wr_strbuf(": ",1);
	work=read_kateg(num);
	if((Bufer_SPI[2]==0xFF)||(Bufer_SPI[2]==0x00)){wr_strbuf("Empty.",1);continue;} 
	wr_ptbuf(Bufer_SPI[0],1);wr_ptbuf(' ',1);wr_ptbuf(Bufer_SPI[1],1);wr_ptbuf(' ',1);
//	num--;
	adr=num*16+st_adr_abon;
	Frame_Buf_Read(adr,16);
	if((Bufer_SPI[2]!='+')&&(Bufer_SPI[2]!='8')){wr_ptbuf('+',1);}		//если номер без + добавить +
	for (p=2;p<16;p++) 
	 {
	 rd=Bufer_SPI[p];
	 if (rd>'9') break;
	 if (rd=='"') break;
	 if (rd==0x00) break;
 	 wr_ptbuf(rd,1);
	 }
	}
	send_long_SMS(STR_Buf,n_abon);
}
//-------------------
void snd_SMS_otchet_temperatura(unsigned  char n_abon)				// отправка смс с абонентами
{
unsigned char work,temp;
	delay_s(2);
	putst("ATH\r");
	delay_s(1);
	load_date_time();			// прочитать дату- время
	putst("AT+CMGF=1\r");
	delay_ms(500);
	putst("AT+CMGS=\"");
  	snd_abon(n_abon);putch('"');putch(0x0d);	// сформировать номер  главного абонента
	delay_ms(255);
	putst_buf(&STR_Buf[0]);				// напечатать дату

	if(temperatura_dallas[0]==0x7F){putst("\rDat1 Temp.: Error");}
		else {if (temperatura_dallas[0]>=0){ putst("Dat1 Temp.:+");putchdec(temperatura_dallas[0]);}
					else { putst("Dat1 Temp.:-");putchdec(0-temperatura_dallas[0]);}
			 }	
	putst("\r");
		temp=0;
		if(temperatura_dallas[0]>=EE_READ(TEMP_LOW)) {temp=1;}
		if(temperatura_dallas[0]>EE_READ(TEMP_HIGH)) {temp=2;}
		if(!EE_READ(Conf_TEMP_DAT)) {temp=0xFE;}
		switch(temp)
			{
				case 0: putst("Vnimanie! Ponigenie temperaturi.");break;
				case 1: putst("Temperatura norma.");break;
				case 2: putst("Vnimanie! Povishenie temperaturi.");break;
				case 0xFE: putst("Datchik temperaturi ne podklychen!.");break;
				default: putst("Avariya temperaturnogo datchika!");break;
			}

	if(temperatura_dallas[1]==0x7F){putst("\rDat2 Temp.: Error");}
		else {if (temperatura_dallas[1]>=0){ putst("Dat2 Temp.:+");putchdec(temperatura_dallas[1]);}
					else { putst("Dat2 Temp.:-");putchdec(0-temperatura_dallas[1]);}
			 }	
	putst("\r");
		temp=0;
		if(temperatura_dallas[1]>=EE_READ(TEMP_LOW+1)) {temp=1;}
		if(temperatura_dallas[1]>EE_READ(TEMP_HIGH+1)) {temp=2;}
		if(!EE_READ(Conf_TEMP_DAT+1)) {temp=0xFE;}
		switch(temp)
			{
				case 0: putst("Vnimanie! Ponigenie temperaturi.");break;
				case 1: putst("Temperatura norma.");break;
				case 2: putst("Vnimanie! Povishenie temperaturi.");break;
				case 0xFE: putst("Datchik temperaturi ne podklychen!.");break;
				default: putst("Avariya temperaturnogo datchika!");break;
			}

	putch(0x1A);			//cntrol Z
	check_OK(10);			//10 секунд ждать OK
	delay_s(1); 
}

//------------------------
void send_sms_slugeb(unsigned char cod_opovesheniya,unsigned char type_oth,unsigned  char n_abon)
{
unsigned char work,CH_DL;
	delay_s(2);

	wr_strbuf("\r",0);
switch (cod_opovesheniya)
	{
	case post_sn_kod:
		if(type_oth){wr_strbuf("\rSistema na ohrane.",1);}
			else{wr_strbuf("\rSistema snjata s ohrani.",1);}
		break;
	case test_kod:
			if(glob_knopka_system){wr_strbuf("\rTest. Sistema na ohrane.\r",1);}
				else {wr_strbuf("\rTest. Sistema snjata s ohrani.\r",1);}
			if(type_oth==3){bild_sms_balans();}	// если sms+ баланс - впечатать баланс
		break;
	case napr_kod:
		if(type_oth) wr_strbuf("\rBatarejnoe pitanie norma.",1);
			else wr_strbuf("\rNizkoe batarejnoe pitanie.",1);
		break;
	case temperatura_kod1:
	case temperatura_kod2:
	CH_DL=cod_opovesheniya-temperatura_kod1+1;
		if(temperatura_dallas[CH_DL-1]==0x7F){wr_strbuf("\rDat",1);wr_ptbuf_dec(CH_DL,1);wr_strbuf(" Temp.: Error",1);}
			else {
				if (temperatura_dallas[CH_DL-1]>=0){wr_strbuf("Dat",1);wr_ptbuf_dec(CH_DL,1);wr_strbuf(" Temp.:+",1);wr_ptbuf_dec(temperatura_dallas[CH_DL-1],1);}
					else { wr_strbuf("Dat",1);wr_ptbuf_dec(CH_DL,1);wr_strbuf(" Temp.:-",1);wr_ptbuf_dec(0-temperatura_dallas[CH_DL-1],1);}
				}	
			wr_strbuf("\r",1);
			switch(type_oth)
				{
				case 0:
					wr_strbuf("Vnimanie! Ponigenie temperaturi.",1);
					break;
				case 1: 
						wr_strbuf("Temperatura norma.",1);
					break;
				case 2:
					wr_strbuf("Vnimanie! Povishenie temperaturi.",1);
					break;
				case 0xFE:
					wr_strbuf("Datchik temperaturi ne podklychen!.",1);
					break;
				default:
					wr_strbuf("Avariya temperaturnogo datchika!",1);
					break;
			}
				
		break;
	default:
		break;
	}

	send_long_SMS(STR_Buf,n_abon);		//отправить SMS абоненту 0


}