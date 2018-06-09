/****************************************************************
Copyright(c) 2005 Tsinghua Tongfang Optical Disc Co., Ltd. 
Developed by Storage Product Department, Beijing, China 
*****************************************************************
Filename: zmodem.c
Description: 
****************************************************************/

//#include "arm.h"
//#include "global.h"
//#include "rs232.h"
//#include "timer.h"
#include "zmodem.h"
#include"string.h"
#include<stdbool.h>
#include "M451Series.h"
#include<stdio.h>
#define WAZOOMAX        8192           /* Max packet size (WaZOO)           */


#define SENDBYTE(c) zmputchar(c)//SendChar(1,c)
#define SendHEX(a,b) zmputchar(b) //chuck×ÔÐÐ¶¨Òå
#define Z_GetByte(a) readline(a)
#define TEXT_DATA  "hello world"
/*--------------------------------------------------------------------------*/
/* Private data                                                             */
/*--------------------------------------------------------------------------*/

#define ZATTNLEN 32              /* Max length of attention string          */

BYTE  Attn[ZATTNLEN+1]; /* String rx sends to tx on err            */        
char  Zconv;            /* ZMODEM file conversion request          */
BYTE  Recbuf[64];       /* Pointer to receive buffer               */
BYTE  *FileBuf;         /* Pointer to received file buffer         */
int   Rxtype;           /* Type of header received                 */
int   Rxframeind;       /* ZBIN ZBIN32,ZHEX type of frame received */
int   Tryzhdrtype;      /* Hdr type to send for Last rx close      */
int   RxCount;          /* Count of data bytes received            */
char  Rxhdr[4];         /* Received header                                  */
char  Txhdr[4];         /* Transmitted header                               */
long  Rxpos;            /* Received file position         ½ÓÊÜÎÄ¼þµÄÎ»ÖÃ                  */
DWORD  rxbytes;          /* bytes of data received                           */

BOOL bTimeOut=FALSE;
/*--------------------------------------------------------------------------*/
/* Z GET BYTE       »òÐíÓ¦¸Ã¸ü»»Îª readline                                                       */
/* Get a byte from the modem.                                               */ //?modem???????
/* return TIMEOUT if no read within timeout tenths,                         */
/*--------------------------------------------------------------------------*/
#if 0
BYTE Z_GetByte(int tenths)
{
  bTimeOut = FALSE;
  tenths = tenths / 10;
  SetSysTimer(tenths);
  while (GetSysTimer()) { if (Z_TestOneByte())  return Z_GetOneByte(); }
  bTimeOut = TRUE;
  return '\xff';
}
#endif

void Z_ClearByte()
{
	memset(Attn,0,sizeof(Attn));
	memset(Recbuf,0,sizeof(Recbuf));
}


int Z_TestOneByte(){return 0;}

/*--------------------------------------------------------------------------*/
/* Z TIMED READ                                                             */
/* Read a character from the modem line with timeout.                       */  //?modem???????with timeout
/*  Eat parity, XON and XOFF characters.                                    */
/*--------------------------------------------------------------------------*/
BYTE _Z_TimedRead()
{
  for (;;) {
    BYTE c = Z_GetByte(30000); if (bTimeOut) return c;
    if ((c & 0x7f) == XON) ;
    else if ((c & 0x7f) == XOFF) ;
    else return c;
    }
}

/*--------------------------------------------------------------------------*/
/* Z UPDATE CRC                                                             */
/* update CRC                                                               */   //??CRC
/*--------------------------------------------------------------------------*/
#ifdef CRCTABLE
/* crctab calculated by Mark G. Mendel, Network Systems Corporation */
static WORD crctab[256] = {                                                      //CRC? 
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
};

#define Z_UpdateCRC(cp, crc) ( crctab[((crc >> 8) & 255)] ^ (crc << 8) ^ cp)

#else

WORD Z_UpdateCRC(WORD c, WORD crc)                           //??CRC(Z_UpdateCRC)
{ int count;

  for (count=8; --count>=0; ) {
    if (crc & 0x8000) {
      crc <<= 1;
      crc += (((c<<=1) & 0400)  !=  0);
      crc ^= 0x1021;
      }
    else {
      crc <<= 1;
      crc += (((c<<=1) & 0400)  !=  0);
      }
    }
  return crc;
}
#endif



/* Send a byte as two hex digits */
static BYTE hex[] = "0123456789abcdef";
#define Z_PUTHEX(i,c) {i=(c);SENDBYTE(hex[((i)&0xF0)>>4]);SENDBYTE(hex[(i)&0xF]);}

