//Функции программирования параметров
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
//10-19 Настройки коммуникатора основные				
				'*','1','1','1','#',0,0,0,0,0,0,0,0,0,0, //0x01	USSD строка
				'5','5','5',0,		//0x10 код доступа
				1,					//0x14 включение запроса пароля на входе
				1,			 		//0x15 включение оповешения о состоянии сигнализации на входе
				0,					//0x16 выбор языка меню
				0,					//0x17 подключение расширителя 8х7х8
 				'1','2','3','4',0,			//0x18 PIN Bluetooth
				0,0,0,
				0,
//20-39 Настройка отчетов
				0,					//0x21 включение впечатывания даты времени SMS
				0,					//0x22 ячейка включения оповещения при восстановлении зон
				1,					//0x23 режим отправки тестовых сообщений
				0,					//0x24 период рассылки тестовых сообщений в часах
				8,					//0x25 время старта тестовых сообщений
				0,					//0x26 способ отчета о постановке снятии 0 не отчитываться 1 SMS 2 voice
				1,					//0x27 режим отправки сообщений о пропаже напряжения
				0,0,0,0,0,0,0,0,
				2,					//0x30 колличество дозвонов на номер
				1,					//0x31 колличество смс на номер 
				0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,
//40-59 Настройка зон
				10,					//0x40 время на вход
				10,					//0x41 время на выход
				10,					//0x42 время бездействия
				10,					//0x43 сохраненная константа реакции датчикв вмиллисекундах /10
				0,0,0,0,
				1,3,3,1,1,1,1,1,1,1,1,1,1,1,1,1, //0x48 режимы напряжения на зонах по 47 (16 зон)
				0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x58	конфигурация  зон по 57  0-обычная 1-входная-проходная	2-круглосуточная,3-зона бездействия
										 // старший бит в 1 зона заблокирована. в 0 разблокирована
				0,					//0x68 конфигурация постановки снятия 0 -постановка снятие тумблером 1 -постановка снятие кнопкой 2,3 - +звуковая     индикация 4- режим контроя для работы с централями
				0,					//0x69 время в минутах для автопостановки на охрану (если 0 то без автопостановки)
				0,0,0,0,0,0,
//60-69 Настройка выходов
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x70 привязка  выходов к голосовым сообщениям по 67 (8 выходов) 
				1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x80 конфигурация  выходов по 6F (16 выходов)  
										 						   // 0- выход включается на 1 сек 1- инверсия 2- специального назначения
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x90	привязка зон к выходам   0-привязки нет или номер выхода срабатываемый при тревоге в зоне. работает только с триггерными выходами  
				5,					//0xA0 время работы сирены
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const char eep_reset2[]={
//70-79 Настройка тревожных сообщений
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xB0	конфигурация  СМС 16 зон
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xC0	конфигурация  VOICE 16 зон
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xD0	конфигурация  СМС 32 зон для Contact ID
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xF0 конфигурация  VOICE 32 зон для Contact ID
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//80-89 Настройка температурного датчика.
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x110 включение внешних датчиков температуры (16 шт)0- выключен 1- Включен 2- термо нагрев 
										                           // 3- термо охлаждение 4 - нагрев охлаждение
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x120 Настройка отчетов датчиков температуры (16шт) 0- выключен 1- SMS отчет 2- Голосовой отчет 
				5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, //0x130 нижнее значение  датчиков температуры (16шт)
				40,40,40,40,40,40,40,40,40,40,40,40,40,40,40,40, //0x140 верхнее значение датчиков температуры (16шт)
				2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, //0x150 гистерезис температуры				 (16шт)
//100-119 Настройка Contact ID. 
				'5','4','3','2','1',0,0,0,0,0,0,0,0,0,0,0, //0x160  включение постановки снятия через комманды contact_id
				0,					//0x170	Включение постановки-снятия командами CONTACT ID 
				0,					//0x171	ячейка включения режима отладочного протокола в порт программирования 
				0,0,0,0,0,0};		//End
				

