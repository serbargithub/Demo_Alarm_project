void initI2C_my(void);
void I2C_my_start(void);
void I2C_my_stop(void);
char I2C_my_putch(char byte);
char I2C_my_getch(char set_ACK);
unsigned char transl_I2C(char,char);
unsigned char wait_scl_ms(int wait);
unsigned char read_answer(char,char);
unsigned char connect_I2C_master (char,char);
void putst_msI2C_buf(const char *str,char fl_ukaz);
void putch_msI2C_buf( char byte,char fl_ukaz);
char wait_send(void);
unsigned long calck_I2Cpar_ms(char ukaz);
char find_charI2C_ms(char simvol,char n_cnt);
char read_dat_rash(void);
char konf_dat_rash(void);
char save_trig_rash(void);

#define MAX_BUF_MS_I2C	128
extern bank3 unsigned char buf_ms_I2C[];


#define ADR_Rash 	0xE0
#define ADR_Loud 	0xD0