/*--------------------------------------------------------------------------*/
/* Z SEND HEX HEADER                                                        */
/* Send ZMODEM HEX header hdr of type type                                  */   
/*--------------------------------------------------------------------------*/   //??zmodem……
void Z_SendHexHeader(BYTE type,char *hdr)                                        //Z_SendHexHeader
{ int n; BYTE i; WORD crc;

  SENDBYTE(ZPAD);
  SENDBYTE(ZPAD);
  SENDBYTE(ZDLE);
  SENDBYTE(ZHEX);
  Z_PUTHEX(i,type);

  crc = Z_UpdateCRC(type, 0);
  for (n=4; --n >= 0;) {
    Z_PUTHEX(i,(*hdr));
    crc = Z_UpdateCRC(((unsigned short)(*hdr++)), crc);
    }
  crc = Z_UpdateCRC(0,crc);
  crc = Z_UpdateCRC(0,crc);
  Z_PUTHEX(i,(crc>>8));
  Z_PUTHEX(i,crc);

  /* Make it printable on remote machine */                                //????????????
  SENDBYTE('\x0d'); 
	SENDBYTE('\x0a');

  /* Uncork the remote in case a fake XOFF has stopped data flow */    
  if (type != ZFIN) SENDBYTE(021);
}

/*--------------------------------------------------------------------------*/
/* Z GET ZDL                                                                */
/* Read a byte, checking for ZMODEM escape encoding                         */
/* including CAN*5 which represents a quick abort                           */
/* 0x10,0x11,0x13,0x90,0x91,0x93 & 0x18 is special treatment                */
/*--------------------------------------------------------------------------*/
int Z_GetZDL(void)                                                      //Z_GetZDL……
{ int c;

  c = Z_GetByte(1000); if (bTimeOut) return ZZTIMEOUT;
  if (c != ZDLE) return c;

  c = Z_GetByte(1000); if (bTimeOut) return ZZTIMEOUT;
  switch (c) {
    case CAN:
      c = Z_GetByte(1000); if (bTimeOut) return ZZTIMEOUT;
      if (c != CAN) return c;
      c = Z_GetByte(1000); if (bTimeOut) return ZZTIMEOUT;
      if (c != CAN) return c;
      c = Z_GetByte(1000); if (bTimeOut) return ZZTIMEOUT;
      if (c != CAN) return c;
      return (GOTCAN);
    case ZCRCE:
    case ZCRCG:
    case ZCRCQ:
    case ZCRCW: return (c | GOTOR);
    case ZRUB0: return 0x7F;
    case ZRUB1: return 0xFF;
    default:
      if ((c & 0x60) == 0x40) return c ^ 0x40;
      else return ZZERROR;
    }
}

/*--------------------------------------------------------------------------*/
/* Z GET BINARY HEADER                                                      */
/* Receive a binary style header (type and position)                        */   //????????style header(?????)
/*--------------------------------------------------------------------------*/
int _Z_GetBinaryHeader(char *hdr)                         //_Z_GetBinaryHeader“?”
{ int c,n; WORD crc;

  if ((c   = Z_GetZDL()) & ~0xFF)   return c;
  Rxtype   = c;
  crc      = Z_UpdateCRC(c, 0);

  for (n=4; --n >= 0;) {
    if ((c = Z_GetZDL()) & ~0xFF) return c;
    crc = Z_UpdateCRC(c, crc);
    *hdr++ = c;
    }
  if ((c   = Z_GetZDL()) & ~0xFF) return c;
  crc      = Z_UpdateCRC(c, crc);
  if ((c   = Z_GetZDL()) & ~0xFF) return c;

  crc = Z_UpdateCRC(c, crc);
  if (crc & 0xFFFF) return ZZERROR;     //CRC error

  return Rxtype;
}

/*--------------------------------------------------------------------------*/
/* Z GET HEX                                                                */
/* Decode two lower case hex digits into an 8 bit byte value                */     
/*--------------------------------------------------------------------------*/
int _Z_GetHex(void)                                                 //_Z_GetHex“?”
{ int ch,cl;

  ch = _Z_TimedRead(); if (bTimeOut) return ZZTIMEOUT;
  ch -= '0';
  if (ch > 9) ch -= ('a' - ':');
  if (ch & 0xf0) return ZZERROR;

  cl = _Z_TimedRead(); if (bTimeOut) return ZZTIMEOUT;
  cl -= '0';
  if (cl > 9) cl -= ('a' - ':');
  if (cl & 0xf0) return ZZERROR;

  return ((ch << 4) | cl);
}

/*--------------------------------------------------------------------------*/
/* Z GET HEX HEADER                                                         */
/* Receive a hex style header (type and position)                           */
/*--------------------------------------------------------------------------*/
int _Z_GetHexHeader(char *hdr)                                     //_Z_GetHexHeader“?”
{ int c,n; WORD crc;

  if ((c   = _Z_GetHex()) < 0) return c;
  Rxtype   = c;
  crc      = Z_UpdateCRC(c, 0);

  for (n=4; --n >= 0;) {
    if ((c = _Z_GetHex()) < 0) return c;
    crc      = Z_UpdateCRC(c, crc);
    *hdr++   = c;
    }
  if ((c = _Z_GetHex()) < 0) return c;
  crc = Z_UpdateCRC(c, crc);
  if ((c = _Z_GetHex()) < 0) return c;
  crc = Z_UpdateCRC(c, crc);
  if (crc & 0xFFFF) return ZZERROR;     //CRC error         //CRC??
  c = Z_GetByte(1000);
  if (!bTimeOut) { if (c == '\x0d') Z_GetByte(1000); } // Throw away possible cr/lf     //?????cr/lf

  return Rxtype;
}

