#include <pic18.h> 
#include "name_port.h"
#include "my_delay.h"
#include "usart.h"
#include "usart_buf.h" 
#include "name_konst.h"

extern unsigned char SW_uart;		//переключатель рабочего COM порта

unsigned long adr_counter;
#define MAX_BUF_SPI  MAX_BUF_STR		
unsigned char Bufer_SPI[MAX_BUF_SPI];	//буфер SPI ограничен максимальной длинной строки
unsigned char Bufer_Change[MAX_BUF_spiChange];


//const char READ          =0x03;		// normal read  25Mhz or less bus speed
//const char READ_HS       =0x0B;		// read with 8 extra clocks between address and data. for high speed bus 
//const char SECTOR_ERASE  =0x20;		// Sector Erase
//const char BLOCK_ERASE   =0x52;		// Block Erase
//const char BLOCK_ERASE_OP=0xD8;		// Block Erase
//const char CHIP_ERASE    =0x60;		// Chip Erase
//const char CHIP_ERASE_OP =0xC7;		// Chip Erase optional cmd
//const char BYTE_PROGRAM  =0x02;		// Byte Program
//const char AAI_PROGRAM   =0xAD;		// auto increment mode Word program
//const char RDSR          =0x05;		// Read Status Register
//const char EWSR          =0x50;		// Enable Write Status Register
//const char WRSR          =0x01;		// Write Status register
//const char WREN_sst      =0x06;		// Write Enable
//const char WRDI          =0x04;		// Write Disable
//const char RDID          =0x90;		// Read ID
//const char RDID_OP       =0xAB;		// Read ID, optional cmd
//const char JEDEC_ID      =0x9F;		// JEDEC ID read cmd
 

//====================================
void wrSPI(char c)
{
unsigned char work;
WCOL1=0;
work=SSP1BUF;
SSP1BUF=c;
while(!BF1);
c=1;
} 


unsigned char Get_Byte(void)
{
   wrSPI(0);
  return SSPBUF;
}
//------------------------
unsigned char FlashStRead(void)		//чтение регистра статуса
{
csEEP=0;     //чип селект
wrSPI(0x05);
wrSPI(0);
csEEP=1;     // чип унселект
CLRWDT();
return SSPBUF;
}
//-----------------------------------------
void initSPI(void)
{
//res_eep=0;
//test_out=0;
delay_ms(100);
//test_out=1;
//res_eep=1;					//reset
TRISC5=0;
TRISC4=1;
TRISC3=0;
//SSPCON1=0b00110000;    		//частота на 4
SSPCON1=0b00110001;    		//частота на 16
delay_ms(100);
FlashStRead();
FlashStRead();
FlashStRead();
}

/************************************************************************/
/* PROCEDURE: WRSR							*/
/* This procedure writes a byte to the Status Register.			*/
/************************************************************************/
void WRSR(unsigned char byte)
{
	csEEP=0;			/* enable device */
	wrSPI(0x01);		/* select write to status register */
	wrSPI(byte);		/* data that will change the status of BPx or BPL (only bits 2,3,4,5,7 can be written) */
	wrSPI(0);			// Configuration registr
	csEEP=1;			/* disable the device */
}
/************************************************************************/
/* PROCEDURE: GBPU							*/
/* This procedure Flobal block protection unlook
/*					*/
/************************************************************************/
void GBPU(void)
{
	csEEP=0;
	wrSPI(0x98);
	csEEP=1;
}
/************************************************************************/
/* PROCEDURE: WREN							*/
/* This procedure enables the Write Enable Latch.  It can also be used 	*/
/* to Enables Write Status Register.					*/
/************************************************************************/
void WREN_sst(void)
{
	csEEP=0;
	wrSPI(0x06);
	csEEP=1;
}
/************************************************************************/
/* PROCEDURE: WRDI							*/
/* This procedure disables the Write Enable Latch.			*/
/************************************************************************/
void WRDI(void)
{
	csEEP=0;			/* enable device */
	wrSPI(0x04);		/* send WRDI command */
	csEEP=1;			/* disable device */
}


