#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NuMicro.h"
#include "i2c_transfer.h"
#include "spi_transfer.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint32_t slave_buff_addr;
volatile uint32_t flash_addr;
volatile uint8_t g_u8Flag;
volatile uint8_t g_au8SlvData[PAGE_LENGTH];
volatile uint8_t g_u8SlvTxData[PAGE_LENGTH];
volatile uint8_t g_au8SlvRxData[7];

volatile uint8_t g_u8DeviceAddr;
volatile uint8_t g_u8SlvDataLen;
volatile static I2C_FUNC s_I2C0HandlerFn = NULL;
extern uint8_t g_u8SlvI2CWK, g_u8PowerStatus;
/*---------------------------------------------------------------------------------------------------------*/
/*  I2C0 IRQ Handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void I2C0_IRQHandler(void)
{
    uint32_t u32Status;

		/* Check I2C Wake-up interrupt flag set or not */
    if(I2C_GET_WAKEUP_FLAG(I2C0))
    {
        /* Clear I2C Wake-up interrupt flag */
        I2C_CLEAR_WAKEUP_FLAG(I2C0);
				g_u8SlvI2CWK = 1;

        return;
    }

    u32Status = I2C_GET_STATUS(I2C0);

//		printf("get status:%x\n", u32Status);
	
    if(I2C_GET_TIMEOUT_FLAG(I2C0))
    {
        /* Clear I2C0 Timeout Flag */
        I2C_ClearTimeoutFlag(I2C0);
    }
    else
    {
        if(s_I2C0HandlerFn != NULL)
            s_I2C0HandlerFn(u32Status);
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  I2C TRx Callback Function                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_SlaveTRx(uint32_t u32Status)
{
	uint8_t u8data;
	static uint32_t u32Count = 0;
//	printf("status:%x\n", u32Status);
	switch (u32Status) {
		case 0x60:		/* Own SLA+W has been receive; ACK has been return */
			g_u8SlvDataLen = 0;
		  u32Count = 0;
      I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI_AA);
		break;
		case 0x80:	/* Previously address with own SLA address Data has been received; ACK has been returned*/
			u8data = (unsigned char) I2C_GET_DATA(I2C0);
//			printf("g_u8SlvDataLen:%d\n", g_u8SlvDataLen);
			if (g_u8SlvDataLen < 7) {
				g_au8SlvRxData[g_u8SlvDataLen++] = u8data;
				g_u8Flag = g_au8SlvRxData[0];
				flash_addr = (g_au8SlvRxData[1] << 16) + (g_au8SlvRxData[2] << 8) + (g_au8SlvRxData[3]);
				slave_buff_addr = (g_au8SlvRxData[4] << 16) + (g_au8SlvRxData[5] << 8) + (g_au8SlvRxData[6]);
				switch (g_u8Flag) {
					case test_flash:
						if (SpiFlash_ReadMidDid() == SPI_FLASH_ID)
							g_u8SlvTxData[0] = 0x0c;
						else
							g_u8SlvTxData[0] = 0xc0;
					break;
					case sys_sleep:
						g_u8PowerStatus = sys_sleep;
					break;
					case sys_wakeup:
						g_u8PowerStatus = sys_wakeup;
						g_u8SlvTxData[0] = sys_wakeup;
					break;
					case get_status:
	//				printf("get status\n");
						g_u8SlvTxData[0] = SpiFlash_ReadStatusReg();
					break;
					case read_flash:
						if (g_u8SlvDataLen == 7)
	//				printf("Normal read...");
							SpiFlash_NormalRead(flash_addr, slave_buff_addr);
//					printf("OK\n");
					break;
					case erase_flash:
						if (g_u8SlvDataLen == 7)
							SpiFlash_Erase(flash_addr, slave_buff_addr);
					break;
				}
			}	else {
//			printf("u32Count:%d\n", u32Count);
				g_au8SlvData[u32Count++] = u8data;
				if (u32Count == slave_buff_addr) {
//					printf("Start to normal write data to Flash ...");
					SpiFlash_NormalPageProgram(flash_addr, slave_buff_addr);
					SpiFlash_WaitReady();
					u32Count = 0;
//					printf("OK\n");
				} else if (u32Count > slave_buff_addr) {
//					printf("Error not left space\n");
					u32Count = 0;
				}
			}
			I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI_AA);
		break;
		case 0xA8:	/* Own SLA+R has been receive; ACK has been return */
			  SpiFlash_WaitReady();
				u8data = (unsigned char)I2C_GET_DATA(I2C0);
				slave_buff_addr = 0;
//				printf("send data:%x\n", g_u8SlvTxData[slave_buff_addr]);
				I2C_SET_DATA(I2C0, slave_buff_addr);
				I2C_SET_DATA(I2C0, g_u8SlvTxData[slave_buff_addr]);
        slave_buff_addr++;
        I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI_AA);
		break;
		case 0xB8:	/* Data byte in I2CDAT has been transmitted ACK has been received */
			SpiFlash_WaitReady();
			I2C_SET_DATA(I2C0, g_u8SlvTxData[slave_buff_addr++]);
      I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI_AA);
		break;
		case 0xC0:	/* Data byte or last data in I2CDAT has been transmitted Not ACK has been received */
			I2C_SET_DATA(I2C0, g_u8SlvTxData[slave_buff_addr++]);
			slave_buff_addr = 0;
      I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI_AA);
		break;
		case 0x88:	/* Previously addressed with own SLA address; NOT ACK has been returned */
			g_u8SlvDataLen = 0;
			u32Count = 0;
      I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI_AA);
		break;
		case 0xA0:	/* A STOP or repeated START has been received while still addressed as Slave/Receiver*/
			g_u8SlvDataLen = 0;
			u32Count = 0;
      I2C_SET_CONTROL_REG(I2C0, I2C_CTL_SI_AA);
		break;
		default:
			printf("Status 0x%x is NOT processed\n", u32Status);
		break;
	}
	return;
}