/*--------------------------------------------------------------------------*/
/* Z PULL LONG FROM HEADER                                                  */
/* Recover a long integer from a header                                     */          //????????from a header
/*--------------------------------------------------------------------------*/
DWORD _Z_PullLongFromHeader(char *hdr)                                                  //_Z_PullLongFromHeader“?”
{ DWORD l;

  l = hdr[ZP3];
  l = (l << 8) | hdr[ZP2];
  l = (l << 8) | hdr[ZP1];
  l = (l << 8) | hdr[ZP0];
  return l;
}

/*--------------------------------------------------------------------------*/
/* Z LONG TO HEADER                                                         */
/* Store long integer pos in Txhdr                                          */
/*--------------------------------------------------------------------------*/
void Z_PutLongIntoHeader(DWORD pos)                                                     //Z_PutLongIntoHeader“?”
{
  Txhdr[ZP0] = pos;
  Txhdr[ZP1] = pos>>8;
  Txhdr[ZP2] = pos>>16;
  Txhdr[ZP3] = pos>>24;
}

/*--------------------------------------------------------------------------*/
/* Z GET HEADER                                                             */
/* Read a ZMODEM header to hdr, either binary or hex.                       */
/*   On success, set Zmodem to 1 and return type of header.                 */         //?zodem??1,????header???
/*   Otherwise return negative on error                                     */         //????negative on error
/*--------------------------------------------------------------------------*/
int Z_GetHeader(char *hdr)                                                             //Z_GetHeader“?”
{ int c,n,cancount;

  n = 10;   /* Max characters before start of frame */                                 //……
  cancount = 5;

Again:
  Rxframeind = Rxtype = 0;

  c = _Z_TimedRead(); if (bTimeOut) c = ZZTIMEOUT;
  switch (c) {
    case ZPAD: break; // This is what we want.                                        //???????
    case ZZRCDO:
    case ZZTIMEOUT: goto Done; //Timeout                                              //????
    case CAN: if (--cancount <= 0) { c = ZCAN; goto Done; } //Cancel no break         //??,?????
    default:
Agn2: if (--n <= 0) return ZCAN;
      if (c != CAN) cancount = 5;
      goto Again;
    }

  cancount = 5;
Splat:
  c = _Z_TimedRead(); if (bTimeOut) c = ZZTIMEOUT;
  switch (c) {
    case ZDLE: break; // This is what we want.                                        //???????
    case ZPAD: goto Splat;
    case ZZRCDO:
    case ZZTIMEOUT: goto Done; //Timeout                                              //????
    default: goto Agn2;
    }

  c = _Z_TimedRead(); if (bTimeOut) c = ZZTIMEOUT;
  switch (c) {
    case ZBIN: //BIN head                                                             
      Rxframeind = ZBIN;
      c =  _Z_GetBinaryHeader(hdr);
      break;
    case ZHEX: //HEX head
      Rxframeind = ZHEX;
      c =  _Z_GetHexHeader(hdr);
      break;
    case CAN: //Cancel
      if (--cancount <= 0) { c = ZCAN; goto Done; }
      goto Agn2;
    case ZZRCDO:
    case ZZTIMEOUT: goto Done; //Timeout
    default: goto Agn2;
    }

  Rxpos = _Z_PullLongFromHeader(hdr);
Done:
  return c;
}

/*--------------------------------------------------------------------------*/
/* RZ RECEIVE DATA                                                          */  //RZ????
/* Receive array buf of max length with ending ZDLE sequence                */  //?zdle?CRC??????,?????????
/* and CRC.  Returns the ending character or error code.                    */
/*--------------------------------------------------------------------------*/
int RZ_ReceiveData(BYTE *buf,int length)
{ int c,d; WORD crc;

  crc   = RxCount   = 0;
  for (;;) {
    if ((c = Z_GetZDL()) & ~0xff) {
CRCfoo:
      switch (c) {
        case GOTCRCE:
        case GOTCRCG:
        case GOTCRCQ:
        case GOTCRCW: // C R C s
          crc = Z_UpdateCRC(((d=c)&0xff), crc);
          if ((c=Z_GetZDL()) & ~0xff) goto CRCfoo;

          crc = Z_UpdateCRC(c, crc);
          if ((c=Z_GetZDL()) & ~0xff) goto CRCfoo;

          crc = Z_UpdateCRC(c, crc);
          if (crc & 0xffff) return ZZERROR;     //CRC error             //CRC??
          return d;
        case GOTCAN:    return ZCAN;    //Cancel                        //??
        case ZZTIMEOUT: return c;       //Timeout                       //????
        case ZZRCDO:    return c;       //No Carrier                    //????
        default:        return c;       //Something bizarre             //???
        }
      }
    if (--length < 0) return ZZERROR;   //Long pkt                      //……
    ++RxCount;
    *buf++ = c;
    crc = Z_UpdateCRC(c, crc);
    continue;
    }
}

