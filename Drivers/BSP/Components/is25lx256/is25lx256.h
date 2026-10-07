/**
  ******************************************************************************
  * @file    is25lx256.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for ISSI Octal NOR Flash (IS25LX / IS25WX series).
  *          Supports IS25LX064, IS25WX064, IS25LX128, IS25WX128,
  *          IS25LX256, IS25WX256, IS25LX512, IS25WX512.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  ******************************************************************************
  */

#ifndef IS25LX256_H
#define IS25LX256_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "extmem_common.h"
#include "stm32n6xx_hal.h"

#define IS25LX256_FLASH_SIZE             (32U * 1024U * 1024U) /* 256 Mbits = 32 MBytes */
#define IS25LX256_PAGE_SIZE              256U                  /* 256 Bytes */
#define IS25LX256_SECTOR_4K              (4U * 1024U)          /* 4 KBytes  */
#define IS25LX256_BLOCK_64K              (64U * 1024U)         /* 64 KBytes */

#define IS25LX_OK                        (0)
#define IS25LX_ERROR                     (-1)
#define IS25LX_TIMEOUT                   (-3)

/* ISSI Identification */
#define IS25LX_MANUFACTURER_ID           0x9DU
#define IS25LX_MEMORY_TYPE_OCTAL         0x5BU

/* Registers (Volatile Configuration Registers - VCR) */
#define IS25LX_VCR_ADDR_DUMMY_CYCLES     0x00000000U
#define IS25LX_VCR_ADDR_IO_MODE          0x00000001U

/* I/O Mode register values */
#define IS25LX_IO_MODE_SPI               0xFFU
#define IS25LX_IO_MODE_OCTAL_STR         0xDFU
#define IS25LX_IO_MODE_OCTAL_DTR         0xE7U

/* Status Register Masks */
#define IS25LX_SR_WIP                    (1U << 0)
#define IS25LX_SR_WEL                    (1U << 1)

/* Commands (1-1-1 SPI) */
#define IS25LX_CMD_READ_ID               0x9FU
#define IS25LX_CMD_READ_SFDP             0x5AU
#define IS25LX_CMD_READ_FAST_4B          0x0CU
#define IS25LX_CMD_PAGE_PROG_4B          0x12U
#define IS25LX_CMD_SECTOR_ERASE_4K_4B    0x21U
#define IS25LX_CMD_BLOCK_ERASE_64K_4B    0xDCU
#define IS25LX_CMD_CHIP_ERASE            0x60U
#define IS25LX_CMD_WRITE_ENABLE          0x06U
#define IS25LX_CMD_WRITE_DISABLE         0x04U
#define IS25LX_CMD_READ_STATUS           0x05U
#define IS25LX_CMD_READ_VCR              0x85U
#define IS25LX_CMD_WRITE_VCR             0x81U
#define IS25LX_CMD_RESET_ENABLE          0x66U
#define IS25LX_CMD_RESET                 0x99U

/* Commands (8D-8D-8D Octal DTR) */
#define IS25LX_DTR_CMD_READ              0xEE11U
#define IS25LX_DTR_CMD_PAGE_PROG         0x12EDU
#define IS25LX_DTR_CMD_SECTOR_ERASE_4K   0x21DEU
#define IS25LX_DTR_CMD_BLOCK_ERASE_64K   0xDC23U
#define IS25LX_DTR_CMD_CHIP_ERASE        0x609FU
#define IS25LX_DTR_CMD_WRITE_ENABLE      0x06F9U
#define IS25LX_DTR_CMD_WRITE_DISABLE     0x04FBU
#define IS25LX_DTR_CMD_READ_STATUS       0x05FAU
#define IS25LX_DTR_CMD_READ_VCR          0x857AU
#define IS25LX_DTR_CMD_WRITE_VCR         0x817EU

/* Exported Functions --------------------------------------------------------*/
int32_t IS25LX256_ReadID(XSPI_HandleTypeDef *Ctx, uint8_t *pID);
int32_t IS25LX256_WriteEnable(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode);
int32_t IS25LX256_AutoPollingMemReady(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Timeout);
int32_t IS25LX256_ReadVCR(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t RegAddr, uint8_t *pValue);
int32_t IS25LX256_WriteVCR(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t RegAddr, uint8_t Value);
int32_t IS25LX256_EnterOctalDTRMode(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles);
int32_t IS25LX256_ExitOctalDTRMode(XSPI_HandleTypeDef *Ctx);
int32_t IS25LX256_Read(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles);
int32_t IS25LX256_PageProgram(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address, const uint8_t *pData, uint32_t Size);
int32_t IS25LX256_EraseSector4K(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address);
int32_t IS25LX256_EraseBlock64K(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address);
int32_t IS25LX256_ChipErase(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode);
int32_t IS25LX256_EnableMemoryMappedModeDTR(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles);
int32_t IS25LX256_Reset(XSPI_HandleTypeDef *Ctx);

#ifdef __cplusplus
}
#endif

#endif /* IS25LX256_H */
