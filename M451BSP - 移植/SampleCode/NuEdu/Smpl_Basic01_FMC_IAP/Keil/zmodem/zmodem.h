/****************************************************************
Copyright(c) 2005 Tsinghua Tongfang Optical Disc Co., Ltd. 
Developed by Storage Product Department, Beijing, China 
*****************************************************************
Filename: zmodem.h
Description: 
****************************************************************/
#include<stdio.h>

#define BYTE unsigned char 
#define WORD unsigned int 
#define DWORD long int 
#define FALSE 0
#define TRUE 1
#define BOOL bool
//#define NULL '\0'


//error define
#define ZMODEMOK 		0
#define ZMODEMTRUE 		1
#define ZMODEMERROR 	(-1)
#define ZMODEMTIMEOUT 	(-2)
#define ZMODEMRCDO 		(-3)
#define ZMODEMGCOUNT 	(-4)

#define sendline(c) 	zmputchar(c)
#define xsendline(c) 	zmputchar(c)
extern void zmputchar(char c);
extern int readline(unsigned int timeout);

/*--------------------------------------------------------------------------*/
/* Contro Text define                                                       */
/*--------------------------------------------------------------------------*/
#define ZPAD            '*'            /* 052 Pad character begins frames   */
#define ZDLE            030            /* ^X Zmodem escape- `ala BISYNC DLE */
#define ZDLEE           (ZDLE^0100)    /* Escaped ZDLE as transmitted       */
#define ZBIN            'A'            /* Binary frame indicator            */
#define ZHEX            'B'            /* HEX frame indicator               */
#define ZBIN32          'C'            /* Binary frame with 32 bit FCS      */

/*--------------------------------------------------------------------------*/
/* Frame types (see array "frametypes" in zmodem.c)                         */
/*--------------------------------------------------------------------------*/
#define ZRQINIT         0              /* Request receive init              */
#define ZRINIT          1              /* Receive init                      */
#define ZSINIT          2              /* Send init sequence (optional)     */
#define ZACK            3              /* ACK to above                      */
#define ZFILE           4              /* File name from sender             */
#define ZSKIP           5              /* To sender: skip this file         */
#define ZNAK            6              /* Last packet was garbled           */
#define ZABORT          7              /* Abort batch transfers             */
#define ZFIN            8              /* Finish session                    */
#define ZRPOS           9              /* Resume transmit at this position  */
#define ZDATA           10             /* Data packet(s) follow             */
#define ZEOF            11             /* End of file                       */
#define ZFERR           12             /* Fatal Read/Write error Detected   */
#define ZCRC            13             /* Request for file CRC and response */
#define ZCHALLENGE      14             /* Receiver's Challenge              */
#define ZCOMPL          15             /* Request is complete               */
#define ZCAN            16             /* Other end canned with CAN*5       */
#define ZFREECNT        17             /* Request for free bytes on disk    */
#define ZCOMMAND        18             /* Command from sending program      */
#define ZSTDERR         19             /* Send following to stderr          */

/*--------------------------------------------------------------------------*/
/* ZDLE sequences                                                           */
/*--------------------------------------------------------------------------*/
#define ZCRCE           'h'            /* CRC next/frame ends/hdr follows   */
#define ZCRCG           'i'            /* CRC next/frame continues nonstop  */
#define ZCRCQ           'j'            /* CRC next/frame continues/want ZACK*/
#define ZCRCW           'k'            /* CRC next/ZACK expected/end of frame*/
#define ZRUB0           'l'            /* Translate to rubout 0177          */
#define ZRUB1           'm'            /* Translate to rubout 0377          */

/*--------------------------------------------------------------------------*/
/* Z_GetZDL return values (internal)                                        */
/* -1 is general error, -2 is timeout                                       */
/*--------------------------------------------------------------------------*/
#define GOTOR           0400           /* Octal alert! Octal alert!         */
#define GOTCRCE         (ZCRCE|GOTOR)  /* ZDLE-ZCRCE received               */
#define GOTCRCG         (ZCRCG|GOTOR)  /* ZDLE-ZCRCG received               */
#define GOTCRCQ         (ZCRCQ|GOTOR)  /* ZDLE-ZCRCQ received               */
#define GOTCRCW         (ZCRCW|GOTOR)  /* ZDLE-ZCRCW received               */
#define GOTCAN          (GOTOR|030)    /* CAN*5 seen                        */

/*--------------------------------------------------------------------------*/
/* Byte positions within header array                                       */
/*--------------------------------------------------------------------------*/
#define ZF0             3              /* First flags byte                  */
#define ZF1             2
#define ZF2             1
#define ZF3             0
#define ZP0             0              /* Low order 8 bits of position      */
#define ZP1             1
#define ZP2             2
#define ZP3             3              /* High order 8 bits of file pos     */

/*--------------------------------------------------------------------------*/
/* Bit Masks for ZRINIT flags byte ZF0                                      */
/*--------------------------------------------------------------------------*/
#define CANFDX          01             /* Can send and receive true FDX     */
#define CANOVIO         02             /* Can receive data during disk I/O  */
#define CANBRK          04             /* Can send a break signal           */
#define CANCRY          010            /* Can decrypt                       */
#define CANLZW          020            /* Can uncompress                    */
#define CANFC32         040            /* Can use 32 bit Frame Check        */