const unsigned int mas_size1=sizeof(eep_reset1);
const unsigned int mas_size2=sizeof(eep_reset2);
unsigned int i_int;

		for (i_int=0;i_int<mas_size1;i_int++)	{ EE_WRITE(i_int,eep_reset1[i_int]);CLRWDT();}	//	записать начальные настройки
		for (i_int=0;i_int<mas_size2;i_int++)	{ EE_WRITE(i_int+mas_size1,eep_reset2[i_int]);CLRWDT();}	//	записать начальные настройки
		EE_WRITE(save_bort_napr,1);
		EE_WRITE(system_status,0);		// снять с охраны
		EE_WRITE(knopka_system,0);		// кнопка снять с охраны
	// очистить параметры сигнализации
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
	put_command("AT+BTPOWER=1\r");			// включить Bluetooth
	delay_s(3);
 	putst("AT+BTUNPAIR=0\r");
	delay_ms(500);
	Frame_Buf_Read(IMEI_Save,0);			// прочитать имей
 	putst("AT+BTHOST=");putst(Name_Bluetooth);putst_buf(Bufer_SPI+11);(putst("\r");
	delay_ms(500);
	put_command("AT+BTPOWER=0\r");			// выключить Bluetooth
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
if (vhod) 							//если зашли от кнопки
  {
	fl_ring_yes=1;
	delay_s(1);
	if(vhod==1){if (sys_key) return;}				//проверить кнопку еще раз
 	put_command("ATH\r");						
 	led_inform=7;							// занять линию
	for(i_int=0;i_int<60;i_int++)	{delay_ms(250);if (sys_key) break;}
	if(i_int==60)			//если удерживали кнопку больше 15 секунд полный сброс
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
	clear_buf_RX(Bluetooth_CH);				//очистить буфер приема Bluetooth
	put_command("AT+BTPOWER=1\r");			// включить Bluetooth
	read_lexem(1,3);
	EE_READ_String(pin_bluetooth,work_string);// Прочитать PIN блютузв
	putst("AT+BTPAIRCFG=1,");putst_buf(work_string);putst("\r");	// включить Bluetooth
	read_lexem(2,1);

	for (i_int=0;i_int<1200;i_int++)						//ожидать 120 сек звонка
  	{
		if(fl_ring_yes)  break;
		if(fl_bt_CONNECT_yes) {Bluetooth_Connecting();putch(0x1A);break;}
		if(fl_bt_DISCONN_yes) {putch(0x1A);break;}
	 	delay_ms(100);
		if (!sys_key) {led_inform=0;delay_s(1);put_command("AT+BTPOWER=0\r");delay_s(2);return;} //проверить кнопку если нажата возврат
  	}
	delay_s(1);
	put_command("AT+BTPOWER=0\r");	// выключить Bluetooth
	delay_s(1);
	if(!fl_ring_yes) return; 		//не дождались звонка
	put_command("ATA\r");			// снять трубку
 	read_lexem(1,30);		// прочитать 1 лексему 30 сек
 	if (uncode_com("CONNECT")) {terminal_PDP();return;}	//звонок данными
 	if (!uncode_com("OK")){putst("+++");delay_s(1);putst("\rATH\r");return;}//eсли голос то на голос	
  }
	delay_s(1);
	change_in(voice_in);		
	delay_ms(250);
	voice_speech(regim_program,PWM1);		//режим программирования

		delay_s(1);
	 while(1)
   		{ 
		if(detekt_razg()!=2) return;		// eсли разговор оборвался то выход
		delay_s_dso(5);
		if (dso_vg) {if(set_dtmf()==rehetka)detect_command();}
		voice_speech(naberite_komandu,PWM1);					// наберите комманду
   		}
}
//-----------------------------
unsigned int calk_DTMF_parametr (unsigned char n_cif)
{
unsigned int wrk_int,mnog,i;
	if(!n_cif) return 0;
	wrk_int=0;
	mnog=1;						// множитель разряда
	for (i=n_cif;i!=0;i--)
	{
	wrk_int+=((unsigned int)(number_str[i-1]-'0'))*mnog;
	mnog*=10;	
	}
	return wrk_int;
}
//---------------
void Voice_param_prinjat (void)
{	delay_ms(500);voice_speech(param_prinjat,PWM1);}					// параметр принят

