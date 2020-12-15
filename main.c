#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NuMicro.h"
#include "i2c_transfer.h"
#include "spi_transfer.h"
#include "power.h"

extern uint8_t g_u8PowerStatus;

void SYS_Init(void)
{    
		/*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable HIRC clock */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Waiting for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Switch HCLK clock source to HIRC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));
	
    /* Enable module clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(SPI0_MODULE);
	
    /* Set module clock */
    CLK_SetModuleClock(SPI0_MODULE, CLK_CLKSEL2_SPI0SEL_PCLK1, MODULE_NoMsk);
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /* Enable I2C0 clock */
    CLK_EnableModuleClock(I2C0_MODULE);

    /* Enable IP clock */
    CLK_EnableModuleClock(TMR0_MODULE);

    /* Select IP clock source */
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock and CyclesPerUs automatically. */
    SystemCoreClockUpdate();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set PB multi-function pins for UART0 RXD=PB.12 and TXD=PB.13 */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk)) |
                    (SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);

    /* Set I2C0 multi-function pins */
    SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB4MFP_Msk | SYS_GPB_MFPL_PB5MFP_Msk)) |
                    (SYS_GPB_MFPL_PB4MFP_I2C0_SDA | SYS_GPB_MFPL_PB5MFP_I2C0_SCL);

    /* Lock protected registers */
    SYS_LockReg();
}

void UART0_Init()
{
    /* Set GPB multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk);
    SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);
	
    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
}

int main(void)
{
	uint16_t u16ID;
	uint8_t i;

	/* Init System, IP clock and multi-function I/O. */
	SYS_Init();

	/* Init UART0 for printf */
	UART0_Init();
	
	/* Init SPI0 */
	SPI0_Init();

	/* Set PB.2 and PB.3 as Output mode */
	GPIO_SetMode(PB, (BIT2 | BIT3), GPIO_MODE_OUTPUT); 
		
	/* Set WP# and HOLD# of SPI Flash to high */ 
	PB2 = 0;
	PB3 = 0;
	
  /* Init I2C0 */
  I2C0_Init();

  /* I2C enter no address SLV mode */
  I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI_AA);

	/* Check ID of SPI Flash */
	if((u16ID = SpiFlash_ReadMidDid()) != SPI_FLASH_ID) {
		printf("Wrong ID, 0x%x\n", u16ID);
	} else
		printf("Flash found: W25Q32 ...\n");

	printf("[OK]\n");
	printf("\n\nCPU @ %dHz\n", SystemCoreClock);
	EnterLowPowerMode();
	WaitingUpViaI2C();

  while(1) {
		for (i = 0; i < 10; i++)
			TIMER_Delay(TIMER0, 1000000);
		if (g_u8PowerStatus == sys_sleep) {
			EnterLowPowerMode();
			WaitingUpViaI2C();
		}
	}
}
