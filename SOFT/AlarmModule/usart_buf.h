
void putst_buf(char *str);
void wr_strbuf(register const char *str, char point_n);
void wr_strbuf_buf( char *str, char point_n);
void wr_ptbuf(unsigned char c, char point_n);
void wr_ptbuf_dec(unsigned char c, char point_n);
unsigned char find_N_char(unsigned char *massive,unsigned char simvol,unsigned char n_cnt,unsigned int MAX_STR);
void set_ukaz_zap(char*ukz,char zap);
unsigned char compare_buf(char *bufer1,char *bufer2,char end_symb,char max_buf2);
unsigned char compare_str_buf(register const char *str,unsigned char *bufer);
char copy_buf(char *bufer1,char *bufer2,char end_symb,char max_buf2);
unsigned long calk_parametr_buf(unsigned char  *bufer,unsigned char end_symb);	
void wr_ptbuf_long(unsigned long c, char point_n);

extern unsigned char STR_Buf[];
