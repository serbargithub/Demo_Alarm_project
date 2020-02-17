void siren_on(char regim);
void set_zone_int(char n_zone);
unsigned char det_com_Signal(char num_command,char n_cif_param);
unsigned char check_in_ring(void);
void init_mask_siren(void);
void set_out_siren_int(unsigned char sost_ch);
void set_out_siren(unsigned char sost_ch);
void init_datchik(void);
char check_zone_alarm(char r_kont);
void check_datchik(void);
void send_TEST(void);
void send_opov_napr(void);
void voice_alarm(char num_zone,char map_alarm,char type_alarm);
char menu_trevoga(char n_abon);
void opov_postanovka_snjatie (char type_oth);
void sound_opoveshenie(unsigned char infodat);
unsigned char set_param_time(char adr_sm,char n_cif);
void send_opov_temperatura(unsigned char conf_dallas,unsigned char type_opov,unsigned char CH_DL);
void set_vihod(char n_vihod,char sost_ch);
char check_vihod(char n_vihod);
void set_out_ohran(unsigned char sost_ch);  // выставить 

void voice_menu(char);
void clear_zone_alarm(char);
void alarm_dozvon(char regim);
char snd_alarm_voice(char n_abon,char regim);
void init_OK_zone(void);
void voice_slugeb(unsigned char cod_opovesheniya,unsigned char type_oth);
void opoveshenie_slugebnoe(unsigned char cod_opovesheniya,unsigned char type_oth, unsigned char type_opov);
