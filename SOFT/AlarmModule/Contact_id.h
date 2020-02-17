
unsigned char ck_start_id(void);					//посылка стартовых запросов
void end_signal(void);					// сигнал окончания приема
unsigned char ck_sprava(void);						// ожидание следующей посылки
unsigned char load_com_id(void);
unsigned char ld_cif_id(void);			// прием цифры+ перекодировка в контакт ид
unsigned char recive_id(void);								// функция приема сообщений  и записи их в буфер
															// результат колличество принятых сообщений

void print_command(char);				// принт принятых комманд
void rs_Contact_ID(void);
unsigned char det_comContact_ID(char,char);
unsigned char build_SMS(char );
unsigned char chek_num_central(void);

//#include "name_port.h";