/*--------------------------------------------------------------------------*/
/* RZ ACK BIBI                                                              */  //……
/* Ack a ZFIN packet, let byegones be byegones                              */  //……
/*--------------------------------------------------------------------------*/
void RZ_AckBibi(void)
{ int n;

  Z_PutLongIntoHeader(0L);
  for (n=4; --n;) { BYTE c;
    Z_SendHexHeader(ZFIN, Txhdr);
    c = Z_GetByte(1000); if (bTimeOut) return;
    if (c == 'O') Z_GetByte(1000);      // Discard 2nd 'O'   
    }
}

/*--------------------------------------------------------------------------*/
/* RZ INIT RECEIVER                                                         */
/* Initialize for Zmodem receive attempt, try to activate Zmodem sender     */  //???zmodem??,?????zmodem???
/* Handles ZSINIT, ZFREECNT, and ZCOMMAND frames                            */  //……
/*                                                                          */
/* Return codes:                                                            */
/*    ZFILE .... Zmodem filename received                                   */  //zmodem?????
/*    ZCOMPL ... transaction finished                                       */  //????
/*    ERROR .... any other condition                                        */  //????
/*--------------------------------------------------------------------------*/
int RZ_InitReceiver(void)                                                       //RZ_InitReceiver,??????
{ int n,errors = 0;
   
  for (n=3; --n>=0; ) {
    Z_PutLongIntoHeader(0L);
    Txhdr[ZF0] = CANFDX;        // | CANOVIO;
    Z_SendHexHeader(Tryzhdrtype, Txhdr);

AGAIN:
    switch (Z_GetHeader(Rxhdr)) {
      case ZRQINIT:     //USED
        Z_PutLongIntoHeader(0L);
        Txhdr[ZF0] = CANFDX;    // | CANOVIO;
        Z_SendHexHeader(ZRINIT, Txhdr);
        goto AGAIN;
      case ZFILE:       //USED
        Zconv = Rxhdr[ZF0];
        Tryzhdrtype = ZRINIT;
        if (RZ_ReceiveData(Recbuf,WAZOOMAX) == GOTCRCW) return ZFILE;
        Z_SendHexHeader(ZNAK, Txhdr);
        goto AGAIN;
      case ZSINIT:      //not used
        if (RZ_ReceiveData(Attn, ZATTNLEN) == GOTCRCW) Z_SendHexHeader(ZACK,Txhdr);
        else Z_SendHexHeader(ZNAK,Txhdr);
        goto AGAIN;
      case ZFREECNT:    //not used
        Z_PutLongIntoHeader(WAZOOMAX);
        Z_SendHexHeader(ZACK, Txhdr);
        goto AGAIN;
      case ZCOMMAND:    //not used
        /*-----------------------------------------*/
        /* Paranoia is good for you...             */
        /* Ignore command from remote, but lie and */
        /* say we did the command ok.              */
        /*-----------------------------------------*/
        if (RZ_ReceiveData(Recbuf,WAZOOMAX) == GOTCRCW) {
          Z_PutLongIntoHeader(0L); 
          do { Z_SendHexHeader(ZCOMPL, Txhdr); }
             while (++errors<10 && Z_GetHeader(Rxhdr) != ZFIN);
          RZ_AckBibi();
          return ZCOMPL;
          }
        else Z_SendHexHeader(ZNAK, Txhdr);
        goto AGAIN;
      case ZCOMPL:      //not used
        goto AGAIN;
      case ZFIN:        //USED
        RZ_AckBibi(); return ZCOMPL;
      case ZCAN:        //USED
      case ZZRCDO: 
		return ZZERROR;
      case ZZTIMEOUT:   //USED
        return ZZERROR;
      }
    }
    return ZZERROR;
}

/*--------------------------------------------------------------------------*/
/* RZ GET HEADER                                                            */
/* Process incoming file information header                                 */  //??????header
/*--------------------------------------------------------------------------*/
DWORD filesize; BYTE filename[32];
int RZ_GetHeader(void)                                                          //RZ_GetHeader……
{ BYTE *p; BYTE i=0;

  filesize = 0L;
  p = Recbuf; while (*p) filename[i++] = *p++; filename[i] = '\0'; p++;
  while (*p) { filesize = filesize * 10 + (*p - '0'); p++; }

  return ZZOK;
}

/*--------------------------------------------------------------------------*/
/* RZ SAVE TO DISK                                                          */
/* Writes the received file data to the output file.                        */
/* If in ASCII mode, stops writing at first ^Z, and converts all            */
/*   solo CR's or LF's to CR/LF pairs.                                      */
/*--------------------------------------------------------------------------*/
//Program Flash  address=base+ *prxbytes; cnt=RxCount;
int RZ_SaveToDisk(DWORD *prxbytes)
{
  *prxbytes += RxCount;
  return ZZOK;
}

