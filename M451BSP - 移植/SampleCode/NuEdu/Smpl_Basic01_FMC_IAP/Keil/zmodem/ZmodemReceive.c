/*
*********************************************************************************************************
* Company   : ',',''''',,'╭⌒╮╭⌒╮⌒╮.','''⌒╮'',,',',╭⌒╮',,'. 
*           : ╱◥█████◣''o┈nicholasldf@yahoo.com.cn┄o','. 
* Website   : ︱田︱田田│‘'o┈www.line.com.cn┄o'',,','',,,','',,'. 
*           : ╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬╬
* Copyright : All Rights Reserved
* Product   : 
* File      : ZmodemReceive.c
* Version   : V1.00
* Author    : Chuck Forsberg, port by nicholasldf
*********************************************************************************************************
*/
/*
 * rz.c By Chuck Forsberg
 *    Copyright 2006 Omen Technology Inc All Rights Reserved
 *
 * A program for Unix to receive files and commands from computers running
 *  Professional-YAM, PowerCom, YAM, IMP, or programs supporting XMODEM.
 *  rz uses Unix buffered input to reduce wasted CPU time.
 *
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "M451Series.h"
#include "zmodem.h"
#include "hw_cg.h"
#include "uart.h"

extern int IsDebugFifoEmpty(void);

					   		  


unsigned int rxbytes = 0;//接收到的总字节数
/* Attention string rx sends to tx on err */
char AttnOfRx[ZATTNLEN+1] = {"Attn Of Rx"};
char ReceiveBuf[1028];//zmodem接收通信缓冲区


/*
 * Ack a ZFIN packet, let byegones be byegones
 */
void ackbibi(void)
{
	int n;

	stohdr(0L);
	for (n=3; --n>=0; ) {
		zshhdr(4, ZFIN, Txhdr);
		switch (readline(TIMEOUT_WHEN_READ_CHAR)) {
		case 'O':
			readline(TIMEOUT_WHEN_READ_CHAR);/* Discard 2nd 'O' */
			return;
		case ZMODEMRCDO:
			return;
		case ZMODEMTIMEOUT:
		default:
			break;
		}
	}
} 


/*
  Initialize for Zmodem receive attempt,  //对Zmodem接受尝试初始化，
	try to activate Zmodem sender 					//尝试激活Zmodem发送器（本身作为接收器）
  Handles ZSINIT frame										//处理ZSINIT框架
  Return ZFILE if Zmodem filename received, -1 on error,//返回ZFILE，如果ZMODEM文件名接受，-1 在接受上
  ZCOMPL if transaction finished,  else 0			//如果传输结束ZCOMPL，不然为0
 */
int TryZmodem(void)
{
	int c, n;
	int m;

	for (n=2; --n>=0; ) {//n=15
		/* Set buffer length (0) and capability flags */
		stohdr(0L);
		Txhdr[ZF0] = CANFC32|CANFDX|CANOVIO;
		if (Zctlesc)
			Txhdr[ZF0] |= TESCCTL;

		/* tryzhdrtype may == ZRINIT */
		zshhdr(4,ZRINIT, Txhdr);//
		//zsbhdr(4,ZRINIT, Txhdr);//二进制分析
//		zsbhdr_test(4,ZRINIT, Txhdr);
TryAgain:
		switch (zgethdr(Rxhdr)) {
		case ZRQINIT: 
			continue;
		case ZEOF:  
			continue;
		case ZMODEMTIMEOUT: 
			continue;
		case ZFILE:
			//仅仅查看ZFILE选项
			m = Rxhdr[ZF0];
			m = Rxhdr[ZF1];
			m = Rxhdr[ZF2];
			//接收文件信息数据：文件名、文件大小。。。
			c = zrdata(ReceiveBuf, 1024);
			if (c == GOTCRCW)
				return ZFILE;
			zshhdr(4,ZNAK, Txhdr);
			goto TryAgain;
		case ZSINIT:
			Zctlesc = TESCCTL & Rxhdr[ZF0];
			if (zrdata(AttnOfRx, ZATTNLEN) == GOTCRCW) {
				stohdr(1L);
				zshhdr(4,ZACK, Txhdr);
				goto TryAgain;
			}
			zshhdr(4,ZNAK, Txhdr);
			goto TryAgain;
		case ZFREECNT:
			//zmputs("\n 12345648798465 \n");
			stohdr(0x7FFFFFFF);//many
			zshhdr(4, ZACK, Txhdr);
			goto TryAgain;
		case ZCOMMAND:
			if (zrdata(ReceiveBuf, 1024) == GOTCRCW) {
				//no action, just ack
				stohdr(0L);
				m = 0;
				do {
					zshhdr(4, ZCOMPL, Txhdr);
				}
				while (++m<20 && zgethdr(Rxhdr) != ZFIN);
				ackbibi();
				return ZCOMPL;
			}else{
				zshhdr(4,ZNAK, Txhdr); goto TryAgain;
			}
		case ZCOMPL:
			goto TryAgain;
		case ZFIN:
			ackbibi(); return ZCOMPL;
		case ZCAN:
			return ZMODEMERROR;
		default:
			continue;
		}
	}
	return ZMODEMERROR;
}


