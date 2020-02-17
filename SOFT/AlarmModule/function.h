char wait_dtmf (char time);
char cod_dtmf(void);
char set_dtmf(void);
char detekt_razg(void);
unsigned char wait_razg(char time);

unsigned char load_cif(unsigned char time_out);
unsigned char load_pulse(void);
unsigned char load_num(unsigned char n_cif);
void beep_on(char,char);
void sound_error(void);
void sound_ok(void);
void razgov_on(void);
unsigned char delay_s_trub(unsigned char wait);
void busy_on(void);
void power_on(void);
void reset_module(void);
unsigned char ring_system (void);
void telef_on(void);
	
void ch_usart(char ch);
void change_in (char num);

unsigned char check_reg_GSM(void);
void reboot_GSM(void);
void read_imei(void);