#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "M451Series.h"
//#include "zmodem.h"
#include "hw_cg.h"
#include "uart.h"
#define ZSEND 1
#ifdef ZSEND

extern char Rxhdr[ZMAXHLEN];	/*  接收的包头 */
extern char Txhdr[ZMAXHLEN];	/*  发送的包头 */


unsigned char Filename[] = "test.txt";
unsigned char length[] = "11";//"hello world"
unsigned char UnixData[] = "13303720100";
unsigned char mode = '0';
unsigned char SNNumber = '0';
unsigned char File_left = '1';
unsigned char Byte_left[] = "11";

int Send_ZRQINT(void)
{
	int  n = 0;
	//for (n=2; --n>=0; ) 
//	{//n=15
		/* Set buffer length (0) and capability flags */
		
		stohdr(0L);
		Txhdr[ZF3] = 010;//CANFC32|CANFDX|CANOVIO;
		/* tryzhdrtype may == ZRINIT */
		sendline(0x72); //'r'
		sendline(0x7a); //'z'
		sendline(0x0d); //'\n'
		CLK_SysTickDelay(10000);//10ms
		zshhdr(4,ZRQINIT, Txhdr);//ZRQINIT 在发送字节的第八和第九个字节（ZRQINIT）
//	}
	return 0;
}

/*Send a file with ZMODEM protocol
 *文件名称框架在发送缓冲区中
 */
int ZmodemSendOneFile(void)
{

}

//开始文件传输
int StartSending(void)//updcrc
{
	unsigned char Data[21] = 0;
	Send_ZRQINT();
	//zgethdr(Rxhdr);
	CLK_SysTickDelay(200000);
	//UART_Read(UART0,Data, 20);//读取系统返回的ZRINIT（其他命令暂时没有解析）
	#if 0
	if(Data[5] == (ZRINIT+0x30))
		printf("receive ZRINIT\r\n");
	
	#endif	
//	printf("%s",Data);
	//CLK_SysTickDelay(200000);
		return 0;
}
#if 0
int Send_ZFILE(void)
{		
		unsigned int i;
		unsigned int crc = 0;
		stohdr(0L);
		//Txhdr[ZF3] = 010;//CANFC32|CANFDX|CANOVIO;
		//crc = updcrc((0377 & *hdr), crc);
		zsbhdr(4,ZFILE, Txhdr);					//ZFILE文件头
	
		for(i=0;i<strlen(Filename);i++) //文件名
			{
				xsendline(Filename[i]);
				crc = Z_UpdateCRC((0377 &Filename[i]), crc);
			}
		//xsendline('\0');
		crc = Z_UpdateCRC('\0', crc);
			
		for(i=0;i<strlen(length);i++) 	//文件长度
			{
				xsendline(length[i]);
				crc = Z_UpdateCRC((0377 &length[i]), crc);
			}
			
		xsendline(0x20);
		crc = Z_UpdateCRC(0x20, crc);	
			
		for(i=0;i<strlen(UnixData);i++)//unix日期
			{
				xsendline(UnixData[i]);
				crc = Z_UpdateCRC((0377 &UnixData[i]), crc);
			}
			
		xsendline(0x20);
		crc = Z_UpdateCRC(0x20, crc);
			
		xsendline(mode);						 	//模式
		crc = Z_UpdateCRC(mode, crc);
			
		xsendline(0x20);
		crc = Z_UpdateCRC(0x20, crc);
			
		xsendline(SNNumber);				 	//SN编号
		crc = Z_UpdateCRC(SNNumber, crc);
			
		xsendline(0x20);
		crc = Z_UpdateCRC(0x20, crc);
			
		xsendline(File_left);				 	//剩余文件数目
		crc = Z_UpdateCRC(File_left, crc);
		
		xsendline(0x20);
		crc = Z_UpdateCRC(0x20, crc);
			
		for(i=0;i<strlen(Byte_left);i++)
			{		
				xsendline(Byte_left[i]);				 //剩余字节数目
				crc = Z_UpdateCRC((0377 &Byte_left[i]), crc);
			}
		
		xsendline('\0');	
		crc = Z_UpdateCRC('\0', crc);
			
		xsendline(ZDLE);
		crc = Z_UpdateCRC(ZDLE, crc);
			
		xsendline(ZCRCW);
		crc = Z_UpdateCRC(ZCRCW, crc);	
		
		//crc = Z_UpdateCRC(0,Z_UpdateCRC(0,crc));//两次
		xsendline(crc>>8);
		xsendline((unsigned char)(crc&0xff));	
		//xsendline(0xa4);		//校验
		//xsendline(0x16);		//			
		xsendline(0x11);								//XON最后一个字符
			
		return 0;
}
#endif

