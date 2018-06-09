/*
*********************************************************************************************************
* Company   : ',',''''',,'�q�Шr�q�Шr�Шr.','''�Шr'',,',',�q�Шr',,'. 
*           : �u��������������''o��nicholasldf@yahoo.com.cn��o','. 
* Website   : ��������辶��'o��www.line.com.cn��o'',,','',,,','',,'. 
*           : �p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p�p
* Copyright : All Rights Reserved
* Product   : 
* File      : FlashProgrammer.c
* Version   : V1.00
* Author    : Chuck Forsberg, port by nicholasldf
*********************************************************************************************************
*/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "M451Series.h"
#include "zmodem.h"
#include "uart.h"
#include "time.h"
#include "hw_cg.h"



//���տ���
volatile unsigned int  ZmodemUsbRevCnt = 0;//����������Ч��������
volatile unsigned int  ZmodemUsbInInx  = 0;//������������
unsigned int  ZmodemUsbOutInx = 0;//���ݳ�������
//zmodem�����ݽ��ջ�����
char ZmodemUsbRevBuff[SYSTEM_UNIT_ZMODEM_USBREV_BUFF_SIZE];

//���Ϳ���
volatile unsigned int  UsbTransferComplete = 0;//USB���ݷ�����ɱ�־

//��̿���
//�����ļ���FLASH�Ĵ�ŵ�ַ������STM32Ӧ�ó��򣬵���0x8004000�������ֿ⣬����DATAFLASH�ĵ�27�����׵�ַ
unsigned int FlashDestination;
unsigned int ZmodemReceiveType = 0;//�����ļ���ʶ��1-STM32Ӧ�ó���2-�ֿ⣬������Ч
#define size_rxbuf 16*1024
uint8_t Rx_BUF_boot[size_rxbuf];

/*
 ** zmodem�ĵײ㴮�ڽӿں�����zmputchar���һ���ֽڣ�zmputs���һ���ֽڣ�readline��ȡһ���ֽ� **
 */
 
/*
 * zmputchar display one char
 */
void zmputchar(char c)
{
	uint8_t data;

	data = c;
	UART_Write(UART0,&data,1);
}

/*
 * zmputs display one string  ��ӡ�ַ���
 */
void zmputs(char *str)
{
	if(0 == str) return;
	UART_Write(UART0,(uint8_t *)str,strlen(str));					 

}
#if 0//test
void zmgetchar(char c)
{
	if(0 == c) return;
	UART_Write(UART0,(uint8_t *)c,strlen(c));					 

}
#endif
/*
 * readline read one char with timeout
 */
int readline(unsigned int timeout)
{
	uint8_t temp;
	
	do{
		if(UART_Read(UART0,&temp, 1)){
			return temp & 0x000000FF;
		}else{
			/* ������Ϊ�գ��򴥷����ݽ��� */
			CLK_SysTickDelay(10);
			timeout--;
		}		

	}while(timeout);//�Ƿ��ж�ʱ�������

	return ZMODEMTIMEOUT;
}



/*
 *************************** zmodem�����ļ����ܵ�putsecд�ļ����ݺ��� ***************************
 */
/*
 * Putsec writes the n characters of buf to receive file buffer
 * ����zmodem���յ������ݣ�����λ�ÿ������κοɹ̻����ݵĴ洢����
 * Ҳ�����������ʾ�նˣ�Ҳ���Զ�������ͨ�Ų���
 */
int putsec(char *buf, unsigned int n)
{
#if  1 //���Կ���
	unsigned int j;
	unsigned int RamSource;
	if(1 == ZmodemReceiveType){
		//�����ļ����ܳ�����FLASH�ռ�256K - ��������ռ�15K = Ӧ�ó���ռ�496K����0x40000-0x3c00=0x3c400����������ƻ�FLASH��������
//		if(rxbytes + n > 0x3c400)
//			return ZMODEMERROR;
		
		RamSource = (uint32_t)buf;
		for (j = 0; j < n; j += 4){
        	//�ж��Ƿ����µ�һҳ��ʼ������ʱ����STM32�ڲ�FLASH��һ��ҳ
			if(0 == (FlashDestination) % STM32_FLASH_PAGE_SIZE){
				if (FMC_Erase(FlashDestination)) {
            printf("Erase failed on 0x%x\n", FlashDestination);
            return -1;
        }
			}
			/* Program the data received into STM32F10x Flash */
			
			FMC_Write(FlashDestination, *(uint32_t*)RamSource);
      if (*(uint32_t *)FlashDestination != *(uint32_t *)RamSource){
				/* �Ƚ�ʧ�ܣ�STM32�ڲ�FLASH�ֱ�̳���End session */
					return ZMODEMERROR;
			}      
			//��һ���λ�ú���һ�������
      FlashDestination += 4;
      RamSource += 4;
    }
	}else{
		//�����ļ���ʶ
		return ZMODEMERROR;
	}	
#endif
	return ZMODEMOK;
////		unsigned int j;
//// 		unsigned int RamSource;
////		static uint32_t FlashDestination=0x00004000;
////		RamSource = (uint32_t)buf;
////		for (j = 0; j < n; j += 4){
////        	//�ж��Ƿ����µ�һҳ��ʼ������ʱ����STM32�ڲ�FLASH��һ��ҳ
////			if(0 == (FlashDestination) % STM32_FLASH_PAGE_SIZE){
////				if (FMC_Erase(FlashDestination)) {
////            printf("Erase failed on 0x%x\n", FlashDestination);
////            return -1;
////        }
////			}
////			/* Program the data received into STM32F10x Flash */
////			
////			FMC_Write(FlashDestination, *(uint32_t*)RamSource);
////      if (*(uint32_t *)FlashDestination != *(uint32_t *)RamSource){
////				/* �Ƚ�ʧ�ܣ�STM32�ڲ�FLASH�ֱ�̳���End session */
////					return ZMODEMERROR;
////			}
////	
////			printf("\n\r\n\r%x\n\r\n\r", *(uint32_t *)FlashDestination);
////			//��һ���λ�ú���һ�������
////      FlashDestination += 4;
////      RamSource += 4;
////		
////		}
////		return ZMODEMOK;
}



