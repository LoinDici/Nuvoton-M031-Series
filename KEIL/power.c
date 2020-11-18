#include "NuMicro.h"
#include "spi_transfer.h"
#include <stdio.h>

volatile uint8_t g_u8SlvPWRDNWK, g_u8SlvI2CWK;

/*---------------------------------------------------------------------------------------------------------*/
/*  Power Wake-up IRQ Handler                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void PWRWU_IRQHandler(void)
{
    /* Check system power down mode wake-up interrupt flag */
    if(((CLK->PWRCTL) & CLK_PWRCTL_PDWKIF_Msk) != 0)
    {
        /* Clear system power down wake-up interrupt flag */
        CLK->PWRCTL |= CLK_PWRCTL_PDWKIF_Msk;
        g_u8SlvPWRDNWK = 1;

    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Function for System Entry to Power Down Mode                                                           */
/*---------------------------------------------------------------------------------------------------------*/
void PowerDownFunction(void)
{
    /* Check if all the debug messages are finished */
    UART_WAIT_TX_EMPTY(DEBUG_PORT);

    /* Enter to Power-down mode */
    CLK_PowerDown();
}

void EnterLowPowerMode()
{
		/* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable power wake-up interrupt */
    CLK->PWRCTL |= CLK_PWRCTL_PDWKIEN_Msk;
    NVIC_EnableIRQ(PWRWU_IRQn);
    g_u8SlvPWRDNWK = 0;

    /* Enable I2C wake-up */
    I2C_EnableWakeup(I2C0);
    g_u8SlvI2CWK = 0;

    printf("Enter PD 0x%x 0x%x\n", I2C0->CTL0, I2C0->STATUS0);
    printf("\n");
    printf("CHIP enter power down status.\n");

    if(((I2C0->CTL0)&I2C_CTL0_SI_Msk) != 0)
    {
        I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI);
    }
    /* Enter to Power-down mode */
    PowerDownFunction();

		/* Lock protected registers */
    SYS_LockReg();
}

void WakeUpViaI2C()
{
		/* Unlock protected registers */
    SYS_UnlockReg();

		/* Waiting for syteem wake-up and I2C wake-up finish*/
    while((g_u8SlvPWRDNWK & g_u8SlvI2CWK) == 0)
			;

    /* Waitinn for I2C response ACK finish */
    while(!I2C_GET_WAKEUP_DONE(I2C0))
			;

    /* Clear Wakeup done flag, I2C will release bus */
    I2C_CLEAR_WAKEUP_DONE(I2C0);

    /* Wake-up Interrupt Message */
    printf("Power-down Wake-up INT 0x%x\n", (unsigned int)((CLK->PWRCTL) & CLK_PWRCTL_PDWKIF_Msk));
    printf("I2C0 WAKE INT 0x%x\n", I2C0->WKSTS);

    /* Disable power wake-up interrupt */
    CLK->PWRCTL &= ~CLK_PWRCTL_PDWKIEN_Msk;
    NVIC_DisableIRQ(PWRWU_IRQn);

    /* Lock protected registers */
    SYS_LockReg();
   
		printf("\n");
    printf("Slave wake-up from power down status.\n");

    printf("\n");
    printf("Slave Waiting for receiving data.\n");
}
