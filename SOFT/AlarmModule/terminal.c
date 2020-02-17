#include <pic18.h>
#include "ctype.h"
#include "usart.h" 
#include "usart_buf.h" 
#include "my_spi_SST26.h"
#include "my_delay.h"
#include "function.h"
#include "lexem_rutine.h"
#include "name_konst.h"
#include "name_adres.h"
#include "name_port.h"
#include "eeprom.h"
#include "SMS_rutine.h"

extern unsigned char fl_bridge_to_RX,SW_uart;		// SW_uart ������������� �������� COM �����
unsigned char terminal_echo_ON;
unsigned char buf_rx_command[MAX_BUF_RX_COM];   // ����� �������� �������� ������
unsigned char work_string[MAX_BUF_RX_COM];
extern unsigned char end_bufrx,st_bufrx;
extern unsigned char st_bufrx2,end_bufrx2;
extern unsigned char st_bufRxblth,end_bufRxblth,count_Txblth;
extern unsigned int cnt_secund;
extern  unsigned char led_inform;
extern unsigned char opros_dat_enable,fl_sec_code,fl_no_carier_yes,fl_no_carier;
void detect_command_terminal(char ukaz,char);
char detect_command_programmator(char ukaz);
unsigned char	multi_write(unsigned char);
unsigned char	multi_read(unsigned char);
//--------------------------------------------------------------

unsigned char load_command_terminal(unsigned char echo_answer)		// �������� �������  ������������ � �� ��������� - ��������� �� �������� ���� ������  
																	// �������� 1 ��� �������� 0 ���������
{
char temp;
static char count_ukaz; 
static unsigned char start_ld_comm;
 CLRWDT();
if(SW_uart==0){if (end_bufrx==st_bufrx){ return 0; }}// ��� ������� �����
if(SW_uart==1){if (end_bufrx2==st_bufrx2){ return 0;}}
if(SW_uart==Bluetooth_CH){if (end_bufRxblth==st_bufRxblth){ return 0;}}
 temp=getch_us(SW_uart);
if(!temp) return 0;
if (!start_ld_comm) {count_ukaz=0; if(!((temp=='A')||(temp=='a'))) { return 0;}}
	start_ld_comm=1;
  	buf_rx_command[count_ukaz]=temp;
if ((count_ukaz==1)&&(!((temp=='T')||(temp=='t')))){start_ld_comm=0;return 0;} //���� �� AT
if(echo_answer){	putch(temp);}			//���  
	if (temp=='\b') {if(echo_answer){putst(" \b");}count_ukaz--; return 0;}
	if ((temp==0x0d)||(temp==0x0a))
			{
			  start_ld_comm=0;
				if(echo_answer)	// ��������� ���������
				{if (temp==0x0d){putch(0x0a);}
					else {putch(0x0d);}
				}	
				buf_rx_command[count_ukaz+1]=0;// ������� �����
			 return	count_ukaz;
			}
	if (count_ukaz<(MAX_BUF_RX_COM-2)) count_ukaz++;
	return 0;
}
//---------

unsigned char uncode_com(register const char *str)		//�������� ��������
{
char ukaz=0;
	while((*str)!=0)
	{
    if ((*str)!=buf_rx_command[ukaz]) {return 0;}	// �� ������� ��������
		ukaz++;
		str++;
	}
return 1;		// ������ ����������
}