/*--------------------------------------------------------------------------*/
/* RZ RECEIVE FILE                                                          */
/* Receive one file; assumes file name frame is preloaded in Recbuf         */
/*--------------------------------------------------------------------------*/
int RZ_ReceiveFile(BYTE *p)
{ int c,n;

  n        = 10;        //retry times;
  rxbytes  = 0L;

  for (;;) {
    Z_PutLongIntoHeader(rxbytes);
    Z_SendHexHeader(ZRPOS, Txhdr);
NxtHdr:
    switch (c = Z_GetHeader(Rxhdr)) {
	case ZDATA: // Data Packet
		  if (Rxpos != rxbytes) {
          if ( --n < 0) return ZZERROR;
          continue;
          }
MoreData:
        switch (c = RZ_ReceiveData(FileBuf+rxbytes,WAZOOMAX)) {
          case ZCAN:
          case ZZRCDO:  return ZZERROR; //CAN or CARRIER
          case ZZERROR: // CRC error
            if (--n<0) return ZZERROR;
            continue;
          case ZZTIMEOUT: //Timeout
            if (--n<0) return ZZERROR;
            continue;
          case GOTCRCW: // End of frame
            n = 10;
            if (RZ_SaveToDisk(&rxbytes) == ZZERROR) return ZZERROR;
            Z_PutLongIntoHeader(rxbytes);
            Z_SendHexHeader(ZACK, Txhdr);
            goto NxtHdr;
          case GOTCRCQ: // Zack expected
            n = 10;
            if (RZ_SaveToDisk(&rxbytes) == ZZERROR) return ZZERROR;
            Z_PutLongIntoHeader(rxbytes);
            Z_SendHexHeader(ZACK, Txhdr);
            goto MoreData;
          case GOTCRCG: // Non-stop
            n = 10;
            if (RZ_SaveToDisk(&rxbytes) == ZZERROR) return ZZERROR;
            goto MoreData;
          case GOTCRCE: // Header to follow
            n = 10;
            if (RZ_SaveToDisk(&rxbytes) == ZZERROR) return ZZERROR;
            goto NxtHdr;
          }
      case ZNAK:
      case ZZTIMEOUT: // Packed was probably garbled
        if ( --n < 0) return ZZERROR;
        continue;
      case ZFILE: // Sender didn't see our ZRPOS yet
        RZ_ReceiveData(Recbuf, WAZOOMAX);
        continue;
      case ZEOF: // End of the file
        if (Rxpos != rxbytes) continue;
        return c;
      case ZZERROR: // Too much garbage in header search error
        if ( --n < 0) return ZZERROR;
        continue;

      default: return ZZERROR;
      }
    }
}

/*--------------------------------------------------------------------------*/
/* RZFILES                                                                  */
/* Receive a batch of files using ZMODEM protocol                           */
/*--------------------------------------------------------------------------*/
int RZ_ReceiveBatch(BYTE *p)
{ int c;

  for (;;) {
    switch (c = RZ_ReceiveFile(p)) {
      case ZEOF:
      case ZSKIP:
        switch (RZ_InitReceiver()) {
          case ZCOMPL:   return ZZOK;
          case ZFILE:    break;
          default:       return ZZERROR;
          }
        break;
      default: return c;
      }
    }
}

/*--------------------------------------------------------------------------*/
/* GET ZMODEM                                                               */
/* Receive a batch of files. //½ÓÊÜÎÄ¼þ                                               */
/* returns TRUE (1) for good xfer, FALSE (0) for bad                        */
/* can be called from f_upload or to get mail from a WaZOO Opus             */
/*--------------------------------------------------------------------------*/

DWORD get_Zmodem(BYTE *p)
{ int i;

  Tryzhdrtype = ZRINIT;

  //Z_ClearByte();

  FileBuf = (BYTE *)p;
  i = RZ_InitReceiver();
  if ((i == ZCOMPL) || ((i == ZFILE) && ((RZ_ReceiveBatch(p)) == ZZOK))) {
    SendHEX(1,rxbytes>>24);
		SendHEX(1,rxbytes>>16); 
		SendHEX(1,rxbytes>>8); 
		SendHEX(1,rxbytes);
    //SendBuffer(1," bytes received!!!\x0d\x0a");
		printf("bytes received!!!\x0d\x0a");
    Z_ClearByte();
    return rxbytes;
    }
  else {
    //SendBuffer(1,"Download error!!!\x0d\x0a");
		printf("Download error!!!\x0d\x0a");
    Z_ClearByte();
    return 0L;
    }
}




BYTE OneSramByte(DWORD add)
{
	unsigned char data;

	data = TEXT_DATA[add];
	return data;
}


/*--------------------------------------------------------------------------*/
/* Private data                                                             */
/*--------------------------------------------------------------------------*/

DWORD  Txpos;              /* Transmitted file position     ÎÄ¼þ´«Êä·½Î»    */
int Rxbuflen = 0;          /* Receiver's max buffer length       ½ÓÊÜµÄ×î´ó»º³åÇø³¤¶È     */


/*--------------------------------------------------------------------------*/
/* ZS SEND BYTE                                                             */
/* Send character c with ZMODEM escape sequence encoding.                   */
/* Escape XON, XOFF. Escape CR following @ (Telenet net escape)             */
/*--------------------------------------------------------------------------*/
void ZS_SendByte(BYTE c)
{
	static BYTE lastsent;

	switch (c)
	{
		case 015:
		case 0215:  /*--------------------------------------------------*/
					/*                                                  */
					/*--------------------------------------------------*/
			if ((lastsent & 0x7F) != '@') goto SendIt;
		case 020:
		case 021:
		case 023:
		case 0220:
		case 0221:
		case 0223:
		case ZDLE:  /*--------------------------------------------------*/
					/* Quoted characters                                */
					/*--------------------------------------------------*/
			SENDBYTE(ZDLE);
			c ^= 0x40;

		default:    /*--------------------------------------------------*/
					/* Normal character output                          */
SendIt:				/*--------------------------------------------------*/
			SENDBYTE(lastsent = c);
	}
}

