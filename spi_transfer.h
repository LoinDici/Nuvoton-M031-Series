#ifndef __SPI_TRANSFER_H__
#define __SPI_TRANSFER_H__

#include <stdio.h>
#include "NuMicro.h"
#include "i2c_transfer.h"

#define SPI_FLASH_PORT		SPI0
#define SPI_FLASH_FREQ		10000000
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

typedef struct {
   uint8_t device_id[2];
   unsigned char version;
   unsigned char date[5];
   unsigned char calibra_data_start[3];
   unsigned char calibra_data_end[3];
   unsigned char compensate_data_start[3];
   unsigned char compensate_data_end[3];
   void *calibra_data;
   void *compensate_data;
   unsigned char md[16];
}SPI_INFO, *PSPI_INFO;
#endif