//--------------------------------------------------------------
unsigned long calk_parametr(char ukaz)					//���������� ��������� �� ������ ( �������� �������� ���� ������ 0xOD)
{
unsigned long mnog,work_long;
ukaz--;			//���������
work_long=0;
mnog=1;						// ��������� �������
while ((buf_rx_command[ukaz]!='=')&&(buf_rx_command[ukaz]!=',')&&(buf_rx_command[ukaz]!=0x20))
	{
	work_long+=(buf_rx_command[ukaz]-'0')*mnog;	
	mnog*=10;
	if (!(ukaz--)) {return 0;}		// ������ �� ������
	}
return work_long;
}
//----------------------------------
void save_parametr(unsigned long Adres)
{
char ukaz_tek,ukaz;
		ukaz_tek=find_N_char(buf_rx_command,'=',1,MAX_BUF_RX_COM);	// ����� =
		ukaz_tek++;
		ukaz=copy_buf(Bufer_SPI,&buf_rx_command[ukaz_tek],0x0D,MAX_BUF_RX_COM);//����������� �� 0D
		Bufer_SPI[ukaz]=0;		//������� �����
		Frame_Buf_Write(Adres,ukaz+1);
		putst("\rWrite OK\r");

}
//------------------------
void read_parametr(unsigned long Adres)
{
		Frame_Buf_Read(Adres,0);					// ��������� �� ����
		putst("\r");putst_buf(Bufer_SPI);putst("\r");

}
//------------------------
void print_parametr(unsigned long Adres)
{
		Frame_Buf_Read(Adres,0);					// ��������� �� ����
		putst_buf(Bufer_SPI);

}
//------------------------
void change_uart_init(char napravl)
{
if (napravl==mod_GSM)
	{
	GIE=0;
	T2CON=0b00011000;	//���� ���/4
	TMR2IF=0;
	TMR2ON=1;
	GIE=1;
	ch_usart(mod_GSM);
	serial_setup(sp115200,PRI_US);		//������������������� usart
	}
if (napravl==komputer)
	{
	GIE=0;
	T2CON=0b00001000;	//���� ���/2
	TMR2IF=0;
	TMR2ON=1;
	GIE=1;
	ch_usart(komputer);
	serial_setup(sp115200,SEC_US);		//������������������� usart
	}
if (napravl==Bluetooth_CH)
	{
	GIE=0;
	T2CON=0b00011000;	//���� ���/4
	TMR2IF=0;
	TMR2ON=1;
	GIE=1;
	ch_usart(mod_GSM);
	serial_setup(sp115200,PRI_US);		//������������������� usart
	ch_usart(Bluetooth_CH);
	}
}

//======================================== �������� �������
void terminal_on(void)
{
char work,i;
//if(!in_tel)	return;
opros_dat_enable=0;
led_inform=0;delay_s(1);
while(!sys_key){led_inform=3;delay_ms(100);}
change_uart_init(komputer);

putst("\rSystem Ready\r");
led_inform=5;					// ������������ ��������� �� ��������� �� ��������
terminal_echo_ON=1;				// �������� ��� 
clear_buf_RX(SW_uart);
while (1)					// ���� ����� ������
{
	work=load_command_terminal(terminal_echo_ON);
	if (work)for(i=0;i<=work;i++) 
			{buf_rx_command[i]=(char)toupper(buf_rx_command[i]);
			if(buf_rx_command[i]=='=')break;	// �������� � �������� ��������
			}
	if(work){
			detect_command_terminal(work,0);			//����� �������� �� at ������������
			}
	check_err();
	CLRWDT();
		if(!sys_key)
				{
				 delay_ms(500);
				if(!sys_key) 
					{
					while(!sys_key){led_inform=3;delay_ms(100);}
					break;	
					}
				}
}
putst("\rSystem Close\r");

change_uart_init(mod_GSM);

opros_dat_enable=1;
}

//---------------------------------
void terminal_PDP(void)
{
char work,i;
fl_no_carier_yes=0;
fl_no_carier=1;
putst("\rSystem Ready\r");
led_inform=5;					// ������������ ��������� �� ��������� �� ��������
terminal_echo_ON=1;				// �������� ��� 
cnt_secund=0;
while (!fl_no_carier_yes)					// ���� ����� ������
{
	work=load_command_terminal(terminal_echo_ON);
	if (work)for(i=0;i<=work;i++) 
			{buf_rx_command[i]=(char)toupper(buf_rx_command[i]);
			if(buf_rx_command[i]=='=')break;	// �������� � �������� ��������
			}
	if(work) detect_command_terminal(work,1);			//����� �������� �� at ������������
	check_err();
	CLRWDT();
	if (cnt_secund>300)			// 5 ����� �� ������
		{
		putst("\rSystem Close\r"); 
		putst("+++");delay_s(1);putst("\rATH\r");	// ����� �����
		break;
		}
}
fl_no_carier=0;
opros_dat_enable=1;
}