/*--------------------------------------------------------------------------*/
/* ZS SEND BINARY HEADER                                                    */
/* Send ZMODEM binary header hdr of type type                               */
/*--------------------------------------------------------------------------*/
void ZS_SendBinaryHeader(int type, char *hdr)
{
	char *hptr;
	WORD crc;
	int  n;

	SENDBYTE(ZPAD);
	SENDBYTE(ZDLE);
	SENDBYTE(ZBIN);
	ZS_SendByte(type);

	crc = Z_UpdateCRC(type, 0);

	hptr  = hdr;
	for (n=4; --n >= 0;)
	{
		ZS_SendByte(*hptr);
		crc = Z_UpdateCRC(((unsigned short)(*hptr++)), crc);
	}

	crc = Z_UpdateCRC(0,crc);
	crc = Z_UpdateCRC(0,crc);
	ZS_SendByte(crc>>8);
	ZS_SendByte(crc);
}

/*--------------------------------------------------------------------------*/
/* ZS GET RECEIVER INFO                                                     */
/* Get the receiver's init parameters                                       */
/*--------------------------------------------------------------------------*/
int ZS_GetReceiverInfo()
{
	int   n;

	for (n=10; --n>=0; )
	{
		switch ( Z_GetHeader(Rxhdr) )
		{
			case ZCHALLENGE:	/*--------------------------------------*/
								/* Echo receiver's challenge number     */
								/*--------------------------------------*/
				Z_PutLongIntoHeader(Rxpos);
				Z_SendHexHeader(ZACK, Txhdr);
				continue;
			case ZCOMMAND:		/*--------------------------------------*/
								/* They didn't see our ZRQINIT          */
								/*--------------------------------------*/
				Z_PutLongIntoHeader(0L);
				Z_SendHexHeader(ZRQINIT, Txhdr);
				continue;
			case ZRINIT:		/*--------------------------------------*/
								/*                                      */
								/*--------------------------------------*/
				Rxbuflen = ((WORD)Rxhdr[ZP1]<<8)|Rxhdr[ZP0];
				return ZZOK;
			case ZCAN:
			case ZZRCDO:
			case ZZTIMEOUT:		/*--------------------------------------*/
								/*                                      */
								/*--------------------------------------*/
				return ZZERROR;
			case ZRQINIT:		/*--------------------------------------*/
								/*                                      */
								/*--------------------------------------*/
				if (Rxhdr[ZF0] == ZCOMMAND) continue;
			default:			/*--------------------------------------*/
								/*                                      */
								/*--------------------------------------*/
				Z_SendHexHeader(ZNAK, Txhdr);
				continue;
		}
	}
	return ZZERROR;
}

/*--------------------------------------------------------------------------*/
/* ZS SEND DATA                                                             */
/* Send binary array buf with ending ZDLE sequence frameend                 */
/*--------------------------------------------------------------------------*/
void ZS_SendData(DWORD pos, DWORD end, unsigned short frameend)
{
	WORD crc;
	//unsigned char *data = "hello world";
	crc = 0;
	
	
	while (pos < end)
	{
		BYTE b = OneSramByte(pos++);//ÔÝÍ£Ò»ÏÂ
		ZS_SendByte(b);
		crc = Z_UpdateCRC(((unsigned short)b), crc);
	}
	/*
	while(*data)//pos < end
	{
		ZS_SendByte(*data);
		crc = Z_UpdateCRC(((unsigned short)data), crc);
		if (*data) ;
		else break;
		data++;
	}*/
//	frameend = ZCRCE;//chuck×Ô¼ºÔö¼Ó
	SENDBYTE(ZDLE);
	SENDBYTE(frameend);
	crc = Z_UpdateCRC(frameend, crc);

	crc = Z_UpdateCRC(0,crc);
	crc = Z_UpdateCRC(0,crc);
	ZS_SendByte(crc>>8);
	ZS_SendByte(crc);
}

