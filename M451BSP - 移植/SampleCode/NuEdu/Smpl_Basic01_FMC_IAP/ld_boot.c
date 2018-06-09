/**************************************************************************//**
 * @file     ld_boot.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/12/22 2:14p $
 * @brief    FMC VECMAP sample program (LDROM code) for M451 series MCU
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include "M451Series.h"
#include "map.h"
#include "NuEdu-Basic01.h"
#define PLL_CLOCK           72000000

#ifdef __ARMCC_VERSION
__asm __INLINE __set_SP(uint32_t _sp)
{
    MSR MSP, r0
    BX lr
}
#endif

//void SendChar_ToUART(int ch);
/*---------------------------------------------------------------------------------------------------------*/
/*  Simple printf() function                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
//void printInteger(uint32_t u32Temp);
//void printHex(uint32_t u32Temp);
//void printf_UART(uint8_t *str,...)
//{
//		va_list args;
//		va_start( args, str );
//    while (*str != '\0')
//    {
//			if(*str == '%')
//			{
//				str++;
//				if (*str == '\0') return;
//				if( *str == 'd' )
//				{
//					str++;
//					printInteger(va_arg( args, int ));
//				}
//				if( *str == 'x' )
//				{
//					str++;
//					printHex(va_arg( args, int ));
//				}
//			}
//        SendChar_ToUART(*str++);
//    }
//}

//void printInteger(uint32_t u32Temp)
//{
//	uint8_t print_buf[16];
//	uint32_t i=15;
//	
//	*(print_buf+i) = '\0';
//		do
//    {
//			i--;
//      *(print_buf+i) = '0'+u32Temp%10;
//			u32Temp = u32Temp /10;
//    }while (u32Temp != 0);
//		printf_UART(print_buf+i);
//}

//void printHex(uint32_t u32Temp)
//{
//	uint8_t print_buf[16];
//	uint32_t i=15;
//	uint8_t hextemp;
//	
//	*(print_buf+i) = '\0';
//		do
//    {
//			i--;
//			hextemp = u32Temp%16;
//			if(hextemp > 10)
//				*(print_buf+i) = 'A'+(hextemp - 10);
//      else
//				*(print_buf+i) = '0'+hextemp;
//			u32Temp = u32Temp /16;
//    }while (u32Temp != 0);
//		printf_UART(print_buf+i);
//}
void TMR0_IRQHandler(void)
{
    static uint32_t sec = 1;
//    printf("%d sec\n", sec++);
//    LED_On(sec);

    // clear timer interrupt flag
    TIMER_ClearIntFlag(TIMER0);

}
__INLINE void BranchTo(uint32_t u32Address)
{
    FUNC_PTR        *func;    
    FMC_SetVectorPageAddr(u32Address);
    func =  (FUNC_PTR *)(*(uint32_t *)(u32Address+4));
    printf("branch to address 0x%x\n", (int)func);
    printf("\nChange VECMAP and branch to user application...\n");
    while (!(UART0->FIFOSTS & UART_FIFOSTS_TXEMPTY_Msk));
    __set_SP(*(uint32_t *)u32Address);
    func();		    
}
void SYS_Init(void)
{
	      /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Unlock protected registers */
    SYS_UnlockReg();
	
    /* Enable Internal RC 22.1184MHz clock */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Waiting for Internal RC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Enable external XTAL 12MHz clock */
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);

    /* Waiting for external XTAL clock ready */
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(PLL_CLOCK);
		  
		/* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock and CycylesPerUs automatically. */
    SystemCoreClockUpdate();
		
    /* Enable module clock */
    CLK_EnableModuleClock(UART0_MODULE);
		CLK_EnableModuleClock(TMR0_MODULE);

    /* Select module clock source as HXT and UART module clock divider as 1 */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UARTSEL_HXT, CLK_CLKDIV0_UART(1));
		CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HXT, 0);
		
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set PD multi-function pins for UART0 RXD(PD.6) and TXD(PD.1) */
    SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD6MFP_Msk | SYS_GPD_MFPL_PD1MFP_Msk);
    SYS->GPD_MFPL |= (SYS_GPD_MFPL_PD6MFP_UART0_RXD | SYS_GPD_MFPL_PD1MFP_UART0_TXD);
		
    /* Lock protected registers */
    SYS_LockReg();
