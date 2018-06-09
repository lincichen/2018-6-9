/*
*********************************************************************************************************
* Company   : ',',''''',,'╭⌒╮╭⌒╮⌒╮.','''⌒╮'',,',',╭⌒╮',,'. 
*           : ╱◥█████◣''o┈nicholasldf@yahoo.com.cn┄o','. 
* Website   : ︱田︱田田│‘'o┈www.line.com.cn┄o'',,','',,,','',,'. 
*           : ╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬
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



//接收控制
volatile unsigned int  ZmodemUsbRevCnt = 0;//缓冲区的有效数据数量
volatile unsigned int  ZmodemUsbInInx  = 0;//数据入列索引
unsigned int  ZmodemUsbOutInx = 0;//数据出列索引
//zmodem的数据接收缓冲区
char ZmodemUsbRevBuff[SYSTEM_UNIT_ZMODEM_USBREV_BUFF_SIZE];

//发送控制
volatile unsigned int  UsbTransferComplete = 0;//USB数据发送完成标志

//编程控制
//接收文件在FLASH的存放地址，对于STM32应用程序，等于0x8004000，对于字库，等于DATAFLASH的第27扇区首地址
unsigned int FlashDestination;
unsigned int ZmodemReceiveType = 0;//接收文件标识，1-STM32应用程序，2-字库，其他无效
#define size_rxbuf 16*1024
uint8_t Rx_BUF_boot[size_rxbuf];

/*
 ** zmodem的底层串口接口函数，zmputchar输出一个字节，zmputs输出一串字节，readline读取一个字节 **
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
 * zmputs display one string  打印字符串
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
			/* 缓冲区为空，则触发数据接收 */
			CLK_SysTickDelay(10);
			timeout--;
		}		

	}while(timeout);//是否有定时溢出产生

	return ZMODEMTIMEOUT;
}



/*
 *************************** zmodem接收文件功能的putsec写文件数据函数 ***************************
 */
/*
 * Putsec writes the n characters of buf to receive file buffer
 * 保存zmodem接收到的数据，保存位置可以是任何可固化数据的存储区，
 * 也可以输出到显示终端，也可以丢弃以作通信测试
 */
int putsec(char *buf, unsigned int n)
{
#if  1 //测试开关
	unsigned int j;
	unsigned int RamSource;
	if(1 == ZmodemReceiveType){
		//接收文件不能超过“FLASH空间256K - 引导程序空间15K = 应用程序空间496K，即0x40000-0x3c00=0x3c400”，否则会破坏FLASH其他数据
//		if(rxbytes + n > 0x3c400)
//			return ZMODEMERROR;
		
		RamSource = (uint32_t)buf;
		for (j = 0; j < n; j += 4){
        	//判断是否到了新的一页起始处，此时擦除STM32内部FLASH另一新页
			if(0 == (FlashDestination) % STM32_FLASH_PAGE_SIZE){
				if (FMC_Erase(FlashDestination)) {
            printf("Erase failed on 0x%x\n", FlashDestination);
            return -1;
        }
			}
			/* Program the data received into STM32F10x Flash */
			
			FMC_Write(FlashDestination, *(uint32_t*)RamSource);
      if (*(uint32_t *)FlashDestination != *(uint32_t *)RamSource){
				/* 比较失败，STM32内部FLASH字编程出错，End session */
					return ZMODEMERROR;
			}      
			//下一编程位置和下一编程数据
      FlashDestination += 4;
      RamSource += 4;
    }
	}else{
		//错误文件标识
		return ZMODEMERROR;
	}	
#endif
	return ZMODEMOK;
////		unsigned int j;
//// 		unsigned int RamSource;
////		static uint32_t FlashDestination=0x00004000;
////		RamSource = (uint32_t)buf;
////		for (j = 0; j < n; j += 4){
////        	//判断是否到了新的一页起始处，此时擦除STM32内部FLASH另一新页
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
////				/* 比较失败，STM32内部FLASH字编程出错，End session */
////					return ZMODEMERROR;
////			}
////	
////			printf("\n\r\n\r%x\n\r\n\r", *(uint32_t *)FlashDestination);
////			//下一编程位置和下一编程数据
////      FlashDestination += 4;
////      RamSource += 4;
////		
////		}
////		return ZMODEMOK;
}



