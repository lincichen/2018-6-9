#ifndef __HW_CF_H__
#define __HW_CF_H__


#include "M451Series.h"

//Ӧ�ó�����STM32�ڲ�FLASH�Ĵ�ŵ�ַ�����������СΪ0x4000��16K
#define ApplicationAddress    		(0x00004000)
//STM32�ڲ�Flash����
#define STM32_FLASH_PAGE_SIZE       (2048)	/*   2K */
#define STM32_FLASH_TOTAL_SIZE      (0x40000) /* 256K */

/* zmodem���ܻ�������С���ã��������ô�һ�㣬��߽����ٶ� */
#define  SYSTEM_UNIT_ZMODEM_USBREV_BUFF_SIZE	40*1024


//���տ���
extern volatile unsigned int  ZmodemUsbRevCnt;//����������Ч��������
extern volatile unsigned int  ZmodemUsbInInx; //������������
extern          unsigned int  ZmodemUsbOutInx;//���ݳ�������

//����shell����zmodem�����ݽ��ջ�����
extern char ZmodemUsbRevBuff[SYSTEM_UNIT_ZMODEM_USBREV_BUFF_SIZE];


//���Ϳ���
extern volatile unsigned int  UsbTransferComplete;//USB���ݷ�����ɱ�־
//������������
extern unsigned int ZmodemReceiveType;//�����ļ���ʶ��1-STM32Ӧ�ó���2-�ֿ⣬������Ч
extern unsigned int FlashDestination;//�����ļ���FLASH�Ĵ�ŵ�ַ

//flash
int readline(unsigned int timeout);
int putsec(char *buf, unsigned int n);
//receive
void ackbibi(void);
int TryZmodem(void);
int procheader(char *name);
int ZmodemReceiveOneFile(void);
void ZmodemReceiveFiles(void);

//6x8��8x8Ӣ���ַ��ֿ�			 
extern unsigned char GUI_acFont6x8[][8];
extern unsigned char GUI_F8x8_acFont[][8];
#endif