//   /*---------------------------------------------------------------------------------------------------------*/
//    /* 系统时钟初始化                                                                                       */
//    /*---------------------------------------------------------------------------------------------------------*/
//    SYS_UnlockReg();                        /* 解锁保护寄存器 */
//    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);/* 使能内部rc 22.1184MHZ时钟 */   
//    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);/* 等待内部RC时钟就绪 */    
//    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));/* 切换HCLK时钟源内部RC和HCLK的1选项 */
//    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);/* 启动外部晶振12MHZ时钟 */    
//    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);/* 等待外部晶振时钟就绪 */    
//    CLK_SetCoreClock(PLL_CLOCK);/* 从PLL内设置核心频率 PLL_CLOCK */    
//    CLK_EnableModuleClock(UART0_MODULE);/* 启动模块时钟 */
//    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UARTSEL_HXT, CLK_CLKDIV0_UART(1));/* 选择模块时钟源HXT和UART模块时钟分频器为1 */
//		/* 更新系统内核时钟 */
//    /* 用户可以使用SystemCoreClockUpdate() 来计算SystemCoreClock和CycylesPerUs */
//    SystemCoreClockUpdate();
//		
//    /*---------------------------------------------------------------------------------------------------------*/
//    /* Init I/O Multi-function  初始化多功能I/O口                                                                               */
//    /*---------------------------------------------------------------------------------------------------------*/
//    /* 设置 PD 多功能引脚为UART0 RXD(PD.6)和TXD(PD.1) */
//    SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD6MFP_Msk | SYS_GPD_MFPL_PD1MFP_Msk);
//    SYS->GPD_MFPL |= (SYS_GPD_MFPL_PD6MFP_UART0_RXD | SYS_GPD_MFPL_PD1MFP_UART0_TXD);
//		
//    /* 锁保护寄存器 */
//    SYS_LockReg();
}
void Timer0_Init(void)
{
    // Give a dummy target frequency here. Will over write capture resolution with macro
    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 1);

    // Enable timer interrupt
    TIMER_EnableInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);

    // Start Timer 0
    TIMER_Start(TIMER0);    
}
void UART0_Init()
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset UART module */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main (void)
{
    int             cbs;
    uint32_t        au32Config[2];
    uint32_t        au32Version[2];
    volatile int    loop;
		uint8_t Qhuan;
    /* Init System, IP clock and multi-function I/O */
    SYS_Init();
    /* Init UART to 115200-8n1 for print message */
    UART0_Init();
		Timer0_Init();
    /* Enable FMC ISP function */
    SYS_UnlockReg();

    FMC_Open();
		
    FMC_ReadConfig(au32Config, 2);
    cbs = (au32Config[0] >> 6) & 0x3;
    printf("Config0 = 0x%x, Config1 = 0x%x, CBS=%d\n", au32Config[0], au32Config[1], cbs);
    printf("Boot loader program running on APROM\n");

//    au32Version[0] = FMC_Read(USER_AP0_ENTRY+0x1000);
    au32Version[1] = FMC_Read(USER_AP1_ENTRY+0x1000+0x1);

		while(1){	
			printf("\n是否更新APP1请输入 y or n？\n");
			while(1){
				UART_Read(UART0,&Qhuan,1);
				if(Qhuan=='y'){

				}else if(Qhuan=='n'){
						break;	
				}
				
			}
			printf("\n\nDo you want to branch AP1?(Yes/No)\n");
			while(1){
				UART_Read(UART0,&Qhuan,1);
				if(Qhuan=='y'){
					printf("Version Check\n");
					printf("Version for AP1:0x%x\n",au32Version[1]);							
					printf("Jumps to AP1\n");	
						BranchTo(USER_AP1_ENTRY);
					
				}else if(Qhuan=='n'){
					break;
				}
				
					
			}
				
//     printf_UART("Version for AP1:0x%x\n",au32Version[1]);	
//     printf_UART("Boot Selection\n");
//    if((au32Version[0]>=au32Version[1])&(au32Version[0]!=0xFFFFFFFF))
//    {
//        printf_UART("Jumps to AP0\n");	
//        BranchTo(USER_AP0_ENTRY);
//    }else if(au32Version[1]!=0xFFFFFFFF)
//    {
//        printf_UART("Jumps to AP1\n");	
//        BranchTo(USER_AP1_ENTRY);
//    }    
//    if((au32Version[0]<=au32Version[1])&(au32Version[1]!=0xFFFFFFFF))
//    {
//        printf_UART("Jumps to AP1\n");	
//        BranchTo(USER_AP1_ENTRY);
//    }else if(au32Version[0]!=0xFFFFFFFF)
//    {
//        printf_UART("Jumps to AP0\n");	
//        BranchTo(USER_AP0_ENTRY);
//    } 
    printf("Don't find any program on APROM\n");
	    
  }  
}