/************************************************************************/
/* PROCEDURE: Jedec_ID_Read						*/
/* This procedure Reads the manufacturer's ID (BFh), memory type (25h)  */
/* and device ID (41h).  It will use 9Fh as the JEDEC ID command.    	*/
/* Please see the product datasheet for details.  			*/
/* Returns:								*/
/*		IDs_Read:ID1(Manufacture's ID = BFh, Memory Type (25h), */
/*		 and Device ID (80h)					*/
/************************************************************************/ 
unsigned long Jedec_ID_Read(void) 
{
	unsigned long temp;
	temp = 0;
	csEEP=0;			 /* enable device */
	wrSPI(0x9F);		 /* send JEDEC ID command (9Fh) */
    temp = (temp | Get_Byte()) << 8; /* receive byte */
	temp = (temp | Get_Byte()) << 8;	
	temp = (temp | Get_Byte()); 	 /* temp value = 0xBF2541 */
	csEEP=1;			 /* disable device */

	return temp;
} 

/************************************************************************/
/* PROCEDURE: Chip_Erase						*/
/* This procedure erases the entire Chip.				*/
/************************************************************************/
void Chip_Erase()
{			
	WREN_sst();
	GBPU();					//global unprotect	
	WREN_sst();
	csEEP=0;				/* enable device */
	wrSPI(0xC7);			/* send Chip Erase command (60h or C7h) */
	csEEP=1;				/* disable device */
}
//------------
void Open_SST (void)
{
	WREN_sst();
	GBPU();					//global unprotect	
}

/************************************************************************/
/* PROCEDURE: Sector_Erase						*/
/* This procedure Sector Erases the Chip.				*/
/************************************************************************/
void Sector_Erase(unsigned long set_adr)
{
	while(FlashStRead()&0x01) ; 		//ждем готовности
	WREN_sst();
	csEEP=0;				/* enable device */
	wrSPI(0x20);			/* send Sector Erase command */
	wrSPI(*((char*)&set_adr+2));       //передаем адрес
	wrSPI(*((char*)&set_adr+1));       //передаем адрес
	wrSPI(*((char*)&set_adr));         //передаем адрес
	csEEP=1;				/* disable device */
}
/************************************************************************/
/* PROCEDURE: Block_Erase						*/
/* This procedure Block Erases 8-32 KByte of the Chip.			*/
/************************************************************************/
void Block_Erase(unsigned long set_adr)
{
	WREN_sst();
	csEEP=0;				/* enable device */
	wrSPI(0xD8);			/* send 32 KByte Block Erase command */
	wrSPI(*((char*)&set_adr+2));       //передаем адрес
	wrSPI(*((char*)&set_adr+1));       //передаем адрес
	wrSPI(*((char*)&set_adr));         //передаем адрес
	csEEP=1;				/* disable device */
}


/************************************************************************/
// PROCEDURE:	Write one byte to memory 					

/************************************************************************/
void Write_One_Byte(unsigned long set_adr, unsigned char byte)
{
	csEEP=0;				/* enable device */
	wrSPI(0x02);			/* send AAI command */
	wrSPI(*((char*)&set_adr+2));       //передаем адрес
	wrSPI(*((char*)&set_adr+1));       //передаем адрес
	wrSPI(*((char*)&set_adr));         //передаем адрес
	wrSPI(byte);			/* send 1st byte to be programmed */	
	csEEP=1;				/* disable device */
	while(FlashStRead()&0x01) ; //ждем готовности

}

/************************************************************************/
// PROCEDURE:	Write two byte to memory				
//							
/************************************************************************/
void Write_Two_Byte(unsigned long set_adr,unsigned char byte1, unsigned char byte2)
{
	csEEP=0;				/* enable device */
	wrSPI(0x02);			/* send AAI command */
	wrSPI(*((char*)&set_adr+2));       //передаем адрес
	wrSPI(*((char*)&set_adr+1));       //передаем адрес
	wrSPI(*((char*)&set_adr));         //передаем адрес
	wrSPI(byte1);			/* send 1st byte to be programmed */
	wrSPI(byte2);			/* send 2nd byte to be programmed */
	csEEP=1;				/* disable device */

	while(FlashStRead()&0x01) ; //ждем готовности

} 

