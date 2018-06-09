/**************************************************************************//**
 * @file     ap_main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 14/12/22 2:14p $
 * @brief    FMC VECMAP sample program (loader) for Nano100 series MCU
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "M451Series.h"
#include "core_cm4.h"
#include "uart.h"
#include "NuEdu-Basic01.h"
#include "zmodem.h"
#include "map.h"
#include "hw_cg.h"

int  load_image_to_flash(uint32_t image_base, uint32_t image_limit, uint32_t flash_addr, uint32_t max_size);
int IsDebugFifoEmpty(void);

int psec(char *buf, unsigned int n);
char bbs[]={0x11,0x12,0x13,0x14};

void TMR0_IRQHandler(void)
{
    static uint32_t sec = 1;
//    printf("%d sec\n", sec++);
//    LED_On(sec);

    // clear timer interrupt flag
    TIMER_ClearIntFlag(TIMER0);

}
#ifdef __ARMCC_VERSION
__asm __INLINE __set_SP(uint32_t _sp)
{
    MSR MSP, r0
    BX lr
}
#endif
__INLINE void BranchTo(uint32_t u32Address)
{
    FUNC_PTR        *func;    
    FMC_SetVectorPageAddr(u32Address);
    func =  (FUNC_PTR *)(*(uint32_t *)(u32Address+4));
    printf("branch to address 0x%x\n", (int)func);
    printf("\n\nChange VECMAP and branch to user application...\n");
    while (!IsDebugFifoEmpty());
    __set_SP(*(uint32_t *)u32Address);
    func();		    
}

#define PLL_CLOCK           72000000
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
}

void Timer0_Init(void)
{
    // Give a dummy target frequency here. Will over write capture resolution with macro
    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 11);

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
unsigned char RECV[2048] = {0};
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
//    printf("Config0 = 0x%x, Config1 = 0x%x, CBS=%d\n", au32Config[0], au32Config[1], cbs);
//    printf("Boot loader program running on APROM\n");

//    au32Version[0] = FMC_Read(USER_AP0_ENTRY+0x1000);
//		send_Zmodem("test.txt",11);
			for(;;)
			{
				printf("\n请使用hypertrm进行文件接收或发送\n");
				printf("\n请选择功能\n1：接收文件（test.txt）\n2: 发送文件\n");
				UART_Read(UART0,&Qhuan,1);
				if(Qhuan == '1'||Qhuan == '2')
					break;
			}
			if(Qhuan == '1')
				send_Zmodem("test.txt",11);
//		RZ_InitReceiver();
			if(Qhuan == '2')
				get_Zmodem(RECV);
#if 0
		while(1){	
		//	printf("\n是否更新APP1请输入 y or n？\n");
			while(1){
				printf("\n是否更新APP1请输入 y or n？\n");	
				UART_Read(UART0,&Qhuan,1);
				if(Qhuan=='y'){
				//	printf("this is a test\n");
					FMC_EnableAPUpdate();
					ZmodemReceiveFiles();					
				  FMC_DisableAPUpdate();
				  while (!IsDebugFifoEmpty()){};
				}else if(Qhuan=='n'){
						break;	
				}
////				FMC_EnableAPUpdate();
////					psec(bbs,4);
////				FMC_DisableAPUpdate();
////				while (!IsDebugFifoEmpty()){};
			}
			printf("\n\nDo you want to branch AP1?(Yes/No)\n");
			au32Version[1] = FMC_Read(USER_AP1_ENTRY+0x1000);
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
    printf("Don't find any program on APROM\n");
	    
  } 
#endif		
}


int  load_image_to_flash(uint32_t image_base, uint32_t image_limit, uint32_t flash_addr, uint32_t max_size)
{
    uint32_t   i, j, u32Data, u32ImageSize, *pu32Loader;

    u32ImageSize = image_limit;
    if (u32ImageSize == 0) {
        printf("  ERROR: Loader Image is 0 bytes!\n");
        return -1;
    }

    if (u32ImageSize > max_size) {
        printf("  ERROR: Loader Image is larger than %d KBytes!\n", max_size/1024);
        return -1;
    }

    printf("Program image to flash address 0x%x...", flash_addr);
    pu32Loader = (uint32_t *)image_base;
    for (i = 0; i < u32ImageSize; i += FMC_FLASH_PAGE_SIZE) {
        if (FMC_Erase(flash_addr + i)) {
            printf("Erase failed on 0x%x\n", flash_addr + i);
            return -1;
        }

        for (j = 0; j < FMC_FLASH_PAGE_SIZE; j += 4) {
            FMC_Write(flash_addr + i + j, pu32Loader[(i + j) / 4]);
        }
    }
    printf("OK.\n");

    printf("Verify ...");

    /* Verify loader */
    for (i = 0; i < u32ImageSize; i += FMC_FLASH_PAGE_SIZE) {
        for (j = 0; j < FMC_FLASH_PAGE_SIZE; j += 4) {
            u32Data = FMC_Read(flash_addr + i + j);
            if (u32Data != pu32Loader[(i+j)/4]) {
                printf("data mismatch on 0x%x, [0x%x], [0x%x]\n", flash_addr + i + j, u32Data, pu32Loader[(i+j)/4]);
                return -1;
            }

            if (i + j >= u32ImageSize)
                break;
        }
    }
    printf("OK.\n");
    return 0;
}
int psec(char *buf, unsigned int n)
{
		unsigned int j;
 		unsigned int RamSource;
		static uint32_t FlashDestination=0x00004000;
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
	
			printf("\n\r\n\r%x\n\r\n\r", *(uint32_t *)FlashDestination);
			//下一编程位置和下一编程数据
      FlashDestination += 4;
      RamSource += 4;
		
		}
		return 0;
}


