//¬ерси€ программы
#define	Ver_Signal "Alarm Module"

#define	Ver_Build "Ver 2.01" 
#define	time_build  __TIME__    // врем€ компил€ции
#define	date_build  __DATE__	// дата компил€ции

#define	Name_Bluetooth "Alarm Module sn:"


#define	PWM1	1	//
#define	PWM2	2	//

#define mod_GSM   0
#define komputer  1
#define Bluetooth_CH 2

//  датчики
#define zona_ohran  0
#define zona_1   	1
#define zona_2	 	2
//#define zona_3	 	3
//#define zona_4	 	4
#define in_trub		20
#define speek_rec	50
#define in_voice_rec	100

#define last_zon 2
//#define all_zon 12

//#define last_zon_contactID 16
//#define const_dat_wait 100		//константа задержки (реакции)

#define zon_lo	1
#define zon_mid	2
#define zon_hi	3


#define LOW_bort_napr 119		//нижн€€ граница(при 100к-15к  11,8в(12 c диодом)) 
#define HIGH_bort_napr	121		//верхн€€ граница  12(12.2в c диодом)

//-------
#define voice_in  	0
#define mik_in    	1
#define mik_ext_in  2



#define cost_wt_dtmf 100
#define  zirochka (0b00001011+'0')
#define  rehetka (0b00001100+'0')
//---коды служебных сообщений

#define post_sn_kod  0		// постановка сн€тие
#define test_kod  	 1		// тестовое сообщение
#define napr_kod     2		// пропадание напр€жени€
#define temperatura_kod1  3  // температуры датчика 1
#define temperatura_kod2  4  // температуры датчика 2


#define MAX_NUMB		  32      //максимальна€ длинна цифр в строке программировани€ DTMF
#define MAX_BUF_UART	  32      //максимальна€ длинна кольцевого UART буфера
#define MAX_BUF_STR		  128     //максимальна€ длинна буфера дл€ работы с короткими сообщени€ми типа строка
#define MAX_BIG_BUF_STR	  512    //максимальна€ длинна буфера дл€ работы с длинными сообщени€ми типа строка (используетс€ дл€ длинных SMS
#define MAX_BUF_spiChange 1024 	  // промежуточный дл€ SPI чтени€ модификации записи. 
#define MAX_BUF_RX_COM	  128     // максимальна€ длинна буфера дл€ приема команд
#define MAX_BUF_BLTH      128     //максимальна€ длинна Bluetooth буферов