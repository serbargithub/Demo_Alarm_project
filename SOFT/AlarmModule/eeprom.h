char EE_READ (unsigned int addr);
void EE_WRITE (unsigned int addr,char var);
int EE_READ_Int (unsigned int addr);
void EE_WRITE_Int (unsigned int addr,int var);
void EE_WRITE_Long (unsigned int addr,long var);
long EE_READ_Long (unsigned int addr);
void EE_WRITE_String (unsigned int addr,char *str);
void  EE_READ_String (unsigned int addr,char *str);

unsigned char read_eep_frame(unsigned int set_adr_eep,unsigned int cnt_byte_eep);
unsigned char write_eep_frame(unsigned int set_adr_eep,unsigned int cnt_byte_eep);
/*
extern int EE2int (unsigned int addr);
extern void int2EE (unsigned int addr,int var);
extern void float2EE (unsigned int addr,double var);
extern double EE2float (unsigned int addr);
*/
