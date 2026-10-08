/**
  ******************************************************************************
  * @file    mt35xu512a.h
  * @author  STM32N6 External Memory Driver Suite Team
  * @brief   Driver header for Micron Xccela(TM) Octal NOR Flash memory
  *          (MT35XU / MT35XL series: 256Mb, 512Mb, 1Gb, 2Gb).
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics / Advanced Embedded Systems.
  * All rights reserved.
  ******************************************************************************
  */

#ifndef MT35XU512A_H
#define MT35XU512A_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "extmem_common.h"
#include "stm32n6xx_hal.h"

/* Memory Constants ----------------------------------------------------------*/
#define MT35XU_FLASH_SIZE_256MBIT        (32U * 1024U * 1024U)   /* 32 MBytes  */
#define MT35XU_FLASH_SIZE_512MBIT        (64U * 1024U * 1024U)   /* 64 MBytes  */
#define MT35XU_FLASH_SIZE_1GBIT          (128U * 1024U * 1024U)  /* 128 MBytes */
#define MT35XU_FLASH_SIZE_2GBIT          (256U * 1024U * 1024U)  /* 256 MBytes */

#define MT35XU_PAGE_SIZE                 256U                    /* 256 Bytes  */
#define MT35XU_SECTOR_4K_SIZE            (4U * 1024U)            /* 4 KBytes   */
#define MT35XU_BLOCK_128K_SIZE           (128U * 1024U)          /* 128 KBytes */

#define MT35XU_OK                        (0)
#define MT35XU_ERROR                     (-1)
#define MT35XU_BUSY                      (-2)
#define MT35XU_TIMEOUT                   (-3)

/* JEDEC Identification */
#define MT35XU_MANUFACTURER_ID           0x2CU
#define MT35XU_MEMORY_TYPE_1V8           0x5BU                   /* MT35XU (1.8V) */
#define MT35XU_MEMORY_TYPE_3V0           0x5AU                   /* MT35XL (3.0V) */

/* Commands (Standard SPI 1-line) */
#define MT35XU_CMD_READ_ID               0x9FU
#define MT35XU_CMD_READ_STATUS_REG       0x05U
#define MT35XU_CMD_READ_FLAG_STATUS_REG  0x70U
#define MT35XU_CMD_WRITE_ENABLE          0x06U
#define MT35XU_CMD_WRITE_DISABLE         0x04U
#define MT35XU_CMD_READ_VCR              0x85U                   /* Volatile Configuration Register */
#define MT35XU_CMD_WRITE_VCR             0x81U
#define MT35XU_CMD_READ_NVCR             0xB5U                   /* Non-Volatile Configuration Register */
#define MT35XU_CMD_WRITE_NVCR            0xB1U
#define MT35XU_CMD_ENTER_4BYTE_ADDR      0xB7U
#define MT35XU_CMD_FAST_READ_4B          0x0CU                   /* 4-byte Fast Read (8 dummy) */
#define MT35XU_CMD_DIE_ERASE             0xC4U                   /* Die erase (multi-die parts) */
#define MT35XU_CMD_RESET_ENABLE          0x66U
#define MT35XU_CMD_RESET                 0x99U

/* Commands (8D-8D-8D Octal DTR): the command extension is the repeated opcode byte */
#define MT35XU_OCTAL_CMD_READ_ID         0x9F9FU
#define MT35XU_OCTAL_CMD_READ_STATUS_REG 0x0505U
#define MT35XU_OCTAL_CMD_READ_FLAG_STATUS 0x7070U
#define MT35XU_OCTAL_CMD_WRITE_ENABLE    0x0606U
#define MT35XU_OCTAL_CMD_WRITE_VCR       0x8181U
#define MT35XU_OCTAL_CMD_FAST_READ_DTR   0xFDFDU                 /* DDR Octal I/O Fast Read */
#define MT35XU_OCTAL_CMD_PAGE_PROGRAM    0x1212U                 /* 4-byte Page Program */
#define MT35XU_OCTAL_CMD_SECTOR_ERASE_4K 0x2121U                 /* 4KB Subsector Erase */
#define MT35XU_OCTAL_CMD_BLOCK_ERASE     0xDCDCU                 /* 128KB Sector Erase */
#define MT35XU_OCTAL_CMD_CHIP_ERASE      0xC7C7U                 /* Bulk Erase (single die) */
#define MT35XU_OCTAL_CMD_DIE_ERASE       0xC4C4U                 /* Die Erase (multi-die) */

/* Volatile Configuration Register map */
#define MT35XU_VCR_ADDR_IO_MODE          0x00000000U
#define MT35XU_VCR_ADDR_DUMMY_CYCLES     0x00000001U
#define MT35XU_VCR_IO_MODE_OCTAL_DTR     0xE7U                   /* Octal DDR with DQS */
#define MT35XU_VCR_IO_MODE_EXT_SPI       0xFFU                   /* Extended SPI (default) */
#define MT35XU_VCR_DUMMY_DEFAULT         0x1FU
#define MT35XU_DIE_SIZE                  (64U * 1024U * 1024U)   /* 512 Mbit per die */

/* Status & Flag Status Bits */
#define MT35XU_SR_WIP                    (1U << 0)               /* Write In Progress */
#define MT35XU_SR_WEL                    (1U << 1)               /* Write Enable Latch */
#define MT35XU_FSR_READY                 (1U << 7)               /* Program/Erase Controller Ready */
#define MT35XU_FSR_ERASE_ERROR           (1U << 5)               /* Erase Error */
#define MT35XU_FSR_PROGRAM_ERROR         (1U << 4)               /* Program Error */

/* Exported Functions --------------------------------------------------------*/
int32_t MT35XU_Init(XSPI_HandleTypeDef *Ctx, uint32_t ClockPrescaler, uint32_t MemorySize);
int32_t MT35XU_ReadID(XSPI_HandleTypeDef *Ctx, uint8_t *pID);
int32_t MT35XU_ReadStatus(XSPI_HandleTypeDef *Ctx, uint8_t *pStatus);
int32_t MT35XU_ReadFlagStatus(XSPI_HandleTypeDef *Ctx, uint8_t *pFlagStatus);
int32_t MT35XU_WriteEnable(XSPI_HandleTypeDef *Ctx);
int32_t MT35XU_EnterOctalDTRMode(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles);
int32_t MT35XU_ExitOctalMode(XSPI_HandleTypeDef *Ctx);
int32_t MT35XU_Read(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address, uint8_t *pData, uint32_t Size, uint8_t DummyCycles);
int32_t MT35XU_PageProgram(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address, const uint8_t *pData, uint32_t Size);
int32_t MT35XU_EraseSector4K(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address);
int32_t MT35XU_EraseBlock128K(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t Address);
int32_t MT35XU_EraseChip(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode);
int32_t MT35XU_EraseDie(XSPI_HandleTypeDef *Ctx, ExtMem_Mode_t Mode, uint32_t DieAddress);
int32_t MT35XU_EnableMemoryMappedModeDTR(XSPI_HandleTypeDef *Ctx, uint8_t DummyCycles);
int32_t MT35XU_Reset(XSPI_HandleTypeDef *Ctx);

#ifdef __cplusplus
}
#endif

#endif /* MT35XU512A_H */