/*
 * Process incoming file information header 处理传入的文件信息头
 *  Returns 0 for success, other codes for errors 成功返回0，其他返回error
 *  or skip conditions. 或者跳过条件
 */
int procheader(char *name)
{
	
	if (!name || !*name)
		return ZMODEMERROR;
	if(0 == strcmp("fmc_ap1_main.bin", name))
//if(0 == strcmp("fmc_ld_boot.bin", name))		
		{
	    
		ZmodemReceiveType = 1;
		/* Initialize FlashDestination variable */
  		FlashDestination = ApplicationAddress;//应用程序地址0x00003c00	
	}else if(0 == strcmp("xxxxx.xxx", name)){
	    //这里可以添加其他类型的烧录选项，，，如果是xxxxx
		ZmodemReceiveType = 2;
	
	}else{
		/* 复位接收文件标识 */
		ZmodemReceiveType = 0;
		return ZMODEMERROR;
	}

	return ZMODEMOK;
}

/*
 * Receive a file with ZMODEM protocol
 * Assumes file name frame is in ReceiveBuf
 */
int ZmodemReceiveOneFile(void)
{
	int c, n;

	if (ZMODEMERROR == procheader(ReceiveBuf)) {
		
		return ZMODEMERROR;//接收到文件名不匹配
	}

	n = 100; rxbytes = 0;
	for (;;) {
		stohdr(rxbytes);
		zshhdr(4, ZRPOS, Txhdr);
nxthdr:
		switch (c = zgethdr(Rxhdr)) {
		default:
			//vfile("rzfile: Wrong header %d", c);
			if ( --n < 0){
				return ZMODEMERROR;
			}else{
				continue;
			}
		case ZCAN:
			//sprintf(endmsg, "Sender CANcelled");
			return ZMODEMERROR;
		case ZNAK:
			//sprintf(endmsg, "rzfile: got ZNAK", c);
			if ( --n < 0){
				return ZMODEMERROR;
			}else{
				continue;
			}
		case ZMODEMTIMEOUT:
			//sprintf(endmsg, "rzfile: ZMODEMTIMEOUT", c);
			if ( --n < 0){
				return ZMODEMERROR;
			}else{
				continue;
			}
		case ZFILE:
			zrdata(ReceiveBuf, 1024);
			continue;
		case ZEOF:
			if (rclhdr(Rxhdr) != rxbytes) {
				/*
				 * Ignore eof if it's at wrong place - force
				 *  a timeout because the eof might have gone
				 *  out before we sent our zrpos.
				 */
				goto nxthdr;
			}else{
				//vfile("rzfile: normal EOF");
				return ZEOF;
			}
		case ZMODEMERROR:	/* Too much garbage in header search error */
			//sprintf(endmsg, "Persistent CRC or other ZMODEMERROR");
			if ( --n < 0){
				return ZMODEMERROR;
			}else{
				zmputs(AttnOfRx);
				continue;
			}
		case ZSKIP:
			//putsec(ReceiveBuf, chinseg);
			//chinseg = 0;
			//sprintf(endmsg, "Sender SKIPPED file");
			return ZSKIP;
//数据帧头
		case ZDATA:
			if (rclhdr(Rxhdr) != rxbytes) {
				//sprintf(endmsg,"Data has bad addr");
				if ( --n < 0){
					return ZMODEMERROR;
				}else{
					zmputs(AttnOfRx);  continue;
				}
			}
//更多的数据子包
moredata:
			switch (c = zrdata(ReceiveBuf, 1024)){
			case ZCAN:
				//sprintf(endmsg, "Sender CANcelled");
				return ZMODEMERROR;
			case ZMODEMERROR:	/* CRC error */
				//sprintf(endmsg, "Persistent CRC or other ZMODEMERROR");
				if ( --n < 0){
					return ZMODEMERROR;
				}else{
					zmputs(AttnOfRx);
					continue;
				}
			case ZMODEMTIMEOUT:
				//sprintf(endmsg, "ZMODEMTIMEOUT");
				if ( --n < 0){
					return ZMODEMERROR;
				}else{
					continue;
				}
			case GOTCRCW:
				n = 100;
				c = putsec(ReceiveBuf, Rxcount);
				if(ZMODEMERROR == c)//写数据出错则终止zmodem会话
					return ZMODEMERROR;
				rxbytes += Rxcount;
				stohdr(rxbytes);
				sendline(XON);
				zshhdr(4, ZACK, Txhdr);
				goto nxthdr;
			case GOTCRCQ:
				n = 100;
				c = putsec(ReceiveBuf, Rxcount);
				if(ZMODEMERROR == c)//写数据出错则终止zmodem会话
					return ZMODEMERROR;
				rxbytes += Rxcount;
				stohdr(rxbytes);
				zshhdr(4, ZACK, Txhdr);
				goto moredata;
			case GOTCRCG:
				n = 100;
				c = putsec(ReceiveBuf, Rxcount);
				if(ZMODEMERROR == c)//写数据出错则终止zmodem会话
					return ZMODEMERROR;
				rxbytes += Rxcount;
				goto moredata;
			case GOTCRCE:
				n = 100;
				c = putsec(ReceiveBuf, Rxcount);
				if(ZMODEMERROR == c)//写数据出错则终止zmodem会话
					return ZMODEMERROR;
				rxbytes += Rxcount;
				goto nxthdr;
			}//switch (c = zrdata(ReceiveBuf, 1024))
		}//switch (c = zgethdr(Rxhdr))
	}//for (;;) 
}


