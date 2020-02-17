void terminal_on (void);
void terminal_PDP (void);
unsigned char uncode_com(register const char *str);
void read_parametr(unsigned long Adres);
void print_parametr(unsigned long Adres);
unsigned long calk_parametr(char ukaz);
unsigned char	multi_write(unsigned char);
unsigned char	multi_read(unsigned char);
unsigned char load_command_terminal(unsigned char);
void detect_command_terminal(char ukaz,char marker_viz);

extern unsigned char buf_rx_command[];
