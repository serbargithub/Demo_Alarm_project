//���������� ������ ��� ����������������

__EEPROM_DATA (
		0,
//10-19 ��������� ������������� ��������				
				'*','1','1','1','#',0,0); __EEPROM_DATA (0,0,0,0,0,0,0,0); //0x01	USSD ������
__EEPROM_DATA (	'5','5','5',0,		//0x10 ��� �������
				1,					//0x14 ��������� ������� ������ �� �����
				1,			 		//0x15 ��������� ���������� � ��������� ������������ �� �����
				0,					//0x16 ����� ����� ����
				0);					//0x17 ����������� ����������� 8�7�8
__EEPROM_DATA ('1','2','3','4',0,			//0x18 PIN Bluetooth
				0,0,0);
__EEPROM_DATA (0,
//20-39 ��������� �������
				0,					//0x21 ��������� ������������ ���� ������� SMS
				0,					//0x22 ������ ��������� ���������� ��� �������������� ���
				0,					//0x23 ����� �������� �������� ���������
				24,					//0x24 ������ �������� �������� ��������� � �����
				8,					//0x25 ����� ������ �������� ���������
				0,					//0x26 ������ ������ � ���������� ������ 0 �� ������������ 1 SMS 2 voice
				1);					//0x27 ����� �������� ��������� � ������� ����������
__EEPROM_DATA (0,0,0,0,0,0,0,0);
__EEPROM_DATA (	2,					//0x30 ����������� �������� �� �����
				1,					//0x31 ����������� ��� �� ����� 
				0,0,0,0,0,0);
__EEPROM_DATA (0,0,0,0,0,0,0,0);
//40-59 ��������� ���
__EEPROM_DATA (	10,					//0x40 ����� �� ����
				10,					//0x41 ����� �� �����
				10,					//0x42 ����� �����������
				10,					//0x43 ����������� ��������� ������� ������� �������������� /10
				0,0,0,0);
__EEPROM_DATA 	(1,3,3,1,1,1,1,1); __EEPROM_DATA  (1,1,1,1,1,1,1,1); //0x48 ������ ���������� �� ����� �� 47 (16 ���)
__EEPROM_DATA 	(0,1,0,0,0,0,0,0); __EEPROM_DATA  (0,0,0,0,0,0,0,0); //0x58	������������  ��� �� 57  0-������� 1-�������-���������	2-��������������,3-���� �����������
										 // ������� ��� � 1 ���� �������������. � 0 ��������������
__EEPROM_DATA  (0,					//0x68 ������������ ���������� ������ 0 -���������� ������ ��������� 1 -���������� ������ ������� 2,3 - +��������     ��������� 4- ����� ������� ��� ������ � ����������
				0,					//0x69 ����� � ������� ��� �������������� �� ������ (���� 0 �� ��� ��������������)
				0,0,0,0,0,0);
//60-69 ��������� �������
__EEPROM_DATA (	0,0,0,0,0,0,0,0); __EEPROM_DATA (0,0,0,0,0,0,0,0); //0x70 ��������  ������� � ��������� ���������� �� 67 (8 �������) 
__EEPROM_DATA ( 1,1,0,0,0,0,0,0); __EEPROM_DATA (0,0,0,0,0,0,0,0); //0x80 ������������  ������� �� 6F (16 �������)  
										 						   // 0- ����� ���������� �� 1 ��� 1- �������� 2- ������������ ����������
__EEPROM_DATA (	0,0,0,0,0,0,0,0); __EEPROM_DATA (0,0,0,0,0,0,0,0); //0x90	�������� ��� � �������   0-�������� ��� ��� ����� ������ ������������� ��� ������� � ����. �������� ������ � ����������� ��������  
__EEPROM_DATA (	5,					//0xA0 ����� ������ ������
				  0,0,0,0,0,0,0); __EEPROM_DATA (0,0,0,0,0,0,0,0);
//70-79 ��������� ��������� ���������
__EEPROM_DATA  (0,0,0,0,0,0,0,0); __EEPROM_DATA (0,0,0,0,0,0,0,0); //0xB0	������������  ��� 16 ���
__EEPROM_DATA  (0,0,0,0,0,0,0,0); __EEPROM_DATA (0,0,0,0,0,0,0,0); //0xC0	������������  VOICE 16 ���
__EEPROM_DATA  (0,0,0,0,0,0,0,0); __EEPROM_DATA (0,0,0,0,0,0,0,0); //0xD0	������������  ��� 32 ��� ��� Contact ID
__EEPROM_DATA  (0,0,0,0,0,0,0,0); __EEPROM_DATA (0,0,0,0,0,0,0,0);
__EEPROM_DATA  (0,0,0,0,0,0,0,0); __EEPROM_DATA (0,0,0,0,0,0,0,0); //0xF0 ������������  VOICE 32 ��� ��� Contact ID
__EEPROM_DATA  (0,0,0,0,0,0,0,0); __EEPROM_DATA (0,0,0,0,0,0,0,0);
//80-89 ��������� �������������� �������.
__EEPROM_DATA  (0,0,0,0,0,0,0,0); __EEPROM_DATA (0,0,0,0,0,0,0,0); //0x110 ��������� ������� �������� ����������� (16 ��)0- �������� 1- ������� 2- ����� ������ 
										                           // 3- ����� ���������� 4 - ������ ����������
__EEPROM_DATA  (0,0,0,0,0,0,0,0); __EEPROM_DATA (0,0,0,0,0,0,0,0); //0x120 ��������� ������� �������� ����������� (16��) 0- �������� 1- SMS ����� 2- ��������� ����� 
__EEPROM_DATA  (5,5,5,5,5,5,5,5); __EEPROM_DATA (5,5,5,5,5,5,5,5); //0x130 ������ ��������  �������� ����������� (16��)
__EEPROM_DATA  (40,40,40,40,40,40,40,40); __EEPROM_DATA  (40,40,40,40,40,40,40,40); //0x140 ������� �������� �������� ����������� (16��)
__EEPROM_DATA  (2,2,2,2,2,2,2,2); __EEPROM_DATA (2,2,2,2,2,2,2,2); //0x150 ���������� �����������				 (16��)
//100-119 ��������� Contact ID.
__EEPROM_DATA  ('5','4','3','2','1',0,0,0); __EEPROM_DATA (0,0,0,0,0,0,0,0); //0x160  contact_id �����
__EEPROM_DATA  (0,					//0x170	��������� ����������-������ ��������� CONTACT ID 
				0,					//0x171	������ ��������� ������ ����������� ��������� � ���� ���������������� 
				0,0,0,0,0,0);		//End
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //168
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //170
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //178
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //180
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //188
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //190
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //198
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //1A0
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //1A8
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //1B0
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //1B8
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //1C0
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //1C8
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //1D0
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //1D8
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //1E0
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //1E8
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //1F0
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //1F8
__EEPROM_DATA  (1,0,0,0,0,0,0,0);   //200  ��������� ������� ���������  ���������� 
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //208
__EEPROM_DATA  (0,0,0,0,0,0,0,0);   //210