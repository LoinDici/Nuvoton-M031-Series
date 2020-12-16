#ifndef __SPI_TRANSFER_H__
#define __SPI_TRANSFER_H__

#include <stdio.h>
#include "NuMicro.h"
#include "i2c_transfer.h"

#define SPI_FLASH_PORT		SPI0
#define SPI_FLASH_FREQ		48000000
#define SPI_FLASH_ID		0xEF16
#define SPI_FLASH_SECTOR	4096
#define SPI_FLASH_32KB_BLOCK	32768
#define SPI_FLASH_64KB_BLOCK	65536
#define SPI_FLASH_MEMORY	8388608

#define PAGE_NUMBER 		32768   	/* page numbers */
#define PAGE_LENGTH 		256 	/* length */

uint16_t SpiFlash_ReadMidDid(void);
void SpiFlash_ChipErase(void);
uint8_t SpiFlash_ReadStatusReg(void);
void SpiFlash_WriteStatusReg(uint8_t u8Value);
void SpiFlash_WaitReady(void);
void SpiFlash_NormalPageProgram(uint32_t StartAddress, uint32_t u32DataSize);
void SpiFlash_NormalRead(uint32_t StartAddress, uint32_t u32DataSize);
void SPI0_Init(void);
void Compare(uint32_t u32DataSize);
void SpiFlash_BlockErase64KB(uint32_t StartAddress);
void SpiFlash_Erase(uint32_t flash_addr, uint32_t size);

#endif