void set_Single_Parametr(unsigned char max,unsigned int eep_adr,unsigned int param) //запись параметра в память с проверкой и голосом
{	if  (param>max){sound_error(); return;}		 // проверить диапазон
	EE_WRITE(eep_adr,(unsigned char)param);		// прописать параметр
	Voice_param_prinjat();			// параметр принят
}
void set_Two_Parametr(unsigned char max1,unsigned char max2,unsigned int eep_adr,unsigned int param1,unsigned int param2) //запись параметра в память с проверкой и голосом
{	if  (param1>max1){sound_error(); return;}		 // проверить диапазон
	if  (param2>max2){sound_error(); return;}		 // проверить диапазон
	EE_WRITE(eep_adr+param1,(unsigned char)param2);		// прописать параметр
	Voice_param_prinjat();			// параметр принят
}
//--------
void detect_command(void)
{
unsigned char n_cif_param,i,work,num_abon,ukaz,type_command;
unsigned int num_command,num_parametr1,num_parametr2;
	n_cif_param=load_parametr();
	if ((!n_cif_param)||(n_cif_param>4))	{sound_error(); return;}	//принять комманду, если с ошибкой- выход 
	num_command=calk_DTMF_parametr(n_cif_param);		//посчитать номер комманды
	type_command=check_base_command(num_command); //проверить по базе тип комманды (1 или 2 параметра)
	if (!type_command)	{sound_error(); return;} //если совпадений в базе не найдено выход с ошибкой
	if (type_command>20){sound_error(); return;} //если совпадений в базе только SMS выход с ошибкой
	if (type_command>10){type_command-=10;}      //если совпадений в базе только DTMF то коррекция значения
	n_cif_param=load_parametr();
	if (!n_cif_param)	{sound_error();return;}	//принять параметр запомнить колличество цифр параметра если 0 цифр,ошибка
	if (n_cif_param<=3) {num_parametr1=calk_DTMF_parametr(n_cif_param);}	//если параметр до 3 х цифр то посчитать номер параметра 1
	if(type_command==2)								// если у команды 2 параметра то посчитать первый и запомнить второй
		{
		n_cif_param=load_parametr();
		if (!n_cif_param)	{sound_error(); return;}	//принять параметр 2, если с ошибкой- выход 
		if (n_cif_param<=3) {num_parametr2=calk_DTMF_parametr(n_cif_param);}	//если параметр до 3 х цифр то посчитать номер параметра 2
		}

if(num_command==3775)			//команда сброса на заводские параметры
			{
			if (num_parametr1==0) {reset_nastrojki();voice_speech(nast_po_umolch,PWM1); while(1){};}	//  если код сброса то сбросить все настройки
			if (num_parametr1==1) {update_module();	voice_speech(nast_po_umolch,PWM1);  while(1){};}   	//  если код 37751 то то записать все настройки модуля
			return;
			}	

switch (num_command)						//анализ комманды
	{
	case 0: break;
//1-9 Настройки номеров абонентов
	case 1:										// запись абонента
		if  ((!num_parametr1)||(num_parametr1>15)){sound_error(); return;}				
		Bufer_SPI[0]='0';				// отступить под категорию
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
		delay_ms(500);voice_speech(nomer_sohranen,PWM1);					// номер сохранен
		break;
 	case 2:										//стирание абонента
		if  ((!num_parametr1)||(num_parametr1>15)){sound_error(); return;}
		num_abon=num_parametr1-1;
		clear_num(num_abon);
		Voice_param_prinjat();				// параметр принят
		break;
	case 3:										//программирование категории абонента
		if  ((!num_parametr1)||(num_parametr1>15)){sound_error(); return;}
		if (num_parametr2>9){sound_error(); return;}
		num_abon=num_parametr1-1;
		read_kateg(num_abon);					// прочитать все
		Bufer_SPI[0]=num_parametr2+'0';
		write_num(num_abon);				// записать новое значение
		Voice_param_prinjat();				// параметр принят
		break;
 	case 4:										//программирование режима работы абонента
		if  ((!num_parametr1)||(num_parametr1>15)){sound_error(); return;}
		if (num_parametr2>9){sound_error(); return;}
		num_abon=num_parametr1-1;
		read_regim(num_abon);					// прочитать все
		Bufer_SPI[1]=num_parametr2+'0';
		write_num(num_abon);				// записать новое значение
		Voice_param_prinjat();					// параметр принят
		break;
	case 9:
										//стирание всех абонентов
		if  (num_parametr1!=1){sound_error(); return;}
			for (i=0;i<15;i++) 
				{clear_num(i);CLRWDT();}
		Voice_param_prinjat();					// параметр принят
			break;
//10-19 Настройки коммуникатора основные
	case 10: 							//USSD запрос
			if (n_cif_param>15) {sound_error(); return;}	//если больше 15 х цифр
			n_cif_param++;
			number_str[n_cif_param]=0;
			ukaz=copy_buf(Bufer_SPI,number_str,0,MAX_BUF_RX_COM);//скопировать по 0
			Frame_Buf_Write(USSD_Balans,ukaz+1);
			Voice_param_prinjat();
			break;
	case 11:							//программирование кода доступа
			if (n_cif_param!=3) {sound_error(); return;}	//не 3 цифры
			while (n_cif_param)
			{
				n_cif_param--;
				EE_WRITE(kod_parol+n_cif_param,number_str[n_cif_param]);	//записать принятые цифзы
			}
			EE_WRITE(zero_place,0);
			Voice_param_prinjat();	
			break;
	case 12:							//	Запрос кода доступа для входа в систему.
			set_Single_Parametr(1,Menu_pass,num_parametr1);	break;
	case 13:							//	Оповещение о состоянии системы.
			set_Single_Parametr(1,Voice_otchet,num_parametr1);	break;
	case 14:							//	Выбор языка голосового меню и сообщений.
			set_Single_Parametr(1,menu_language,num_parametr1);	break;
	case 15:							//программирование PIN кода BLUETOOTH
			if (n_cif_param!=4) {sound_error(); return;}	//не 4 цифры
			while (n_cif_param)
			{
				n_cif_param--;
				EE_WRITE(pin_bluetooth+n_cif_param,number_str[n_cif_param]);	//записать принятые цифзы
			}

//20-39 Настройка отчетов
	case 20:								//установка системной даты и времени
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
	case 21:							//Впечатывание даты в SMS сообщения.
			set_Single_Parametr(1,on_Date_Time,num_parametr1);	break;
	case 22:							//Оповещение о восстановлении зон.
			set_Single_Parametr(1,Opov_Vostan,num_parametr1);	break;
	case 23:							//Режим отправки тестовых сообщений
			set_Single_Parametr(3,regim_TEST,num_parametr1);	break;
	case 24:							//Период рассылки тестовых сообщений.
			set_Single_Parametr(255,period_TEST,num_parametr1);	break;
	case 25:							//Время старта первого тестового сообщения. 
			set_Single_Parametr(23,start_TEST,num_parametr1);	break;
	case 26:							//Сообщение о постановке и снятия с охраны 
			set_Single_Parametr(2,otchet_postan,num_parametr1);	break;
	case 27:							//Сообщение о снижении внешнего питания.
			set_Single_Parametr(2,regim_BORT_napr,num_parametr1);	break;

	case 30:							//Количество звонков на номер
			set_Single_Parametr(255,lim_DOZVON,num_parametr1);	break;
	case 31:							//Количество SMS на номер
			set_Single_Parametr(255,lim_SMS,num_parametr1);	break;
//40-59 Настройка зон
	case 40:							//Задержка на вход.
			set_Single_Parametr(255,time_VHOD,num_parametr1);	break;
	case 41:							//Задержка на выход.
			set_Single_Parametr(255,time_VIHOD,num_parametr1);	break;
	case 42:							//Задержка бездействия для зоны бездействия.
			set_Single_Parametr(255,time_STAND,num_parametr1);	break;
	case 43:							//Время реакции зон.
			set_Single_Parametr(255,Save_const_dat_wait,num_parametr1);	break;
	case 44:							//Режим уровня напряжения на зоне. (15 зон)
			set_Two_Parametr(15,3,regim_napr,num_parametr1,num_parametr2);	break;
	case 45:							//Конфигурация (выбор типа) зон  (15 зон)
			set_Two_Parametr(15,3,konf_ZONE,num_parametr1,num_parametr2);	break;
	case 46:							//Блокировка зон  установкой старшего бита конфигурации в 1 (15 зон)
			if  (num_parametr2>1){sound_error(); return;}
			work=EE_READ(konf_ZONE+num_parametr1);
			if(!num_parametr2) work&=0x7F;
			if(num_parametr2)  work|=0x80;
			set_Two_Parametr(15,131,konf_ZONE,num_parametr1,work);	break;

	case 50:							//Режим работы входа постановки снятия
			set_Single_Parametr(4,Conf_post_sn,num_parametr1);	break;
	case 51:							//Автопостановка на охрану.
			set_Single_Parametr(255,time_auto_set,num_parametr1);	break;
//60-69 Настройка выходов
	case 60:							//Конфигурация имени выхода с коррекцией (выход 1 это ячейка 0) (16 выходов)
			set_Two_Parametr(15,6,vihod_VOICE,num_parametr1-1,num_parametr2);	break;
	case 61:							//Конфигурация имени выхода с коррекцией (выход 1 это ячейка 0) (16 выходов)
			set_Two_Parametr(15,2,konf_VIHOD,num_parametr1-1,num_parametr2);	break;
	case 62:							//Привязка зон к выходам. (16 зон 16 выходов)
			set_Two_Parametr(16,16,link_Zone_Vih,num_parametr1,num_parametr2);	break;
	case 63:							//Время работы сирены.
			set_Single_Parametr(255,time_SIREN,num_parametr1);	break;
//70-79 Настройка тревожных сообщений
	case 70:							//SMS отчеты для зон модуля . (15 зон)
			set_Two_Parametr(15,255,konf_SMS,num_parametr1,num_parametr2);	break;
	case 71:							//Голосовые отчеты для зон модуля  (15 зон)
			set_Two_Parametr(15,255,konf_VOICE,num_parametr1,num_parametr2);	break;
	case 72:							//SMS отчеты для внешних Contact ID зон (31 зон)
			set_Two_Parametr(31,255,konf_SMS_Cid,num_parametr1,num_parametr2);	break;
	case 73:							//Голосовые отчеты для внешних Contact ID зон (31 зон)
			set_Two_Parametr(31,255,konf_VOICE_Cid,num_parametr1,num_parametr2);break;

	case 75:							//Запись своих голосовых сообщений для отчетов зон
			if ((!num_parametr1)||(num_parametr1>16)){sound_error(); return;}
			voice_rec(num_parametr1);break;	
	case 76:							//Запись своих голосовых сообщений для отчетов зон
			if ((!num_parametr1)||(num_parametr1>16)){sound_error(); return;}
			voice_play(num_parametr1,PWM1);break;
//80-89 Настройка температурного датчика.
	case 80:							//Конфигурация  температурных датчиков.(16 датчиков)
			set_Two_Parametr(15,3,Conf_TEMP_DAT,num_parametr1-1,num_parametr2);	break;
	case 81:							//Настройка отчетов превышения порогов температуры.(16 датчиков)
			set_Two_Parametr(15,2,Inf_TEMP_DAT,num_parametr1-1,num_parametr2);	break;
	case 82:							//Нижний порог датчиков температуры.(16 датчиков)
			set_Two_Parametr(15,255,TEMP_LOW,num_parametr1-1,num_parametr2);	break;
	case 83:							//Верхний порог датчиков температуры. (16 датчиков)
			set_Two_Parametr(15,255,TEMP_HIGH,num_parametr1-1,num_parametr2);break;
	case 84:							//Гистерезис датчиков температуры.(16 датчиков)
			set_Two_Parametr(15,255,TEMP_GIST,num_parametr1-1,num_parametr2);break;
//100-119 Настройка Contact ID.
	case 100: 							//Номер телефона для ответа Contact ID
			if (n_cif_param>15) {sound_error(); return;}	//если больше 15 х цифр
			number_str[n_cif_param]=0; // заптсать конец строки на место решетки
			ukaz=copy_buf(Bufer_SPI,number_str,0,MAX_BUF_RX_COM);//скопировать по 0
			Frame_Buf_Write(Number_Contact_ID,ukaz+1);
			Voice_param_prinjat();
	case 101:							//Включение постановки-снятия командами CONTACT ID 
			set_Single_Parametr(1,post_CONACT_ID,num_parametr1);	break;
	case 102:							//Включение отладочного монитора CONTACT ID 
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
  switch (num_command)						//анализ комманды
	{
	case 0: return 0;
//1-9 Настройки номеров абонентов
	case 1:	return 11;	// только DTMF
	case 2:	return 11;	// только DTMF	
	case 3:	return 2;	
	case 4:	return 2;
	case 5:	return 22;	// только SMS
	case 9:	return 11;	// только DTMF
//10-19 Настройки коммуникатора основные
	case 10:	return 1;	
	case 11:	return 1;	
	case 12:	return 1;	
	case 13:	return 1;
	case 14:	return 1;
//20-39 Настройка отчетов
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
//40-59 Настройка зон
	case 40:	return 1;	
	case 41:	return 1;	
	case 42:	return 1;	
	case 43:	return 1;
	case 44:	return 2;
	case 45:	return 2;	
	case 46:	return 2;	
	case 50:	return 1;	
	case 51:	return 1;
//60-69 Настройка выходов
	case 60:	return 2;	
	case 61:	return 2;	
	case 62:	return 2;	
	case 63:	return 1;
//70-79 Настройка тревожных сообщений
	case 70:	return 2;	
	case 71:	return 2;	
	case 72:	return 2;	
	case 73:	return 2;
	case 74:	return 22;   //только SMS
	case 75:	return 11;   // только DTMF	
	case 76:	return 11;	// только DTMF
//80-89 Настройка температурного датчика.
	case 80:	return 2;	
	case 81:	return 2;	
	case 82:	return 2;	
	case 83:	return 2;
	case 84:	return 2;
//100-119 Настройка Contact ID.
	case 100:	return 1;	
	case 101:	return 1;	
	case 102:	return 1;
// комманды сброса на заводские установки
	case 3775:  return 11;   // только DTMF	
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
		wr_prom=number_str[n_cif]=wait_dtmf(30);								// без тайм аута
		if (!(wr_prom&&(wr_prom^0xFF))) {return 0;}	//если номер принят с ошибкой, или начался набор, или нажата * выход c ошибкой
		if((++n_cif)>=MAX_NUMB) {return  0;}								//если цифр больше 32  ошибка
	}
	n_cif--;
 return n_cif;							//вернуть указатель на последнюю цифру(решетку)	
}


