#include <pic18.h>

//����� ������

#define in_napr		(PORTA&0x01) 		//< AN0 ���� ���������� ������� ����������
#define dso_vg   	(PORTA&0x02)		//< ���� DSO
#define q4_vg		(PORTA&0x04)		//<
#define q3_vg		(PORTA&0x08)		//<
#define q2_vg		(PORTA&0x10)		//<
#define q1_vg		(PORTA&0x20)		//<



#define dat0 		(PORTB&0x01)		//AN12
#define dat1 		(PORTB&0x02)		//AN10
#define dat2 		(PORTB&0x04)		//AN8
#define pwr_key			LB3				//B3>������ ������� ������
//#define in_tel		(PORTB&0x10)	    //B4<AN11 ������ ������ ������

#define csEEP		LC0		//>��� ������ ����
#define pwm2		LC1		//>
#define pwm			LC2		//>
#define sck_eep		LC3		//>
#define sdi_eep		(PORTC&0x10)		//C4<
#define sdo_eep		LC5		//>
#define dat_tel_tx	LC6		//>usart
#define dat_tel_rx	(PORTC&0x80)		//C7<


#define sys_key			(PORTD&0x01)		//D0<��������� ������
#define led_sost		LD1		//> ����� ����������
#define out2			LD2		//> ����������� ����� 2
//#define sig_ohran		LD2 	//> ����������� ����� 1
#define out1			LD3		//> ����������� ����� 1
//#define siren 			LD3 	//> ����������� ����� 2
#define SW_vc_mic		LD4 	//> ������������� ����� ��������
#define TX_inf			LD5 	//> TX ��� �������� ������ �������� ������ ���������� ������
#define dat_prog_tx		LD6		//>usart
#define dat_prog_rx		(PORTD&0x80)		//D7<



#define ms_dallas1_in		(PORTE&0x01)		//<������� ���� I2C
#define ms_dallas1_out		LE0		//<
#define ms_dallas2_in		(PORTE&0x02)		//	>
#define ms_dallas2_out		LE1		//	>
#define in_voice		(PORTE&0x04)		//	<AN7




//#define ms_sda_in		(PORTE&0x01)		//<������� ���� I2C
//#define ms_sda_out		LE0		//<
//#define ms_scl_in		(PORTE&0x02)		//	>
//#define ms_scl_out		LE1		//	>
//#define in_voice		(PORTE&0x04)		//	<AN7


//�������� ������
#define  mask_a 	0b11111111
#define  mask_b 	0b11110111    //
#define  mask_c    	0b10010000
#define  mask_d    	0b10000001		//AAAA
#define  mask_e    	0b00001101




#define tris_sda	TRISE

#define mask_dallas1_in  		0b00000001
#define mask_dallas1_out 		0b11111110
#define mask_dallas2_in  		0b00000010
#define mask_dallas2_out 		0b11111101