/*
 * Receive 1 or more files with ZMODEM protocol
 */
void ZmodemReceiveFiles(void)
{
	unsigned int ReceiveSuccessfulFlag;
	unsigned int RightCnt = 0;
//	char *pNote;

//	pNote = 0;
//	RightCnt = 0;
	ReceiveSuccessfulFlag = 0;
	for(RightCnt=0;RightCnt<=1;RightCnt++)
	{
		//尝试接收文件
		zmputs("\n\r\n\r--- please use HyperTerminal to send a file ---\n\r\n\r");
		CLK_SysTickDelay(200000);
		if(ZFILE == TryZmodem()){			
			switch(ZmodemReceiveOneFile())
			{
			case ZEOF://write dataflash
				ReceiveSuccessfulFlag = 1;
				
				break;
			default:
				ReceiveSuccessfulFlag = 0;
				printf("ZmodemReceiveOneFile = %d",ZmodemReceiveOneFile());
				canit();
				break;
			}
		}
		
		
		
		CLK_SysTickDelay(200000);
		CLK_SysTickDelay(200000);
		CLK_SysTickDelay(200000);
		
		
		if(1 == ReceiveSuccessfulFlag && 1 == ZmodemReceiveType){
				
		   zmputs("\n\r\n\rM451 app update ok\n\r\n\r");//超级终端显示 pNote = "\n\r\n\rstm32 app update ok\n\r\n\r";

			 //break;
		}else{
			//pNote = "\n\r\n\r--- please use HyperTerminal to send a file ---\n\r\n\r";
		}	
	}//while

}
 
	  

/* End of rz.c */ 

 					 
