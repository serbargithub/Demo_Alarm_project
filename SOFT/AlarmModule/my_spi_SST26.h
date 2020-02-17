

unsigned long Jedec_ID_Read();



void initSPI(void);
void wrSPI(char);
char FlashStRead(void);
void Frame_Buf_Read(unsigned long set_adr,char len);
void Frame_Buf_Write(unsigned long set_adr,char len);
void start_FL_read(unsigned long);
char count_FL_read(void);
void stop_FLASH(void);
void Frame512_Fl_Write(unsigned long set_adr,unsigned long len);
unsigned char Frame512_Fl_Read(unsigned long set_adr,unsigned long len);

void start_FL_write(unsigned long set_adr,unsigned char byte0,unsigned char byte1);
void count_FL_write(unsigned char byte0,unsigned char byte1);
void end_FL_write(void);
void Sector_Erase(unsigned long set_adr);
void Chip_Erase();
void Open_SST (void);


extern   unsigned char pageRL;
extern unsigned char Bufer_SPI[];