int modify = 40;
void ZS_SendFileName(BYTE *name, int numK)//1
{
	WORD crc = 0;
	BYTE str[] = "00000000 000 0\0", *p;
	DWORD lenB = numK;//numK * 1024;//1024  Õâ¸ö²ÎÊýµÄÒâÒå£¿ ÎÄ¼þÊýÁ¿»¹ÊÇÎÄ¼þ´óÐ¡Ñ½  ÎÄ¼þ³¤¶È£¿

	p = str;//ÎÄ¼þ³¤¶È 
	while (lenB >= 100000) { lenB -= 100000; (*p)++; } if (p != str) p++; else if (*p != '0') p++;
	while (lenB >=  10000) { lenB -=  10000; (*p)++; } if (p != str) p++; else if (*p != '0') p++;
	while (lenB >=   1000) { lenB -=   1000; (*p)++; } if (p != str) p++; else if (*p != '0') p++;
	while (lenB >=    100) { lenB -=    100; (*p)++; } if (p != str) p++; else if (*p != '0') p++;
	while (lenB >=     10) { lenB -=     10; (*p)++; } if (p != str) p++; else if (*p != '0') p++;
	while (lenB >=      1) { lenB -=      1; (*p)++; } p++; *p++ = ' ';

	modify++;//ÐÞ¸ÄÈÕÆÚ£¿
	while (modify >= 100) { modify -= 100; (*p)++; }  p++;
	while (modify >=  10) { modify -=  10; (*p)++; }  p++;//41
	while (modify >=   1) { modify -=   1; (*p)++; }  p++;
	
	//UnixÊ±¼ä
	*p++ = ' ';
	*p++ = '0'; 
	*p = '\0';
	
	for (;;)
	{
		ZS_SendByte(*name);
		crc = Z_UpdateCRC(((unsigned short)*name), crc);
		if (*name) ;
		else break;
		name++;
	}
	p = str;
	for (;;)
	{
		ZS_SendByte(*p);
		crc = Z_UpdateCRC(((unsigned short)*p), crc);
		if (*p) ; else break;
		p++;
	}
	SENDBYTE(ZDLE);
	SENDBYTE(ZCRCW);
	crc = Z_UpdateCRC(ZCRCW, crc);

	crc = Z_UpdateCRC(0, crc);
	crc = Z_UpdateCRC(0, crc);
	ZS_SendByte(crc >> 8);
	ZS_SendByte(crc);

//	SENDBYTE(XON);
}

/*--------------------------------------------------------------------------*/
/* ZS SYNC WITH RECEIVER                                                    */
/* Respond to receiver's complaint, get back in sync with receiver     »ØÓ¦½ÓÊÕ·½·¢¹ýÀ´µÄÐÅÏ¢£»Óë½ÓÊÕ·½±£³ÖÍ¬²½     */
/*--------------------------------------------------------------------------*/
int ZS_SyncWithReceiver()
{
	int c;
	int num_errs = 7;

	while(1)
	{
		c = Z_GetHeader(Rxhdr);

		switch (c)
		{
			case ZZTIMEOUT:  if ((num_errs--) >= 0) break;
			case ZCAN:
			case ZABORT:
			case ZFIN:
			case ZZRCDO:
						/*-----------------------------------------*/
						/*                                         */
						/*-----------------------------------------*/
				return ZZERROR;
			case ZRPOS:	/*-----------------------------------------*/
						/*                                         */
						/*-----------------------------------------*/
				Txpos = Rxpos;
				return c;
			case ZSKIP:	/*-----------------------------------------*/
						/*                                         */
						/*-----------------------------------------*/
			case ZRINIT:/*-----------------------------------------*/
						/*                                         */
						/*-----------------------------------------*/
				return c;
			case ZACK:	/*-----------------------------------------*/
						/*                                         */
						/*-----------------------------------------*/
				return c;
			default:	/*-----------------------------------------*/
						/*                                         */
						/*-----------------------------------------*/
				ZS_SendBinaryHeader(ZNAK, Txhdr);
				continue;
		}
	}
}

