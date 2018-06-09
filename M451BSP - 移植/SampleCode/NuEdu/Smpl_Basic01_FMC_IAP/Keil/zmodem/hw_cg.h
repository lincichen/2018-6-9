#ifndef __HW_CF_H__
#define __HW_CF_H__


#include "M451Series.h"

//应用程序在STM32内部FLASH的存放地址，引导程序大小为0x4000即16K
#define ApplicationAddress    		(0x00004000)
//STM32内部Flash属性
#define STM32_FLASH_PAGE_SIZE       (2048)	/*   2K */
#define STM32_FLASH_TOTAL_SIZE      (0x40000) /* 256K */

/* zmodem接受缓冲区大小配置，尽量设置大一点，提高接收速度 */
#define  SYSTEM_UNIT_ZMODEM_USBREV_BUFF_SIZE	40*1024


//接收控制
extern volatile unsigned int  ZmodemUsbRevCnt;//缓冲区的有效数据数量
extern volatile unsigned int  ZmodemUsbInInx; //数据入列索引
extern          unsigned int  ZmodemUsbOutInx;//数据出列索引

//用于shell或者zmodem的数据接收缓冲区
extern char ZmodemUsbRevBuff[SYSTEM_UNIT_ZMODEM_USBREV_BUFF_SIZE];


//发送控制
extern volatile unsigned int  UsbTransferComplete;//USB数据发送完成标志
//程序升级控制
extern unsigned int ZmodemReceiveType;//接收文件标识，1-STM32应用程序，2-字库，其他无效
extern unsigned int FlashDestination;//接收文件在FLASH的存放地址

//flash
int readline(unsigned int timeout);
int putsec(char *buf, unsigned int n);
//receive
void ackbibi(void);
int TryZmodem(void);
int procheader(char *name);
int ZmodemReceiveOneFile(void);
void ZmodemReceiveFiles(void);

//6x8和8x8英文字符字库			 
extern unsigned char GUI_acFont6x8[][8];
extern unsigned char GUI_F8x8_acFont[][8];
#endif