//---------------------
void Change_Buf_Read(unsigned long set_adr)			//чтение  фрейма MAX_BUF_spiChange байт с начала страницы 4096 байт в буфер для замены 
{
unsigned int i_int;
set_adr&=0xFFF000;					// отбросить лишнее для исключения ошибок
while(FlashStRead()&0x01) ; 		//ждем готовности
csEEP=0;              				//чип селект
wrSPI(0x03);       					//чтение mem
wrSPI(*((char*)&set_adr+2));       //передаем адрес
wrSPI(*((char*)&set_adr+1));       //передаем адрес
wrSPI(*((char*)&set_adr));         //передаем адрес
CLRWDT();
for(i_int=0;i_int<MAX_BUF_spiChange;i_int++)
   {
    wrSPI(0);
    Bufer_Change[i_int]=SSPBUF;
   }
csEEP=1;
return;
}
//-------------------
void Change_Buf_Write(unsigned long set_adr)			//запись  фрейма MAX_BUF_spiChange байт с начала страницы 4096 байт из буфера для замены 
{
unsigned int i_int;
set_adr&=0xFFF000;					// отбросить лишнее для исключения ошибок
  while(FlashStRead()&0x01) ; //ждем готовности
  WREN_sst();
  GBPU();	
  WREN_sst();
  Write_Two_Byte(set_adr,Bufer_Change[0],Bufer_Change[1]);
	CLRWDT();
	for(i_int=2;i_int<=(MAX_BUF_spiChange-2);i_int+=2)
	{
 //	 while(FlashStRead()&0x01) ; //ждем готовности+++
		WREN_sst();
		set_adr+=2;
		Write_Two_Byte(set_adr,Bufer_Change[i_int],Bufer_Change[i_int+1]);
	}
}
//------------------
void Frame_Buf_Read(unsigned long set_adr,char len)			//чтение  фрейма N байт в буфер SPI если len равн 0 то до признака 0 или 0xFF
{
char pr_end;
unsigned char cnt=0;
pr_end=len;
if(!len) len=MAX_BUF_SPI-1;
while(FlashStRead()&0x01) ; //ждем готовности
csEEP=0;              //чип селект
wrSPI(0x03);       //чтение mem
wrSPI(*((char*)&set_adr+2));       //передаем адрес
wrSPI(*((char*)&set_adr+1));       //передаем адрес
wrSPI(*((char*)&set_adr));         //передаем адрес
//for(i=0;i<4;i++)
//   wrSPI(0);   //передаем 4 байта холостых

do
   {
   wrSPI(0);
  Bufer_SPI[cnt]=SSPBUF;
	if((!pr_end)&&(!Bufer_SPI[cnt])) break;	// если конец строки то стоп 
	CLRWDT();
	cnt++;
   }while(--len);
csEEP=1;
return;
}
//-----------------------------------

void Frame_Buf_Write(unsigned long set_adr,unsigned char len)			//запись врейма через модификацию в буфере для замены
{
unsigned int cnt_buf,adr_buf;
unsigned long adr_sector,work_long;

adr_sector=set_adr&0xFFF000;					// отбросить лишнее 
work_long=set_adr-adr_sector;
if (work_long>=MAX_BUF_spiChange) return;				// ЕСЛИ адрес выходит за размеры буфера для изменений то не писать
Change_Buf_Read(adr_sector);				//прочитать сектор в буфер
adr_buf=work_long;
cnt_buf=0;
do
	{
	  Bufer_Change[adr_buf]=Bufer_SPI[cnt_buf];
	  cnt_buf++;
	  adr_buf++;
      len--;
    } while(len);
WREN_sst();
GBPU();
WREN_sst();
Sector_Erase(adr_sector);
Change_Buf_Write(adr_sector);
}