//-----------------------------------
void detect_command_terminal(char ukaz,char marker_viz)
{
unsigned char fl_read_error,ukaz_tek,wrk_char,i,SW_save;
unsigned long wrk_long;
unsigned int wrk_int;
static unsigned long set_adr;
static unsigned int set_adr_eep;
if (uncode_com("AT\r")) 	{goto	end_ok;	} 
if (uncode_com("ATI1")) 				//������ ������ ��
		{
		putst(Ver_Build);	//��������� ������
		putst(" Build: ");putst(time_build);
		putst(" ");putst(date_build);	//��������� ������ 
		goto	end_ok;
		}  
if (uncode_com("ATI")) 				//������ �������������� ����������
		{putst(Ver_Signal);goto	end_ok;} //��������� ������ 
if (uncode_com("ATE0")) 				//��������� ��� 
		{terminal_echo_ON=0;goto	end_ok;}  
if (uncode_com("ATE1")) 				//�������� ��� 
		{terminal_echo_ON=1;goto	end_ok;}  

if (uncode_com("AT+WR=")) 					//�������� ������ ���������������
		{
		multi_write(0);
		return;
		} 
if (uncode_com("AT+RD=")) 					//�������� ������ ���������������
		{
		multi_read(0);
		return;
		}
if (uncode_com("AT+ADR=")) 					//��������� ������� �����
		{
		set_adr=calk_parametr(ukaz);
		goto	end_ok;
		} 
if (uncode_com("AT+ADR_EEP=")) 					//��������� ������� ����� EEP
		{
		set_adr_eep=calk_parametr(ukaz);
		goto	end_ok;
		} 
if (uncode_com("AT+WRPAKET=")) 				//�������� N ���� �������� �� 64
		{
		wrk_long=calk_parametr(ukaz);
		putst("\r>");
		Frame512_Fl_Write(set_adr,wrk_long);			// ������� � ��������  ����� �� 64 ����� � ������
		goto	end_ok;
		} 
if (uncode_com("AT+RDPAKET=")) 				//��������� N ���� �������� �� 64
		{
		wrk_long=calk_parametr(ukaz);
		putst("\r>");
		if (Frame512_Fl_Read(set_adr,wrk_long))	goto	end_ok;				// ��������� � ������  ����� �� 64 ����� � ������
		goto	end_ok;
		}
if (uncode_com("AT+WR_EEP=")) 				//������ ������ N ���� � EEPROM �� ������
		{
		ukaz_tek=find_N_char(buf_rx_command,',',1,MAX_BUF_RX_COM);	// ����� ,
		set_adr_eep=(int)calk_parametr(ukaz_tek);	// ��������� �����
		wrk_int=calk_parametr(ukaz);			// ��������� ��������
		if(!write_eep_frame(set_adr_eep,wrk_int))return; // ���� ������ (timeout)
		goto	end_ok;			//	
		}  
if (uncode_com("AT+RD_EEP=")) 				//������ ������ N ���� �� EEPROM �� ������
		{
		ukaz_tek=find_N_char(buf_rx_command,',',1,MAX_BUF_RX_COM);	// ����� ,
		set_adr_eep=(int)calk_parametr(ukaz_tek);	// ��������� �����
		wrk_int=calk_parametr(ukaz);			// ��������� ��������
		read_eep_frame(set_adr_eep,wrk_int);
		goto	end_ok;			//	
		} 
if (uncode_com("AT+CHIP_ERASE")) 				//�������� ���� ����������
		{
		Chip_Erase();
		goto	end_ok;			//	
		} 
if (uncode_com("AT+SECTOR_ERASE=")) 				//�������� ������� 4�� �� ������
		{
		wrk_long=calk_parametr(ukaz);			// ��������� ��������
		Sector_Erase(wrk_long);
		goto	end_ok;			//	
		} 

if (uncode_com("AT+BRIDGE=")) 					//��������� ������������ com1 �� com2 ��� ��������
		{
		fl_bridge_to_RX=calk_parametr(ukaz);
		goto	end_ok;
		}
//-------------------------------------
if ((marker_viz)&&uncode_com("AT+TIME?")) 				//������ ���������� �������
		{
		SW_save=SW_uart;
		change_uart_init(mod_GSM);
		putst("AT\r");
		delay_ms(250);
		put_command("AT+CCLK?\r");						// ���� � �������� ������ - ����������
		wr_ptbuf(getch_n(11,SW_uart,&fl_read_error),0);
		for(i=0;i<13;i++){wr_ptbuf(getch_n(1,SW_uart,&fl_read_error),1);}	//�������� �����
		delay_ms(200);
		change_uart_init(SW_save);
		putst_buf(STR_Buf);
		clear_buf_RX(SW_uart);
		delay_ms(250);
		goto	end_ok;
		}  
if ((marker_viz)&&uncode_com("AT+TIME=")) 				//��������� ���������� �������
		{
		SW_save=SW_uart;
		change_uart_init(mod_GSM);
		ukaz=find_N_char(buf_rx_command,'=',1,MAX_BUF_RX_COM);	// ����� =
		ukaz++;
		putst("AT\r");
		delay_ms(250);
		putst("AT+CCLK=\"");
		for(i=0;i<14;i++){putch(buf_rx_command[ukaz]);ukaz++;}	//�������� �����
		putst(":00+00\"\r");
		delay_ms(500);
		change_uart_init(SW_save);
		clear_buf_RX(SW_uart);
		goto	end_ok;
		}  
if ((marker_viz)&&uncode_com("AT+BALANCE?")) 				//������ ������� �� �����
		{
		SW_save=SW_uart;
		change_uart_init(mod_GSM);
		wr_strbuf(" ",0);									//�������� ����� ������ ���������
		bild_sms_balans();
		change_uart_init(SW_save);
		STR_Buf[120]=0;			//�������� ��������� � ������� �� 120 ��������
		putst_buf(STR_Buf);
		clear_buf_RX(SW_uart);
		delay_ms(250);
		goto	end_ok;
		}  
if ((marker_viz)&&uncode_com("AT+STATUS?")) 				//������ ������� ���� (����������� � ������� �������)
		{
		SW_save=SW_uart;
		change_uart_init(mod_GSM);
		if(put_command("AT+CREG?\r")){change_uart_init(SW_save);goto end_error;}
		wr_ptbuf(getch_n(12,SW_uart,&fl_read_error),0);
		wr_ptbuf(':',1);
		delay_ms(250);
		if (put_command("AT+CSQ\r")) {change_uart_init(SW_save);goto end_error;}
		getch_n(8,SW_uart,&fl_read_error);
		for(i=0;i<5;i++) 
		{	wrk_char=getch_n(1,SW_uart,&fl_read_error);
			if ((wrk_char=='\r')||(wrk_char==',')) break;
			wr_ptbuf(wrk_char,1);
		}
		change_uart_init(SW_save);
		putst_buf(STR_Buf);
		clear_buf_RX(SW_uart);
		delay_ms(250);
		goto	end_ok;
		} 
//------------------------------------

if (uncode_com("AT+SERV_IP1=")) 				//IP ����� ������� ������� ������ �� �D
		{
		save_parametr(ADR_IP1);
		goto	end_ok;
		} 
if (uncode_com("AT+SERV_IP1?")) 				//IP ����� ������� ������� ������ �� �D
		{
		read_parametr(ADR_IP1);
		goto	end_ok;
		} 
if (uncode_com("AT+GET_ST=")) 				//���� � ������� GET ������ �� �D
		{
		save_parametr(GET_ST);
		goto	end_ok;
		} 
if (uncode_com("AT+GET_ST?")) 				//IP ����� ������� ������� ������ �� �D
		{
		read_parametr(GET_ST);					// ��������� �� ����
		goto	end_ok;
		} 
if (uncode_com("AT+APN=")) 				//������ APN �� �D
		{
		save_parametr(APN);
//		putst("\rWrite APN OK\r");

		goto	end_ok;
		} 
if (uncode_com("AT+APN?")) 				//������ APN �� �D
		{
		read_parametr(APN);					// ��������� �� ����
		goto	end_ok;
		} 
if (uncode_com("AT+IND_NUM=")) 				//�������������� ����� ���������� 4 ����� 
		{
		ukaz_tek=find_N_char(buf_rx_command,'=',1,MAX_BUF_RX_COM);	// ����� =
		ukaz_tek++;
		ukaz=copy_buf(Bufer_SPI,&buf_rx_command[ukaz_tek],0x0D,MAX_BUF_RX_COM);//����������� �� 0D
		Bufer_SPI[ukaz]=0;		//������� �����
		if(ukaz<4) 	{putst("\rWrong Parametr\r");goto	end_ok;}
		Frame_Buf_Write(IND_NUM,ukaz+1);
		putst("\rWrite IND_NUM OK\r");
		goto	end_ok;
		} 
if (uncode_com("AT+IND_NUM?")) 				//�������������� ����� ���������� 
		{
		read_parametr(IND_NUM);					// ��������� �� ����
		goto	end_ok;
		} 
if (uncode_com("AT+WR_STR_AT=")) 				//������ ������ � at45 �� ������
		{
		ukaz_tek=find_N_char(buf_rx_command,',',1,MAX_BUF_RX_COM);	// ����� ,
		wrk_long=calk_parametr(ukaz_tek);
		ukaz_tek++;
		ukaz=copy_buf(Bufer_SPI,&buf_rx_command[ukaz_tek],0x0D,MAX_BUF_RX_COM);//����������� �� 0D
		Bufer_SPI[ukaz]=0;		//������� �����
		Frame_Buf_Write(wrk_long,ukaz+1);
		putst("\rWrite OK\r");
		goto	end_ok;			//	
		}  
if (uncode_com("AT+RD_STR_AT=")) 				//������ ������ � ��45 �� ������
		{
		wrk_long=calk_parametr(ukaz);
		read_parametr(wrk_long);
		goto	end_ok;			//	
		}  
if (uncode_com("AT+WR_BYTE_EEP=")) 				//������ ����� � EEPROM �� ������
		{
		ukaz_tek=find_N_char(buf_rx_command,',',1,MAX_BUF_RX_COM);	// ����� ,
		wrk_long=calk_parametr(ukaz_tek);		// ��������� �����
		wrk_char=calk_parametr(ukaz);			// ��������� ��������
		EE_WRITE((int)wrk_long,wrk_char);
		putst("\rWrite OK\r");
		goto	end_ok;			//	
		}  
if (uncode_com("AT+RD_BYTE_EEP=")) 				//������ ����� �� EEPROM �� ������
		{
		wrk_long=calk_parametr(ukaz);
		putst("\r");putchdec(EE_READ((int)wrk_long));putst("\r");
		goto	end_ok;			//	
		}  

end_error:
putst("\rERROR\r");
CLRWDT();
return;
end_ok:
putst("\rOK\r");
}