void I2C0_Init(void)
{
    /* Open I2C module and set bus clock */
    I2C_Open(I2C0, 400000);

    /* Get I2C0 Bus Clock */
    printf("I2C clock %d Hz\n", I2C_GetBusClockFreq(I2C0));

    /* Set I2C 4 Slave Addresses */
    I2C_SetSlaveAddr(I2C0, 0, 0x15, 0);   /* Slave Address : 0x15 */
#if 0
		I2C_SetSlaveAddr(I2C0, 1, 0x35, 0);   /* Slave Address : 0x35 */
    I2C_SetSlaveAddr(I2C0, 2, 0x55, 0);   /* Slave Address : 0x55 */
    I2C_SetSlaveAddr(I2C0, 3, 0x75, 0);   /* Slave Address : 0x75 */

    I2C_SetSlaveAddrMask(I2C0, 0, 0x01);
    I2C_SetSlaveAddrMask(I2C0, 1, 0x04);
    I2C_SetSlaveAddrMask(I2C0, 2, 0x01);
    I2C_SetSlaveAddrMask(I2C0, 3, 0x04);
#endif
    /* Enable I2C interrupt */
    I2C_EnableInt(I2C0);
    NVIC_EnableIRQ(I2C0_IRQn);
		
		/* I2C function to Slave receive/transmit data */
    s_I2C0HandlerFn = I2C_SlaveTRx;
}

void I2C0_Close(void)
{
    /* Disable I2C0 interrupt and clear corresponding NVIC bit */
    I2C_DisableInt(I2C0);
    NVIC_DisableIRQ(I2C0_IRQn);

    /* Disable I2C0 and close I2C0 clock */
    I2C_Close(I2C0);
    CLK_DisableModuleClock(I2C0_MODULE);

}