/*--------------------------------------------------------------------------*/
/* PARAMETERS FOR ZFILE FRAME...                                            */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* Conversion options one of these in ZF0                                   */
/*--------------------------------------------------------------------------*/
#define ZCBIN           1              /* Binary transfer - no conversion   */
#define ZCNL            2              /* Convert NL to local EOLN          */
#define ZCRESUM         3              /* Resume interrupted file transfer  */

/*--------------------------------------------------------------------------*/
/* Management options, one of these in ZF1                                  */
/*--------------------------------------------------------------------------*/
#define ZMNEW           1              /* Transfer iff source newer/longer  */
#define ZMCRC           2              /* Transfer if different CRC/length  */
#define ZMAPND          3              /* Append contents to existing file  */
#define ZMCLOB          4              /* Replace existing file             */
#define ZMSPARS         5              /* Encoding for sparse file          */
#define ZMDIFF          6              /* Transfer if dates/lengths differ  */
#define ZMPROT          7              /* Protect destination file          */

/*--------------------------------------------------------------------------*/
/* Transport options, one of these in ZF2                                   */
/*--------------------------------------------------------------------------*/
#define ZTLZW           1              /* Lempel-Ziv compression            */
#define ZTCRYPT         2              /* Encryption                        */
#define ZTRLE           3              /* Run Length encoding               */

/*--------------------------------------------------------------------------*/
/* Parameters for ZCOMMAND frame ZF0 (otherwise 0)                          */
/*--------------------------------------------------------------------------*/
#define ZCACK1          1              /* Acknowledge, then do command      */

/*--------------------------------------------------------------------------*/
/* Miscellaneous definitions                                                */
/*--------------------------------------------------------------------------*/
#define ZZOK            0
#define ZZERROR         (-1)
#define ZZTIMEOUT       (-2)
#define ZZRCDO          (-3)
#define ZZFUBAR         (-4)

#define XON             ('Q'&037)
#define XOFF            ('S'&037)
#define CPMEOF          ('Z'&037)

#define RXBINARY        FALSE          /* Force binary mode uploads?        */
#define RXASCII         FALSE          /* Force ASCII mode uploads?         */
#define LZCONV          0              /* Default ZMODEM conversion mode    */
#define LZMANAG         0              /* Default ZMODEM file mode          */
#define LZTRANS         0              /* Default ZMODEM transport mode     */
#define PATHLEN         128            /* Max legal MS-DOS path size?       */
#define KSIZE           1024           /* Max packet size (non-WaZOO)       */
#define WAZOOMAX        8192           /* Max packet size (WaZOO)           */
#define SECSPERDAY      (24L*60L*60L)  /* Number of seconds in one day      */


/*--------------------------------------------------------------------------*/
/* Status and error words                                                   */
/*--------------------------------------------------------------------------*/
/*
extern char  *EOT_msg;
extern char  *ACK_msg;
extern char  *NAK_msg;
extern char  *SOH_msg;
extern char  *CAN_msg;
extern char  *SYN_msg;
extern char  *DISK_msg;
extern char  *CRC_msg;
extern char  *CHK_msg;
extern char  *DUPE_msg;
extern char  *FUTURE_msg;
extern char  *C_msg;
extern char  *SHRT_msg;
extern char  *TIME_msg;
extern char  *SYN_msg;
extern char  *INIT_msg;
extern char  *FUBAR_msg;
extern char  *CARRIER_msg;
extern char  *NOTHING_msg;
extern char  *KBD_msg;
extern char  *IDUNNO_msg;
extern char  *CMPL_msg;
extern char  *SYNC_msg;
extern char  *OPEN_msg;
extern char  *WRITE_msg;
*/
//extern char  *CAN_xfer;                   /* lots of CANs then BSs          */


/*--------------------------------------------------------------------------*/
/* Parameters for calling ZMODEM routines                                   */
/*--------------------------------------------------------------------------*/
/*
#define TRUE            1
#define FALSE           0
#define END_BATCH       (-1)
#define NOTHING_TO_DO   (-2)
#define DELETE_AFTER    '-'
#define TRUNC_AFTER     '#'
#define NOTHING_AFTER   '@'
#define DO_WAZOO        TRUE
#define DONT_WAZOO      FALSE
*/

/*--------------------------------------------------------------------------*/
/* ASCII MNEMONICS                                                          */
/*--------------------------------------------------------------------------*/
#define NUL 0x00
#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define EOT 0x04
#define ENQ 0x05
#define ACK 0x06
#define BEL 0x07
#define BS  0x08
#define HT  0x09
#define LF  0x0a
#define VT  0x0b
#define FF  0x0c
#define CR  0x0d
#define SO  0x0e
#define SI  0x0f
#define DLE 0x10
#define DC1 0x11
#define DC2 0x12
#define DC3 0x13
#define DC4 0x14
#define NAK 0x15
#define SYN 0x16
#define ETB 0x17
#define CAN 0x18
#define EM  0x19
#define SUB 0x1a
#define ESC 0x1b
#define FS  0x1c
#define GS  0x1d
#define RS  0x1e
#define US  0x1f

/*--------------------------------------------------------------------------*/
/* Routines from N_Zmodem.H ...                                             */
/*--------------------------------------------------------------------------*/

BYTE Z_GetByte(int );
void Z_SendHexHeader(BYTE , char *);
int  Z_GetHeader(char *);
int  Z_GetZDL(void);
void Z_PutLongIntoHeader(DWORD );
WORD Z_UpdateCRC(WORD ,WORD );

DWORD get_Zmodem(BYTE *p);
DWORD send_Zmodem(BYTE *name, int numK);

/* END OF FILE: zmodem.h */