//=================================================================================

unsigned char check_str(register const char *str,unsigned char ukaz)		//�������� ������ � ���������
{
	while((*str)!=0)
	{
    if ((*str)!=buf_rx_command[ukaz]) {return 0;}	// �� ������� ��������
		ukaz++;
		str++;
	}
return 1;		// ������ ����������
}
//----------------------------------------
unsigned char multi_write(unsigned char type_ask)					// ������ ��������������� � ������
																	//type_ask �������� ����������� ������� 0 ��� �������� 1- SMS ������� 
{																	// 	1 - ����������, �� ������� ������
																	//	2- ����������, ������� ������
																	//	3-���������� �� � ���������� ������
unsigned long adres_save,parametr_save;
unsigned char ukaz,ukaz_tek,ukaz_param,type_memory,type_parametr,sig;
ukaz_tek=find_N_char(buf_rx_command,'=',1,MAX_BUF_RX_COM);		// ����� =
ukaz_param=find_N_char(buf_rx_command,',',2,MAX_BUF_RX_COM);		// ����� ������ �������, �� ��������
if(ukaz_param==0xFF) if(!type_ask){{putst("#ERR PARAMETR\r");}return 3;}      // ������ ���������
type_memory=buf_rx_command[ukaz_tek+1];					// ��������� ��� ������������ ������
if((type_memory!='f')&&(type_memory!='e')) {if(!type_ask){putst("#ERR ADRES\r");}return 3;}      // ������ ������
adres_save=calk_parametr_buf(&buf_rx_command[ukaz_tek+2],',');  //��������� ����� ������
ukaz_tek=find_N_char(buf_rx_command,',',1,MAX_BUF_RX_COM);		// ����� ,
if(ukaz_tek==0xFF) {if(!type_ask){putst("#ERR TYPE\r");}return 3;}      // ������ ����
ukaz_tek++;											// ����� �� ��� ��������� � ����� ���������������
type_parametr=0;
if (check_str("bt",ukaz_tek)) {type_parametr=1;}				//��� ������ byte
		else
	 if (check_str("sbt",ukaz_tek)) {type_parametr=2;}				//��� ������ sbyte	
		else
	 if (check_str("int",ukaz_tek)) {type_parametr=3;}				//��� ������ 
 		else
	 if (check_str("sint",ukaz_tek)) {type_parametr=4;}				//��� ������		
		else
	 if (check_str("ln",ukaz_tek)) {type_parametr=5;}				//��� ������	
		else
	 if (check_str("sln",ukaz_tek)) {type_parametr=6;}				//��� ������
		else
	 if (check_str("str",ukaz_tek)) {type_parametr=7;}				//��� ������ 
if(!type_parametr) {if(!type_ask){putst("#ERR TYPE\r");}return 3;}      // ������ ����

ukaz_param++;  // ������� �� ��������� ����� ������� 
if((type_parametr==1)||(type_parametr==3)||(type_parametr==5)) {parametr_save=calk_parametr_buf(&buf_rx_command[ukaz_param],0x0D);}  //��������� �������� �����������
if((type_parametr==2)||(type_parametr==4)||(type_parametr==6)) 
		{
			sig=buf_rx_command[ukaz_param];
			if((sig=='+')||(sig=='-')) {ukaz_param++;}		//��������� ���� �����, ���� ��� �� +
				else {sig='+';}
			parametr_save=calk_parametr_buf(&buf_rx_command[ukaz_param],0x0D);  //��������� �������� ��������
			if(sig=='-'){parametr_save=0-parametr_save;}      // ���� ������������ �� �������� �������������
		}
	// ��������� ���������, ���������� ����������
switch (type_parametr)
	{
		case 1:
		case 2:
				if(type_memory=='f')								//��������� BYTE �� FLASH
				{ 
			  		if(!type_ask){putst("#ERR ADRES\r");}return 3;      // ������ ������					
				}
				if(type_memory=='e')								//��������� BYTE �� EEPROM
				{ 
			  		EE_WRITE((unsigned int)adres_save,(unsigned char)parametr_save);		
				}
			break;
		case 3:
		case 4:
				if(type_memory=='f')								//��������� INT �� FLASH
				{ 
			  		if(!type_ask){putst("#ERR ADRES\r");}return 3;      // ������ ������					
				}
				if(type_memory=='e')								//��������� INT �� EEPROM
				{ 
			  		EE_WRITE_Int((unsigned int)adres_save,(unsigned int)parametr_save);		
				}	
			break;
		case 5:
		case 6:
				if(type_memory=='f')								//��������� LONG �� FLASH
				{ 
			  	if(!type_ask){putst("#ERR ADRES\r");}return 3;      // ������ ������					
				}
				if(type_memory=='e')								//��������� LONG �� EEPROM
				{ 
			  	EE_WRITE_Long((unsigned int)adres_save,(unsigned long)parametr_save);		
				}
			break;
		case 7:
				if(type_memory=='f')								//��������� ������ �� FLASH
				{ 
				 ukaz=copy_buf(Bufer_SPI,&buf_rx_command[ukaz_param],0x0D,MAX_BUF_RX_COM);//����������� �� 00
				 Bufer_SPI[ukaz]=0;		//������� �����
				 Frame_Buf_Write(adres_save,ukaz+1);					
				}
				if(type_memory=='e')								//��������� ������ �� EEPROM
				{ 
				 ukaz=copy_buf(work_string,&buf_rx_command[ukaz_param],0x0D,MAX_BUF_RX_COM);//����������� �� 00
				 work_string[ukaz]=0;		//������� �����
					EE_WRITE_String(adres_save,work_string);
				}
			break;
		default:
				if(!type_ask){putst("#ERR TYPE\r");}return 3;     // ������ ����
				break;
	}

if(!type_ask){putst("#OK\r");}      // 	
return 2;		
}
//-----------------------------------
unsigned char multi_read(unsigned char type_ask)					// ������ ��������������� �� ������
												// type_ask �������� ����������� ������� 0 ��� �������� 1- SMS ������� 
												// 	1 - ����������, �� ������� ������
												//	2- ����������, ������� ������
												//	3-���������� �� � ���������� ������
{
unsigned long adres_read,parametr_read;
unsigned char ukaz,ukaz_tek,type_memory,type_parametr,sig,i;
signed char sigbyte;
signed int sigint;
signed long siglong;

ukaz_tek=find_N_char(buf_rx_command,'=',1,MAX_BUF_RX_COM);		// ����� =
type_memory=buf_rx_command[ukaz_tek+1];					// ��������� ��� ������������ ������
if((type_memory!='f')&&(type_memory!='e')) {if(!type_ask){putst("#ERR ADRES\r");}return 3;}      // ������ ������
adres_read=calk_parametr_buf(&buf_rx_command[ukaz_tek+2],',');  //��������� ����� ������
ukaz_tek=find_N_char(buf_rx_command,',',1,MAX_BUF_RX_COM);		// ����� ,
if(ukaz_tek==0xFF) {if(!type_ask){putst("#ERR TYPE\r");}return 3;}      // ������ ����
ukaz_tek++;											// ����� �� ��� ��������� � ����� ���������������
type_parametr=0;
if (check_str("bt",ukaz_tek)) {type_parametr=1;}				//��� ������ byte
		else
	 if (check_str("sbt",ukaz_tek)) {type_parametr=2;}				//��� ������ sbyte	
		else
	 if (check_str("int",ukaz_tek)) {type_parametr=3;}				//��� ������ 
 		else
	 if (check_str("sint",ukaz_tek)) {type_parametr=4;}				//��� ������		
		else
	 if (check_str("ln",ukaz_tek)) {type_parametr=5;}				//��� ������	
		else
	 if (check_str("sln",ukaz_tek)) {type_parametr=6;}				//��� ������
		else
	 if (check_str("str",ukaz_tek)) {type_parametr=7;}				//��� ������ 

if(!type_parametr) {if(!type_ask){putst("#ERR TYPE\r");}return 3;}      // ������ ����
	// ��������� ���������, ���������� ����������
switch (type_parametr)
	{
		case 1:
				if(type_memory=='f')								//��������� BYTE �� FLASH
				{ 
			  		if(!type_ask){putst("#ERR ADRES\r");}return 3;      // ������ ������					
				}
				if(type_memory=='e')								//��������� BYTE �� EEPROM
				{ 
					if(!type_ask){putchdec(EE_READ((unsigned int)adres_read));putst("\r");}
						else
						{wr_strbuf(buf_rx_command,1);wr_ptbuf('\r',1);wr_ptbuf_dec(EE_READ((unsigned int)adres_read),1);wr_ptbuf('\r',1);}
				}
			break;
		case 2:
				if(type_memory=='f')								//��������� SBYTE �� FLASH
				{ 
			  		if(!type_ask){putst("#ERR ADRES\r");}return 3;      // ������ ������					
				}
				if(type_memory=='e')								//��������� SBYTE �� EEPROM
				{ 
					sigbyte=EE_READ((unsigned int)adres_read);
					if(!type_ask)
						{
						if(sigbyte<0){putch('-');sigbyte=0-sigbyte;}
							else{if(sigbyte>0){putch('+');}}			
							putchdec(sigbyte);putst("\r");
						}
						else
						{
							wr_strbuf(buf_rx_command,1);wr_ptbuf('\r',1);
							if(sigbyte<0){wr_ptbuf('-',1);sigbyte=0-sigbyte;}
								else{if(sigbyte>0){wr_ptbuf('+',1);}}
							wr_ptbuf_dec(EE_READ((unsigned int)adres_read),1);wr_ptbuf('\r',1);
						}
				}
			break;
		case 3:
				if(type_memory=='f')								//��������� INT �� FLASH
				{ 
			  		if(!type_ask){putst("#ERR ADRES\r");}return 3;      // ������ ������					
				}
				if(type_memory=='e')								//��������� INT �� EEPROM
				{ 
					if(!type_ask){putch_long(EE_READ_Int((unsigned int)adres_read));putst("\r");}
						else {wr_strbuf(buf_rx_command,1);wr_ptbuf('\r',1);wr_ptbuf_long(EE_READ_Int((unsigned int)adres_read),1);wr_ptbuf('\r',1);}		
				}	
			break;
		case 4:
				if(type_memory=='f')								//��������� SINT �� FLASH
				{ 
			  		if(!type_ask){putst("#ERR ADRES\r");}return 3;      // ������ ������					
				}
				if(type_memory=='e')								//��������� SINT �� EEPROM
				{ 
						sigint=EE_READ_Int((unsigned int)adres_read);
					if(!type_ask)
						{
						if(sigint<0){putch('-');sigint=0-sigint;}
							else{if(sigint>0  ){putch('+');}}			
						putch_long(sigint);putst("\r");
					    }
						else
						{
						wr_strbuf(buf_rx_command,1);wr_ptbuf('\r',1);
						if(sigint<0){wr_ptbuf('-',1);sigint=0-sigint;}
							else{if(sigint>0  ){wr_ptbuf('+',1);}}	
						wr_ptbuf_long(sigint,1);wr_ptbuf('\r',1);		
						}
				}	
			break;
		case 5:
				if(type_memory=='f')								//��������� LONG �� FLASH
				{ 
			  	if(!type_ask){putst("#ERR ADRES\r");}return 3;      // ������ ������					
				}
				if(type_memory=='e')								//��������� LONG �� EEPROM
				{ 
				if(!type_ask)
					   {parametr_read=EE_READ_Long((unsigned int)adres_read);putch_long(parametr_read);putst("\r");	}
					else {wr_strbuf(buf_rx_command,1);wr_ptbuf('\r',1);wr_ptbuf_long(EE_READ_Long((unsigned int)adres_read),1);wr_ptbuf('\r',1);}		
				}
			break;
		case 6:
				if(type_memory=='f')								//��������� SLONG �� FLASH
				{ if(!type_ask){putst("#ERR ADRES\r");}return 3; }    // ������ ������					
				if(type_memory=='e')								//��������� SLONG �� EEPROM
				{ 
					siglong=EE_READ_Long((unsigned int)adres_read);
					if(!type_ask)
						{
						if(siglong<0){putch('-');siglong=0-siglong;}
							else{if(siglong>0){putch('+');}}			
						putch_long(siglong);putst("\r");
						}
						else
						{
						wr_strbuf(buf_rx_command,1);wr_ptbuf('\r',1);
						if(siglong<0){wr_ptbuf('-',1);siglong=0-siglong;}
							else{if(siglong>0  ){wr_ptbuf('+',1);}}	
						wr_ptbuf_long(siglong,1);wr_ptbuf('\r',1);		
						}
						
				}
			break;
		case 7:
				if(type_memory=='f')								//��������� ������ �� FLASH
				{ 
					Frame_Buf_Read(adres_read,0);					// ��������� �� ����
					if(!type_ask){putst_buf(Bufer_SPI);putst("\r");	}
					   else
						{wr_strbuf(buf_rx_command,1);wr_ptbuf('\r',1);wr_strbuf(Bufer_SPI,1);wr_ptbuf('\r',1);}					
				}
				if(type_memory=='e')								//��������� ������ �� EEPROM
				{ 
				  EE_READ_String(adres_read,work_string);
					if(!type_ask){putst_buf(work_string);putst("\r");}
					   else
						{wr_strbuf(buf_rx_command,1);wr_ptbuf('\r',1);wr_strbuf(work_string,1);wr_ptbuf('\r',1);}		
				}
			break;
		default:
				if(!type_ask){putst("#ERR TYPE\r");}return 3;      // ������ ����
				break;
	}	
return 1;	
}


//**********************************************************************
