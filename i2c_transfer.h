#ifndef __I2C_TRANSFER_H__
#define __I2C_TRANSFER_H__

#include <stdio.h>
#include "NuMicro.h"

typedef void (*I2C_FUNC)(uint32_t u32Status);

void I2C0_Init(void);
void I2C0_Close(void);
void I2C_SlaveTRx(uint32_t u32Status);
void I2C0_IRQHandler(void);

#endif
