

void serial_setup(unsigned char,unsigned char);
unsigned char check_err(void);
void clear_buf_RX(unsigned char num_usart);
unsigned int read_buf_RX(unsigned char num_usart);
unsigned char getch_n(unsigned char cnt_byte,unsigned char num_usart,unsigned char *fl_read_error);
unsigned char getch_us(unsigned char num_usart);
void putch(unsigned char c);
void putst(register const char *str);
void putst_noenter(register const char *str);
unsigned char put_command(register const char *str);
void putchhex(unsigned char c);
void putinthex(unsigned int c);
void putchdec(unsigned char c);
void putch_long(unsigned long c);
void monitor_hex(char temp);
unsigned char check_OK(unsigned int time);



#define sp2400	0
#define sp4800	1
#define sp9600	2
#define sp19200	3
#define sp38400	4
#define sp57600	5
#define sp115200 6

#define PRI_US	0		//порт 1 usart
#define SEC_US	1		//порт 2 usart



