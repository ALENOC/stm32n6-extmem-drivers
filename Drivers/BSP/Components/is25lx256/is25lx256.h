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
#define IS25LX256_BLOCK_128K             (128U * 1024U)        /* 128 KBytes (uniform sector) */

#define IS25LX_OK                        (0)
#define IS25LX_ERROR                     (-1)
#define IS25LX_TIMEOUT                   (-3)

/* ISSI Identification */
#define IS25LX_MANUFACTURER_ID           0x9DU
#define IS25LX_MEMORY_TYPE_3V0           0x5AU                 /* IS25LX (3.0V) */
#define IS25LX_MEMORY_TYPE_1V8           0x5BU                 /* IS25WX (1.8V) */

/* Registers (Volatile Configuration Registers - VCR) */
#define IS25LX_VCR_ADDR_IO_MODE          0x00000000U
#define IS25LX_VCR_ADDR_DUMMY_CYCLES     0x00000001U

/* I/O Mode register values */
#define IS25LX_IO_MODE_SPI               0xFFU /* Extended SPI (default)      */
#define IS25LX_IO_MODE_OCTAL_DTR         0xE7U /* Octal DDR with DQS          */
#define IS25LX_IO_MODE_OCTAL_DTR_NO_DQS  0xC7U /* Octal DDR without DQS       */

/* Status Register Masks */
#define IS25LX_SR_WIP                    (1U << 0)
#define IS25LX_SR_WEL                    (1U << 1)

/* Commands (1-1-1 SPI) */
#define IS25LX_CMD_READ_ID               0x9FU
#define IS25LX_CMD_READ_SFDP             0x5AU
#define IS25LX_CMD_READ_FAST_4B          0x0CU
#define IS25LX_CMD_PAGE_PROG_4B          0x12U
#define IS25LX_CMD_SECTOR_ERASE_4K_4B    0x21U
#define IS25LX_CMD_BLOCK_ERASE_128K_4B   0xDCU
#define IS25LX_CMD_CHIP_ERASE            0x60U
#define IS25LX_CMD_WRITE_ENABLE          0x06U
#define IS25LX_CMD_WRITE_DISABLE         0x04U
#define IS25LX_CMD_READ_STATUS           0x05U
#define IS25LX_CMD_READ_VCR              0x85U
#define IS25LX_CMD_WRITE_VCR             0x81U
#define IS25LX_CMD_ENTER_4BYTE_ADDR      0xB7U
#define IS25LX_CMD_RESET_ENABLE          0x66U
#define IS25LX_CMD_RESET                 0x99U

/* Commands (8D-8D-8D Octal DTR): the command extension is the repeated opcode byte */
#define IS25LX_DTR_CMD_READ              0xFDFDU /* DDR Octal I/O Fast Read, 4-byte address */
#define IS25LX_DTR_CMD_PAGE_PROG         0x1212U
#define IS25LX_DTR_CMD_SECTOR_ERASE_4K   0x2121U
#define IS25LX_DTR_CMD_BLOCK_ERASE_128K  0xDCDCU
#define IS25LX_DTR_CMD_CHIP_ERASE        0xC7C7U
#define IS25LX_DTR_CMD_WRITE_ENABLE      0x0606U
#define IS25LX_DTR_CMD_WRITE_DISABLE     0x0404U
#define IS25LX_DTR_CMD_READ_STATUS       0x0505U /* No address, 8 dummy cycles */
#define IS25LX_DTR_CMD_READ_VCR          0x8585U
#define IS25LX_DTR_CMD_WRITE_VCR         0x8181U

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
int32_t IS25LX256_EraseBlock128K(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address);
int32_t IS25LX256_ChipErase(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode);
int32_t IS25LX256_EnableMemoryMappedModeDTR(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles);
int32_t IS25LX256_Reset(XSPI_HandleTypeDef *Ctx);

#ifdef __cplusplus
}
#endif

#endif /* IS25LX256_H */