/*--------------------------------------------------------------------------
 ZS SEND FILE DATA                                                        
 Send the data in the file  ·¢ËÍ´ÓÎÄ¼þÖÐ»ñµÃµ½µÄÊý¾Ý
²ÎÊý£ºend £»
			blk	£ºÓ¦¸ÃÎªÊý¾Ý³¤¶È

--------------------------------------------------------------------------*/
int ZS_SendFileData(DWORD end, int blk)
{
	int c, e;
//	int newcnt, 
	int blklen, maxblklen;

	if (Rxbuflen && (blk > Rxbuflen)) maxblklen = Rxbuflen;
	else maxblklen = blk;
	blklen = maxblklen;
	//printf("blklen = %d \n",blklen);//Õâ¸öÖµÎªblk
	//CLK_SysTickDelay(2000);
SomeMore:
	if (Z_TestOneByte())
	{
WaitAck:
		switch (c = ZS_SyncWithReceiver())
		{
			default:	/*-----------------------------------------*/
						/*                                         */
						/*-----------------------------------------*/
				return ZZERROR;
			case ZSKIP:	/*-----------------------------------------*/
						/* Skip this file                          */
						/*-----------------------------------------*/
				return c;
			case ZACK:	/*-----------------------------------------*/
						/*                                         */
						/*-----------------------------------------*/
				break;
			case ZRPOS:	/*-----------------------------------------*/
						/* Resume at this position                 */
						/*-----------------------------------------*/
				break;
			case ZRINIT:/*-----------------------------------------*/
						/* Receive init                            */
						/*-----------------------------------------*/
				return ZZOK;
		}
	}
	while (Z_TestOneByte())
	{
		switch (Z_GetByte(1000))
		{
			case CAN:
			case ZZRCDO:
			case ZPAD:  goto WaitAck;
		}
	}

	Z_PutLongIntoHeader(Txpos);
	ZS_SendBinaryHeader(ZDATA, Txhdr);

	do
	{
		c = blklen;
		if ((Txpos + c) > end) //ÒªÂú×ãÕâÒ»²½
			c = end - Txpos;
		if (c < blklen) 
			e = ZCRCE;//ÎÒÒªÖ´ÐÐÕâÒ»²½
//		else if (Rxbuflen && ((newcnt -= c) <= 0)) e = ZCRCW;
		else e = ZCRCG;//'i'

		ZS_SendData(Txpos, Txpos + c, e);//Txpos ³õÊ¼Î»ÖÃ Txpos+c ½áÊøÎ»ÖÃ eÀàÐÍ
		Txpos += c;

		if (e == ZCRCW) goto WaitAck;

		while (Z_TestOneByte())
		{
			switch (Z_GetByte(1000))
			{
				case CAN:
				case ZZRCDO:
				case ZPAD:  /*--------------------------------------*/
							/* Interruption detected;               */
							/* stop sending and process complaint   */
							/*--------------------------------------*/
					ZS_SendData(Txpos, Txpos, ZCRCE);//'h'
					goto WaitAck;
			}
		}
	} while (e == ZCRCG);

	while(1)
	{
		Z_PutLongIntoHeader(Txpos);
		ZS_SendBinaryHeader(ZEOF, Txhdr);

		switch (ZS_SyncWithReceiver())
		{
			case ZACK:		/*-----------------------------------------*/
							/*                                         */
							/*-----------------------------------------*/
				continue;
			case ZRPOS:		/*-----------------------------------------*/
							/* Resume at this position...              */
							/*-----------------------------------------*/
				goto SomeMore;
			case ZRINIT:	/*-----------------------------------------*/
							/* Receive init                            */
							/*-----------------------------------------*/
				return ZZOK;
			case ZSKIP:		/*-----------------------------------------*/
							/* Request to skip the current file        */
							/*-----------------------------------------*/
				return c;
			default:		/*-----------------------------------------*/
							/*                                         */
							/*-----------------------------------------*/
				return ZZERROR;
		}
	}
}

/*--------------------------------------------------------------------------*/
/* ZS SEND FILE                                                             */
/* Send ZFILE frame and begin sending ZDATA frame                           */
/*--------------------------------------------------------------------------*/
int ZS_SendFile(BYTE *name, int numK)
{	int c;

	for (;;)
	{
		Txhdr[ZF0] = LZCONV;    /* Default file conversion mode */
		Txhdr[ZF1] = LZMANAG;   /* Default file management mode */
		Txhdr[ZF2] = LZTRANS;   /* Default file transport mode */
		Txhdr[ZF3] = 0;
		ZS_SendBinaryHeader(ZFILE, Txhdr);
		ZS_SendFileName(name, numK);//µ½Õâ±ß¶¼Ã»ÓÐÎÊÌâ

Again:
		switch (c = Z_GetHeader(Rxhdr))
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
				Txpos = Rxpos;				//ÎÄ¼þ´«ÊäµÄ·½Î»ÎªZRPOS·µ»ØµÄÊýÖµ
		//		return ZS_SendFileData(numK * 1024L, 1024);//Ä¬ÈÏ
			return ZS_SendFileData(numK  , 1024);//£¨end£¬blk£© ÎÒÐÞ¸ÄµÄ
		}
	}
}

/*--------------------------------------------------------------------------*/
/* ZS END SEND                                                              */
/* Say BIBI to the receiver, try to do it cleanly                           */
/*--------------------------------------------------------------------------*/
void ZS_EndSend()
{
	while(1)
	{
		Z_PutLongIntoHeader(0L);
		ZS_SendBinaryHeader(ZFIN, Txhdr);

		switch (Z_GetHeader(Rxhdr))
		{
			case ZFIN:		/*-----------------------------------------*/
							/*                                         */
							/*-----------------------------------------*/
				SENDBYTE('O');
				SENDBYTE('O');
			case ZCAN:
			case ZZRCDO:
			case ZZTIMEOUT:	/*-----------------------------------------*/
							/*                                         */
							/*-----------------------------------------*/
				 return;
		}
	}
}

/*--------------------------------------------------------------------------*/
/* SEND ZMODEM (send a file)                                                */
/*   returns TRUE (1) for good xfer, FALSE (0) for bad                      */
/*   sends one file per call; 'fsent' flags start and end of batch          */
/*--------------------------------------------------------------------------*/
DWORD send_Zmodem(BYTE *name, int numK)
{//numk ÎÄ¼þÊýÁ¿£¿
	Z_ClearByte();//ÔÝÊ±Ã»²»È·¶¨×÷ÓÃ
	
	Z_PutLongIntoHeader(0L);
	Z_SendHexHeader(ZRQINIT, Txhdr);

	if (ZS_GetReceiverInfo() == ZZERROR) return 0;

	/*--------------------------------------------------------------------*/
	/* Check the results                                                  */
	/*--------------------------------------------------------------------*/
	if (ZS_SendFile(name, numK) == ZZERROR) { ZS_EndSend(); return 0; }
	else									{ ZS_EndSend(); return numK; }
}

