#ifndef __I2C_TRANSFER_H__
#define __I2C_TRANSFER_H__

#include <stdio.h>
#include "NuMicro.h"

typedef enum {
				test_flash = 0x0c,
				sys_wakeup = 0x10,
				get_status = 0x20,
				sys_sleep = 0x30,
        erase_flash = 0x40,
        write_flash = 0x80,
        read_flash = 0xff,
}SPI_OPS;

typedef void (*I2C_FUNC)(uint32_t u32Status);

void I2C0_Init(void);
void I2C0_Close(void);
void I2C_SlaveTRx(uint32_t u32Status);
void I2C0_IRQHandler(void);

#endif