//------------------------------------------------
void start_FL_read(unsigned long set_adr)							//выставить на чтение flash по адресу
{
adr_counter=set_adr;
while(FlashStRead()&0x01) ; //ждем готовности
csEEP=0;              //чип селект
wrSPI(0x03);       //чтение mem
wrSPI(*((char*)&set_adr+2));       //передаем адрес
wrSPI(*((char*)&set_adr+1));       //передаем адрес
wrSPI(*((char*)&set_adr));         //передаем адрес
}
//----------------------------------------------
void stop_FLASH(void)											// отключиться от флеш							
{
   csEEP=1;
}
//----------------------------
char count_FL_read(void)							//выставить на чтение flash по адресу
{
char tmp_w;
//unsigned long adr;
  wrSPI(0);
   tmp_w=SSPBUF;
	adr_counter++;
	CLRWDT();
return tmp_w;
}
//------------------------------------------------
void start_FL_write(unsigned long set_adr,unsigned char byte0,unsigned char byte1)							//выставить на запись flash по адресу на лету
{
while(FlashStRead()&0x01) ; //ждем готовности
WREN_sst();
GBPU();
WREN_sst();
adr_counter=set_adr;
adr_counter+=2;
Write_Two_Byte(set_adr,byte0,byte1);
CLRWDT();
}
//-------------------------------
void count_FL_write(unsigned char byte0,unsigned char byte1)							//выставить запись flash по адресу на лету
{
//while(FlashStRead()&0x01) ; //ждем готовности+++

		WREN_sst();
		Write_Two_Byte(adr_counter,byte0,byte1);
	    adr_counter+=2;

}
//-----------------------
void end_FL_write(void)
{
stop_FLASH();
}



//--------------------------------
void Frame512_Fl_Write(unsigned long set_adr,unsigned long len)			//запись и прием фрейма пакетами по 256 байта
{
unsigned char fl_read_error,work,check_summ;
unsigned int count_paket,cnt_paket_write;
clear_buf_RX(SW_uart);
adr_counter=set_adr;
check_summ=0;
count_paket=512;
WREN_sst();
GBPU();

do{
  while(FlashStRead()&0x01); //ждем готовности
  WREN_sst();
  csEEP=0;				/* enable device */
  wrSPI(0x02);			/* send AAI command */
	wrSPI(*((char*)&set_adr+2));       //передаем адрес
	wrSPI(*((char*)&set_adr+1));       //передаем адрес
	wrSPI(*((char*)&set_adr));         //передаем адрес
		for(cnt_paket_write=0;cnt_paket_write<256;cnt_paket_write++)
		{
 		 work=getch_n(1,SW_uart,&fl_read_error);   //читаем байт  
 			if(fl_read_error) {putst("\rError Time\r"); goto end_write_paket;}
			  check_summ^=work;
			  count_paket--;
	 		 if(!len)goto end_len;
				set_adr++;
				wrSPI(work);			/* send 1st byte to be programmed */
     		 if(!(--len)) break;
		} 
	csEEP=1;				/* disable device */

	  if (!count_paket)
		{
		    putch(check_summ);							// ACK
			check_summ=0;
			count_paket=512;
		}
 }while(len);
end_len:
if(count_paket!=512)
	{for(;count_paket!=0;count_paket--)
		{check_summ^=getch_n(1,SW_uart,&fl_read_error);	// дочитать до конца пакет, и ответить
		 if(fl_read_error) {putst("\rError Time\r"); goto end_write_paket;}
		}
			
		   putch(check_summ);							// ACK
	}
end_write_paket:
stop_FLASH();
}
//-------------------------
unsigned char Frame512_Fl_Read(unsigned long set_adr,unsigned long len)			//чтение и вывод фрейма c подтвеждением 
{
unsigned char fl_read_error,work;
unsigned int count_paket;
clear_buf_RX(SW_uart);
csEEP=0;              //чип селект
wrSPI(0x03);       //чтение mem
wrSPI(*((char*)&set_adr+2));       //передаем адрес
wrSPI(*((char*)&set_adr+1));       //передаем адрес
wrSPI(*((char*)&set_adr));         //передаем адрес
count_paket=0;
do
   {
   wrSPI(0);
   work=SSPBUF;
	if (!count_paket) {count_paket=512;if(getch_n(1,SW_uart,&fl_read_error)!='+'){csEEP=1; return 0;}}				// если не пришел ответ error
	count_paket--;
	putch(work);					//эхо
	CLRWDT();
   }while(--len);
	while(count_paket)		// дописать до конца пакета
	{
	putch(0);
	count_paket--;
	}
csEEP=1;
return 1;
}