int modify = 40;
void ZS_SendFileName(unsigned char *name, int numK)//1
{
	unsigned int crc = 0;
	unsigned char str[] = "00000000 000 0\0", *p;
	unsigned lenB = numK ;//* 1024;//1024

	p = str;
	while (lenB >= 100000) { lenB -= 100000; (*p)++; } if (p != str) p++; else if (*p != '0') p++;
	while (lenB >=  10000) { lenB -=  10000; (*p)++; } if (p != str) p++; else if (*p != '0') p++;
	while (lenB >=   1000) { lenB -=   1000; (*p)++; } if (p != str) p++; else if (*p != '0') p++;
	while (lenB >=    100) { lenB -=    100; (*p)++; } if (p != str) p++; else if (*p != '0') p++;
	while (lenB >=     10) { lenB -=     10; (*p)++; } if (p != str) p++; else if (*p != '0') p++;
	while (lenB >=      1) { lenB -=      1; (*p)++; } p++; *p++ = ' ';

	modify++;
	while (modify >= 100) { modify -= 100; (*p)++; }  p++;
	while (modify >=  10) { modify -=  10; (*p)++; }  p++;//41
	while (modify >=   1) { modify -=   1; (*p)++; }  p++;
	*p++ = ' ';
	*p++ = '0'; 
	*p = '\0';
	
	for (;;)
	{
		xsendline(*name);
		crc = Z_UpdateCRC(((unsigned short)*name), crc);
		if (*name) ;
		else break;
		name++;
	}
	p = str;
	for (;;)
	{
		xsendline(*p);
		crc = Z_UpdateCRC(((unsigned short)*p), crc);
		if (*p) ; else break;
		p++;
	}
	xsendline(ZDLE);
	xsendline(ZCRCW);
	crc = Z_UpdateCRC(ZCRCW, crc);

	crc = Z_UpdateCRC(0, crc);
	crc = Z_UpdateCRC(0, crc);
	xsendline(crc >> 8);
	xsendline(crc);

//	SENDBYTE(XON);
}

int Transmission_of_Send(void)
{
	stohdr(0L);
		//Txhdr[ZF3] = 010;//CANFC32|CANFDX|CANOVIO;
		//crc = updcrc((0377 & *hdr), crc);
	zsbhdr(4,ZFILE, Txhdr);					//ZFILE文件头
	
	ZS_SendFileName(Filename,11);//发送ZFILE帧 等待返回一个ZRPOS 
	return 0;
}

/*--------------------------------------------------------------------------*/
/* ZS SEND FILE                                                             */
/* Send ZFILE frame and begin sending ZDATA frame                           */
/*--------------------------------------------------------------------------*/
int ZS_SendFile(unsigned char *name, int numK)
{	int c;

	for (;;)
	{
		Txhdr[ZF0] = 0;    /* Default file conversion mode */
		Txhdr[ZF1] = 0;   /* Default file management mode */
		Txhdr[ZF2] = 0;   /* Default file transport mode */
		Txhdr[ZF3] = 0;
		zsbhdr(4,ZFILE, Txhdr);
		ZS_SendFileName(name, numK);

Again:
		switch (c = zgethdr(Rxhdr))
		{
			case ZRINIT:	/*-----------------------------------------*/
							/*                                         */
							/*-----------------------------------------*/
				goto Again;

			case ZCAN:     
			case ZCRC:     
			case ZZRCDO:     
			case ZZTIMEOUT:  
			case ZFIN:
			case ZABORT:   
							/*-----------------------------------------*/
							/*                                         */
							/*-----------------------------------------*/
				return ZZERROR;
   
			case ZSKIP:	    /*-----------------------------------------*/
							/* Other system wants to skip this file    */
							/*-----------------------------------------*/
				 return c;
   
			case ZRPOS:	    /*-----------------------------------------*/
							/* Resend from this position...            */
							/*-----------------------------------------*/
				Txpos = Rxpos;
				return ZS_SendFileData(numK * 1024L, 1024);
		}
	}
}


/*
 * Send 1 or more files with ZMODEM protocol
 */
void ZmodemSendFiles(void)
{
	unsigned int SendSuccessfulFlag;
	unsigned int RightCnt = 0;
	SendSuccessfulFlag = 0;
	StartSending();
	Transmission_of_Send();//
	
}
#endif
